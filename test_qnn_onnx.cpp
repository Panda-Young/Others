#include <iostream>
#include <vector>
#include <onnxruntime_c_api.h>
#include <random>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <chrono>

// FP16/uint16_t转float32
float fp16_to_fp32(uint16_t h) {
    uint16_t h_exp = (h & 0x7C00u) >> 10;
    uint16_t h_sig = (h & 0x03FFu);
    uint16_t h_sgn = (h & 0x8000u) >> 15;
    uint32_t f_sgn = ((uint32_t)h_sgn) << 31;
    uint32_t f_exp, f_sig;
    if (h_exp == 0) {
        if (h_sig == 0)
            f_exp = 0, f_sig = 0;
        else {
            h_exp = 1;
            while ((h_sig & 0x0400u) == 0)
                h_sig <<= 1, --h_exp;
            h_sig &= 0x03FFu;
            f_exp = (127 - 15 - (10 - h_exp)) << 23;
            f_sig = ((uint32_t)h_sig) << 13;
        }
    } else if (h_exp == 0x1F) {
        f_exp = 0xFF << 23;
        f_sig = ((h_sig != 0) ? 0x200000 : 0);
    } else {
        f_exp = ((uint32_t)(h_exp + (127 - 15))) << 23;
        f_sig = ((uint32_t)h_sig) << 13;
    }
    uint32_t f = f_sgn | f_exp | f_sig;
    float result;
    std::memcpy(&result, &f, sizeof(float));
    return result;
}

template<typename T>
float compute_mean(const T* data, size_t n) {
    double sum = 0.0;
    for(size_t i=0;i<n;++i) sum += (float)data[i];
    return sum / n;
}

float compute_fp16_mean(const uint16_t* data, size_t n) {
    double sum = 0.0;
    for(size_t i=0;i<n;++i) sum += fp16_to_fp32(data[i]);
    return sum / n;
}

#define CHECK_STATUS(expr)                                        \
  do {                                                            \
    OrtStatus* status = (expr);                                   \
    if (status != nullptr) {                                      \
      const char* msg = ort->GetErrorMessage(status);             \
      std::cerr << "ONNXRuntime error: " << msg << std::endl;     \
      ort->ReleaseStatus(status);                                 \
      return -1;                                                  \
    }                                                             \
  } while (0)

const OrtApi* ort = OrtGetApiBase()->GetApi(ORT_API_VERSION);

