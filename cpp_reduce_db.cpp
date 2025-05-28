#include <onnxruntime_cxx_api.h>
#include <vector>
#include <iostream>
#include <array>
#include <windows.h>
#include <stdexcept>
#include <random>

int main() {
    try {
        std::cout << "[INFO] Initializing ONNX Runtime environment..." << std::endl;
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "reduce_db");

        std::cout << "[INFO] Creating session options..." << std::endl;
        Ort::SessionOptions session_options;

        std::wstring model_path = L"reduce_15db.onnx";
        std::wcout << L"[INFO] Loading model: " << model_path << std::endl;
        Ort::Session session(env, model_path.c_str(), session_options);

        // Model input information
        size_t num_input_nodes = session.GetInputCount();
        std::cout << "[INFO] Model has " << num_input_nodes << " input(s)." << std::endl;
        for (size_t i = 0; i < num_input_nodes; ++i) {
            char* input_name = session.GetInputNameAllocated(i, Ort::AllocatorWithDefaultOptions()).get();
            std::cout << "[INFO] Input[" << i << "] name: " << input_name << std::endl;
        }

        // Set the input length to 1*1024
        const int num_samples = 1024;
        std::vector<float> pcm_data(num_samples);

        // Fill input with random float values between 0 and 1
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        for (auto& v : pcm_data) {
            v = dist(gen);
        }


        std::cout << "[RESULT] First 10 input samples: ";
        for (size_t i = 0; i < 10 && i < num_samples; ++i) {
            std::cout << pcm_data[i] << " ";
        }
        std::cout << std::endl;

        std::array<int64_t, 2> input_shape{1, num_samples};
        std::cout << "[INFO] Input shape: [" << input_shape[0] << ", " << input_shape[1] << "]" << std::endl;

        // Create input tensor
        std::cout << "[INFO] Creating input tensor..." << std::endl;
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info,
            pcm_data.data(),
            pcm_data.size(),
            input_shape.data(),
            input_shape.size()
        );
        std::cout << "[INFO] Input tensor created. Element count: " << pcm_data.size() << std::endl;

        // Input and output names
        const char* input_names[] = {"audio_in"};
        const char* output_names[] = {"audio_out"};

        std::cout << "[INFO] Running inference..." << std::endl;
        auto output_tensors = session.Run(
            Ort::RunOptions{nullptr},
            input_names, &input_tensor, 1,
            output_names, 1
        );
        std::cout << "[INFO] Inference finished." << std::endl;

        // Retrieve and print output
        float* out_data = output_tensors[0].GetTensorMutableData<float>();
        size_t out_len = output_tensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
        std::cout << "[INFO] Output element count: " << out_len << std::endl;

        std::cout << "[RESULT] First 10 output samples: ";
        for (size_t i = 0; i < 10 && i < out_len; ++i) {
            std::cout << out_data[i] << " ";
        }
        std::cout << std::endl;

    } catch (const Ort::Exception& e) {
        std::cerr << "[ONNXRuntime ERROR] " << e.what() << std::endl;
        return -1;
    } catch (const std::exception& e) {
        std::cerr << "[STD ERROR] " << e.what() << std::endl;
        return -2;
    } catch (...) {
        std::cerr << "[UNKNOWN ERROR] Unknown exception occurred." << std::endl;
        return -3;
    }
    std::cout << "[INFO] Program finished successfully." << std::endl;
    return 0;
}

// Compile with:
    cl cpp_reduce_db.cpp /I C:\Users\young\Desktop\onnxruntime-win-x64-1.17.0\include /link /LIBPATH:C:\Users\young\Desktop\onnxruntime-win-x64-1.17.0\lib onnxruntime.lib && cpp_reduce_db.exe
