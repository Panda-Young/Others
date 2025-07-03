#ifndef AUDIO_ASYNC_BLOCKING_H
#define AUDIO_ASYNC_BLOCKING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AudioAsyncCtx AudioAsyncCtx;

// 创建上下文，内部自动初始化算法句柄和工作线程
AudioAsyncCtx* audio_async_create();

// 销毁上下文
void audio_async_destroy(AudioAsyncCtx* ctx);

// 提交任务（异步），主线程会阻塞直到algo_process处理完成
// buffer_in, buffer_out: 输入输出缓冲区, samples: 采样点数
// 返回0表示成功，负数表示失败
int audio_async_process(AudioAsyncCtx* ctx, float* buffer_in, float* buffer_out, int samples);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_ASYNC_BLOCKING_H
