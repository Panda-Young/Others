/* **************************************************************************
 * @Description: loop play buffer example
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * @Date: 2025-06-23 16:07:47
 * @Copyright (c) 2025 by @AAC Technologies, All Rights Reserved.
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h> // 添加时间头文件用于随机数种子
#include <math.h> // 添加数学头文件用于生成波形
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// 音频参数配置
#define SAMPLE_RATE 44100         // 44.1kHz
#define CHANNELS 1                // 单声道
#define SAMPLE_SIZE_BYTES 2       // 16-bit PCM
#define SAMPLES_PER_BUFFER 512    // 每次处理的样本数

// 音频缓冲区上下文结构
typedef struct {
    const short* data;      // PCM数据指针
    size_t size;            // 数据总大小(字节)
    size_t pos;             // 当前读取位置(字节)
} pcm_buffer_ctx_t;

// 预处理函数示例（可替换为自定义算法）
void process_audio_buffer(short* buffer, size_t samples) {
    // 示例：反转音频相位（实际使用时替换为您的算法）
    for (size_t i = 0; i < samples; ++i) {
        buffer[i] = -buffer[i];
    }
}

// 音频队列回调函数
void buffer_callback(SLAndroidSimpleBufferQueueItf bq, void *ctx_ptr) {
    pcm_buffer_ctx_t* ctx = (pcm_buffer_ctx_t*)ctx_ptr;
    const size_t chunk_bytes = SAMPLES_PER_BUFFER * SAMPLE_SIZE_BYTES;
    
    // 计算可读取字节数（支持循环播放）
    size_t bytes_left = ctx->size - ctx->pos;
    size_t bytes_to_copy = (bytes_left >= chunk_bytes) ? chunk_bytes : bytes_left;
    
    if (bytes_to_copy == 0) {  // 到达缓冲区末尾，重置位置
        ctx->pos = 0;
        bytes_to_copy = (ctx->size >= chunk_bytes) ? chunk_bytes : ctx->size;
    }

    // 创建处理缓冲区
    short* proc_buffer = (short*)malloc(chunk_bytes);
    if (!proc_buffer) {
        printf("[ERROR] Failed to allocate processing buffer\n");
        return;
    }

    // 复制并处理音频数据
    memcpy(proc_buffer, ctx->data + ctx->pos/SAMPLE_SIZE_BYTES, bytes_to_copy);
    if (bytes_to_copy < chunk_bytes) {  // 填充剩余空间
        memset((char*)proc_buffer + bytes_to_copy, 0, chunk_bytes - bytes_to_copy);
    }
    process_audio_buffer(proc_buffer, SAMPLES_PER_BUFFER);

    // 提交处理后的音频数据
    SLresult res = (*bq)->Enqueue(bq, proc_buffer, chunk_bytes);
    if (res != SL_RESULT_SUCCESS) {
        printf("[ERROR] Enqueue failed: %d\n", res);
    } else {
        ctx->pos = (ctx->pos + bytes_to_copy) % ctx->size;  // 更新位置（循环）
    }
    free(proc_buffer);
}

// 生成白噪声样本（-32768到32767之间）
short generate_white_noise() {
    return (short)(rand() % 65536 - 32768);
}

// 生成正弦波样本（440Hz A4音高）
short generate_sine_wave(size_t index) {
    double frequency = 440.0; // A4音高
    double sample_rate = (double)SAMPLE_RATE;
    double sin_value = sin(2 * M_PI * frequency * index / sample_rate);
    return (short)(sin_value * 32767);
}

int main() {
    printf("[INFO] Initializing audio player with random samples...\n");
    
    // 初始化随机数种子
    srand((unsigned int)time(NULL));
    
    // 创建包含声音的音频缓冲区（1秒时长）
    const size_t buffer_samples = SAMPLE_RATE * CHANNELS;  // 44100样本
    short* audio_buffer = (short*)malloc(buffer_samples * SAMPLE_SIZE_BYTES);
    
    // 填充缓冲区（混合白噪声和正弦波）
    for (size_t i = 0; i < buffer_samples; ++i) {
        // 混合噪声和正弦波（比例可调整）
        short noise = (short)(generate_white_noise() * 0.2); // 20% 白噪声
        short tone = generate_sine_wave(i);                  // 440Hz 正弦波
        
        // 限制在16位范围内防止溢出
        int mixed = noise + tone;
        if (mixed > 32767) mixed = 32767;
        if (mixed < -32768) mixed = -32768;
        
        audio_buffer[i] = (short)mixed * 0.175;
    }
    
    // 初始化OpenSL引擎
    SLObjectItf engine_obj, output_mix, player_obj;
    SLEngineItf engine;
    SLresult res = slCreateEngine(&engine_obj, 0, NULL, 0, NULL, NULL);
    (*engine_obj)->Realize(engine_obj, SL_BOOLEAN_FALSE);
    (*engine_obj)->GetInterface(engine_obj, SL_IID_ENGINE, &engine);

    // 创建输出混音器
    (*engine)->CreateOutputMix(engine, &output_mix, 0, NULL, NULL);
    (*output_mix)->Realize(output_mix, SL_BOOLEAN_FALSE);

    // 配置音频源
    SLDataLocator_AndroidSimpleBufferQueue bufq_loc = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2
    };
    SLDataFormat_PCM pcm_format = {
        SL_DATAFORMAT_PCM,
        CHANNELS,
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER,
        SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource audio_src = {&bufq_loc, &pcm_format};
    
    // 配置音频接收器
    SLDataLocator_OutputMix outmix_loc = {
        SL_DATALOCATOR_OUTPUTMIX, output_mix
    };
    SLDataSink audio_sink = {&outmix_loc, NULL};

    // 创建播放器
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    (*engine)->CreateAudioPlayer(engine, &player_obj, &audio_src, &audio_sink, 
                                1, ids, req);
    (*player_obj)->Realize(player_obj, SL_BOOLEAN_FALSE);

    // 获取播放和缓冲队列接口
    SLPlayItf player;
    SLAndroidSimpleBufferQueueItf player_bufq;
    (*player_obj)->GetInterface(player_obj, SL_IID_PLAY, &player);
    (*player_obj)->GetInterface(player_obj, SL_IID_BUFFERQUEUE, &player_bufq);

    // 设置播放上下文
    pcm_buffer_ctx_t ctx = {
        .data = audio_buffer,
        .size = buffer_samples * SAMPLE_SIZE_BYTES,
        .pos = 0
    };

    // 注册回调并启动播放
    (*player_bufq)->RegisterCallback(player_bufq, buffer_callback, &ctx);
    (*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING);
    
    // 提交初始缓冲块
    buffer_callback(player_bufq, &ctx);  

    printf("[INFO] Playback started. You should hear a 440Hz tone with background noise.\n");
    printf("[INFO] Press Ctrl+C to exit...\n");
    while(1) sleep(1);  // 保持程序运行

    // 清理资源（实际不会执行到此处）
    (*player_obj)->Destroy(player_obj);
    (*output_mix)->Destroy(output_mix);
    (*engine_obj)->Destroy(engine_obj);
    free(audio_buffer);
    return 0;
}
