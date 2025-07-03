#include "audio_async_blocking.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "algo_example.h"

struct AudioAsyncCtx {
    void* algo;
    pthread_t worker;
    pthread_mutex_t mutex;
    pthread_cond_t cond_start;
    pthread_cond_t cond_done;
    int busy;
    int exit_flag;
    int samples;
    float* inbuf;
    float* outbuf;
    int task_ready;
    int task_done;
};

// 工作线程函数
static void* worker_func(void* arg) {
    AudioAsyncCtx* ctx = (AudioAsyncCtx*)arg;
    pthread_mutex_lock(&ctx->mutex);
    while (!ctx->exit_flag) {
        while (!ctx->task_ready && !ctx->exit_flag) {
            pthread_cond_wait(&ctx->cond_start, &ctx->mutex);
        }
        if (ctx->exit_flag) break;
        // 处理任务
        algo_process(ctx->algo, ctx->inbuf, ctx->outbuf, ctx->samples);
        ctx->task_ready = 0;
        ctx->task_done = 1;
        pthread_cond_signal(&ctx->cond_done);
    }
    pthread_mutex_unlock(&ctx->mutex);
    return NULL;
}

AudioAsyncCtx* audio_async_create() {
    AudioAsyncCtx* ctx = (AudioAsyncCtx*)calloc(1, sizeof(AudioAsyncCtx));
    ctx->algo = algo_init();
    pthread_mutex_init(&ctx->mutex, NULL);
    pthread_cond_init(&ctx->cond_start, NULL);
    pthread_cond_init(&ctx->cond_done, NULL);
    pthread_create(&ctx->worker, NULL, worker_func, ctx);
    return ctx;
}

void audio_async_destroy(AudioAsyncCtx* ctx) {
    pthread_mutex_lock(&ctx->mutex);
    ctx->exit_flag = 1;
    pthread_cond_signal(&ctx->cond_start);
    pthread_mutex_unlock(&ctx->mutex);

    pthread_join(ctx->worker, NULL);
    algo_deinit(ctx->algo);
    pthread_mutex_destroy(&ctx->mutex);
    pthread_cond_destroy(&ctx->cond_start);
    pthread_cond_destroy(&ctx->cond_done);
    free(ctx->inbuf);
    free(ctx->outbuf);
    free(ctx);
}

int audio_async_process(AudioAsyncCtx* ctx, float* buffer_in, float* buffer_out, int samples) {
    pthread_mutex_lock(&ctx->mutex);
    if (ctx->busy) {
        pthread_mutex_unlock(&ctx->mutex);
        return -1; // 正在处理，不支持并发
    }
    ctx->busy = 1;
    ctx->samples = samples;
    if (!ctx->inbuf || ctx->samples != samples) {
        free(ctx->inbuf); free(ctx->outbuf);
        ctx->inbuf = (float*)malloc(sizeof(float) * samples);
        ctx->outbuf = (float*)malloc(sizeof(float) * samples);
    }
    memcpy(ctx->inbuf, buffer_in, sizeof(float) * samples);
    ctx->task_ready = 1;
    ctx->task_done = 0;
    pthread_cond_signal(&ctx->cond_start);

    // 阻塞等待处理完成
    while (!ctx->task_done) {
        pthread_cond_wait(&ctx->cond_done, &ctx->mutex);
    }
    memcpy(buffer_out, ctx->outbuf, sizeof(float) * samples);
    ctx->busy = 0;
    pthread_mutex_unlock(&ctx->mutex);
    return 0;
}