int main(int argc, char* argv[]) {
    const char* model_path = "reduce_15db_fp16.onnx";
    if (argc > 1) model_path = argv[1];

    OrtEnv* env = nullptr;
    CHECK_STATUS(ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "qnn_test", &env));

    std::cout << "=== ONNX Runtime QNN/HTP 调试测试 (1.21.1) ===" << std::endl;

    OrtSessionOptions* session_options = nullptr;
    CHECK_STATUS(ort->CreateSessionOptions(&session_options));

    OrtSession* session = nullptr;
    CHECK_STATUS(ort->CreateSession(env, model_path, session_options, &session));

    std::cout << "NOTE: ONNX Runtime 1.21.1 C API does not provide runtime provider list." << std::endl;
    std::cout << "Assuming QNNExecutionProvider is available if statically linked (如已按官方文档静态集成QNN，则QNN/HTP已启用，详见 build log 和 so 体积)." << std::endl;

    size_t num_input_nodes;
    CHECK_STATUS(ort->SessionGetInputCount(session, &num_input_nodes));
    if (num_input_nodes != 1) {
        std::cerr << "仅支持单输入测试，当前模型输入数: " << num_input_nodes << std::endl;
        return -1;
    }
    OrtAllocator* allocator;
    CHECK_STATUS(ort->GetAllocatorWithDefaultOptions(&allocator));
    char* input_name;
    CHECK_STATUS(ort->SessionGetInputName(session, 0, allocator, &input_name));
    OrtTypeInfo* type_info;
    CHECK_STATUS(ort->SessionGetInputTypeInfo(session, 0, &type_info));
    const OrtTensorTypeAndShapeInfo* tensor_info = nullptr;
    CHECK_STATUS(ort->CastTypeInfoToTensorInfo(type_info, &tensor_info));
    ONNXTensorElementDataType elem_type;
    CHECK_STATUS(ort->GetTensorElementType(tensor_info, &elem_type));
    size_t input_dims = 0;
    CHECK_STATUS(ort->GetDimensionsCount(tensor_info, &input_dims));
    std::vector<int64_t> input_shape(input_dims);
    CHECK_STATUS(ort->GetDimensions(tensor_info, input_shape.data(), input_dims));

    std::cout << "模型输入名: " << input_name << std::endl;
    std::cout << "模型输入shape: [";
    for (size_t i=0;i<input_shape.size();++i) std::cout << input_shape[i] << (i+1==input_shape.size()?"]\n":",");

    // 检查并替换动态shape
    bool has_dynamic = false;
    for (size_t i = 0; i < input_shape.size(); ++i) {
        if (input_shape[i] == -1) {
            std::cout << "检测到动态shape（-1），自动替换为 1024" << std::endl;
            input_shape[i] = 1024;
            has_dynamic = true;
        }
    }
    if (has_dynamic) {
        std::cout << "实际用于推理的输入shape: [";
        for (size_t i=0;i<input_shape.size();++i) std::cout << input_shape[i] << (i+1==input_shape.size()?"]\n":",");
    }

    std::cout << "模型输入数据类型: " << (elem_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16 ? "float16":"其他") << std::endl;

    size_t input_numel = 1;
    for (auto d : input_shape) input_numel *= d;

    // 增大测试数据量
    size_t repeat_count = 1000000;
    std::cout << "每次输入数据元素数: " << input_numel << "，总测试次数: " << repeat_count << std::endl;

    // 随机生成fp16数据模板
    std::vector<float> float_input_template(input_numel);
    std::mt19937 gen(42);
    std::normal_distribution<float> dist(0.0f, 1.0f);
    for (auto& v : float_input_template) v = dist(gen);

    // float转fp16
    std::vector<uint16_t> fp16_input_template(input_numel, 0);
    for (size_t i=0;i<input_numel;++i) {
        float f = float_input_template[i];
        uint32_t asInt;
        std::memcpy(&asInt, &f, 4);
        uint16_t sign = (asInt >> 31) & 0x1;
        int16_t exp = ((asInt >> 23) & 0xFF) - 127 + 15;
        uint32_t frac = (asInt >> 13) & 0x3FF;
        if (exp <= 0) exp = 0, frac = 0;
        else if (exp > 31) exp = 31, frac = 0;
        fp16_input_template[i] = (sign << 15) | ((exp & 0x1F) << 10) | (frac & 0x3FF);
    }

    std::cout << "[DEBUG] 输入前10项(float32): ";
    for (size_t i=0;i<std::min<size_t>(10,input_numel);++i) std::cout << float_input_template[i] << " ";
    std::cout << std::endl;
    std::cout << "[DEBUG] 输入前10项(fp16 hex): ";
    for (size_t i=0;i<std::min<size_t>(10,input_numel);++i) printf("0x%04x ", fp16_input_template[i]);
    std::cout << std::endl;

    float input_mean = compute_mean(float_input_template.data(), input_numel);
    std::cout << "输入均值(float32): " << input_mean << std::endl;
    std::cout << "输入均值(fp16->float): " << compute_fp16_mean(fp16_input_template.data(), input_numel) << std::endl;

    OrtMemoryInfo* memory_info;
    CHECK_STATUS(ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info));

    size_t num_output_nodes;
    CHECK_STATUS(ort->SessionGetOutputCount(session, &num_output_nodes));
    if (num_output_nodes != 1) {
        std::cerr << "仅支持单输出测试，当前模型输出数: " << num_output_nodes << std::endl;
        return -1;
    }
    char* output_name;
    CHECK_STATUS(ort->SessionGetOutputName(session, 0, allocator, &output_name));

    std::vector<float> all_output_means;
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t rep = 0; rep < repeat_count; ++rep) {
        OrtValue* input_tensor = nullptr;
        CHECK_STATUS(ort->CreateTensorWithDataAsOrtValue(
            memory_info, (void*)fp16_input_template.data(), fp16_input_template.size() * sizeof(uint16_t),
            input_shape.data(), input_shape.size(), ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16, &input_tensor));

        OrtValue* output_tensor = nullptr;
        CHECK_STATUS(ort->Run(session, nullptr,
                              (const char* const*)&input_name, (const OrtValue* const*)&input_tensor, 1,
                              (const char* const*)&output_name, 1, &output_tensor));

        OrtTensorTypeAndShapeInfo* output_info;
        CHECK_STATUS(ort->GetTensorTypeAndShape(output_tensor, &output_info));
        size_t output_data_len = 0;
        CHECK_STATUS(ort->GetTensorShapeElementCount(output_info, &output_data_len));
        const void* raw = nullptr;
        CHECK_STATUS(ort->GetTensorMutableData(output_tensor, (void**)&raw));
        const uint16_t* output_fp16 = reinterpret_cast<const uint16_t*>(raw);

        if (rep == 0) {
            std::cout << "[DEBUG] 输出前10项(fp16 hex): ";
            for (size_t i = 0; i < std::min<size_t>(10, output_data_len); ++i) {
                printf("0x%04x ", output_fp16[i]);
            }
            std::cout << std::endl;
            std::cout << "[DEBUG] 输出前10项(float32): ";
            for (size_t i = 0; i < std::min<size_t>(10, output_data_len); ++i) {
                std::cout << fp16_to_fp32(output_fp16[i]) << " ";
            }
            std::cout << std::endl;
        }

        float output_mean = compute_fp16_mean(output_fp16, output_data_len);
        all_output_means.push_back(output_mean);

        ort->ReleaseTensorTypeAndShapeInfo(output_info);
        ort->ReleaseValue(output_tensor);
        ort->ReleaseValue(input_tensor);
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total_time_sec = std::chrono::duration<double>(end-start).count();

    std::cout << "总推理次数：" << repeat_count << std::endl;
    std::cout << "总推理耗时（秒）：" << total_time_sec << std::endl;
    std::cout << "平均每次推理耗时（毫秒）：" << (total_time_sec * 1000.0 / repeat_count) << std::endl;
    std::cout << "首个推理输出均值: " << all_output_means[0] << std::endl;

    if (input_mean != 0.0f)
        std::cout << "理论缩放比（首个推理）：" << (all_output_means[0]/input_mean) << std::endl;
    else
        std::cout << "输入均值为0" << std::endl;

    ort->ReleaseMemoryInfo(memory_info);
    ort->ReleaseSession(session);
    ort->ReleaseSessionOptions(session_options);
    ort->ReleaseEnv(env);
    if (input_name) ort->AllocatorFree(allocator, input_name);
    if (output_name) ort->AllocatorFree(allocator, output_name);
    ort->ReleaseTypeInfo(type_info);

    std::cout << "All done!" << std::endl;
    return 0;
}

/* Compile with:
    C:\Users\young\AppData\Local\Android\Sdk\ndk\android-ndk-r27c\toolchains\llvm\prebuilt\windows-x86_64\bin\aarch64-linux-android29-clang++ ^
        -IC:\Users\young\Desktop\onnxruntime-android-1.21.1\headers ^
        -LC:\Users\young\Desktop\onnxruntime-android-1.21.1\jni\arm64-v8a -lonnxruntime ^
        -lz -llog -landroid ^
        test_qnn_onnx.cpp -o test_qnn_onnx
*/

/*Run with:
adb shell mkdir -p /data/local/tmp/qnn_test
adb push test_qnn_onnx /data/local/tmp/qnn_test/
adb shell chmod +x /data/local/tmp/qnn_test/test_qnn_onnx
adb push libonnxruntime.so /data/local/tmp/qnn_test/
adb push reduce_15db_fp16.onnx /data/local/tmp/qnn_test/
adb push libQnnHtp.so /data/local/tmp/qnn_test/
adb push libQnnSystem.so /data/local/tmp/qnn_test/
adb shell
    cd /data/local/tmp/qnn_test
    export LD_LIBRARY_PATH=.
    ./test_qnn_onnx
*/
