#include <stdio.h>
#include <stdlib.h>
#include <fftw3.h>
#include <tensorflow/c/c_api.h>
#include "wav.h"

// 错误报告函数
void NoOpDeallocator(void* data, size_t length, void* arg) {}

// 加载模型的函数
TF_Graph* LoadGraph(const char* model_path) {
    TF_Graph* graph = TF_NewGraph();
    TF_Status* status = TF_NewStatus();
    
    FILE* file = fopen(model_path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open model file: %s\n", model_path);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    void* data = malloc(file_size);
    fread(data, file_size, 1, file);
    fclose(file);
    
    TF_Buffer* model_data = TF_NewBufferFromString(data, file_size);
    
    TF_ImportGraphDefOptions* options = TF_NewImportGraphDefOptions();
    TF_GraphImportGraphDef(graph, model_data, options, status);
    
    if (TF_GetCode(status) != TF_OK) {
        fprintf(stderr, "Failed to import graph: %s\n", TF_Message(status));
        return NULL;
    }
    
    TF_DeleteBuffer(model_data);
    TF_DeleteImportGraphDefOptions(options);
    TF_DeleteStatus(status);
    
    return graph;
}

// 运行模型的函数
void RunModel(TF_Graph* graph, float* input_data, int num_elements, float* output_data) {
    TF_SessionOptions* options = TF_NewSessionOptions();
    TF_Status* status = TF_NewStatus();
    TF_Session* session = TF_NewSession(graph, options, status);
    
    if (TF_GetCode(status) != TF_OK) {
        fprintf(stderr, "Failed to create session: %s\n", TF_Message(status));
        return;
    }
    
    TF_Tensor* input_tensor = TF_NewTensor(TF_FLOAT, (int64_t[]){num_elements}, 1, input_data, sizeof(float) * num_elements, NoOpDeallocator, NULL);
    TF_Output input_op = {TF_GraphOperationByName(graph, "input"), 0};
    
    TF_Tensor* output_tensor = NULL;
    TF_Output output_op = {TF_GraphOperationByName(graph, "output"), 0};
    
    TF_SessionRun(session, NULL, &input_op, &input_tensor, 1, &output_op, &output_tensor, 1, NULL, 0, NULL, status);
    
    if (TF_GetCode(status) != TF_OK) {
        fprintf(stderr, "Failed to run session: %s\n", TF_Message(status));
        return;
    }
    
    float* model_output_data = (float*)TF_TensorData(output_tensor);
    for (int i = 0; i < num_elements; ++i) {
        output_data[i] = model_output_data[i];
    }
    
    TF_DeleteTensor(input_tensor);
    TF_DeleteTensor(output_tensor);
    TF_CloseSession(session, status);
    TF_DeleteSession(session, status);
    TF_DeleteStatus(status);
}

// 音频数据处理函数示例
void ProcessAudio(float* input, int length) {
    fftwf_plan plan;
    fftwf_complex* output = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * length);

    // 创建 FFT 计划
    plan = fftwf_plan_dft_r2c_1d(length, input, output, FFTW_ESTIMATE);

    // 执行 FFT
    fftwf_execute(plan);

    // 输出结果
    for (int i = 0; i < length / 2 + 1; ++i) {
        printf("FFT output[%d]: %f + %fi\n", i, output[i][0], output[i][1]);
    }

    // 清理
    fftwf_destroy_plan(plan);
    fftwf_free(output);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <model.tflite> <input.wav>\n", argv[0]);
        return 1;
    }

    const char* model_path = argv[1];
    const char* input_wav_path = argv[2];

    // 打开 WAV 文件
    WavFile* wav = wav_open(input_wav_path, WAV_OPEN_READ);
    if (wav == NULL) {
        fprintf(stderr, "Failed to open WAV file\n");
        return 1;
    }

    // 获取 WAV 文件信息
    int num_channels = wav_get_num_channels(wav);
    int sample_rate = wav_get_sample_rate(wav);
    int num_samples = wav_get_length(wav);

    // 读取音频数据
    float* audio_data = (float*)malloc(sizeof(float) * num_samples);
    wav_read(wav, audio_data, num_samples);

    // 关闭 WAV 文件
    wav_close(wav);

    // 处理音频数据
    ProcessAudio(audio_data, num_samples);

    // 加载 TensorFlow 模型
    TF_Graph* graph = LoadGraph(model_path);
    if (graph == NULL) {
        fprintf(stderr, "Failed to load model\n");
        free(audio_data);
        return 1;
    }

    // 运行 TensorFlow 模型
    float* output_data = (float*)malloc(sizeof(float) * num_samples);
    RunModel(graph, audio_data, num_samples, output_data);

    // 写入处理后的数据到新的 WAV 文件
    WavFile* out_wav = wav_open("out.wav", WAV_OPEN_WRITE);
    if (out_wav == NULL) {
        fprintf(stderr, "Failed to open output WAV file\n");
        free(audio_data);
        free(output_data);
        TF_DeleteGraph(graph);
        return 1;
    }

    wav_set_num_channels(out_wav, num_channels);
    wav_set_sample_rate(out_wav, sample_rate);
    wav_write(out_wav, output_data, num_samples);

    // 关闭输出 WAV 文件
    wav_close(out_wav);

    // 清理
    TF_DeleteGraph(graph);
    free(audio_data);
    free(output_data);

    return 0;
}

// compile with:
// gcc -Wall -o demo demo.c wav.c -lfftw3f -ltensorflow && ./demo <model.tflite> <input.wav>
