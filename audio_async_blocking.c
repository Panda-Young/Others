#include "audio_async_blocking.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>  // Added for debug logging
#include "algo_example.h"

struct AudioAsyncCtx {
    void* algo;                 // Algorithm instance handle
    pthread_t worker;           // Worker thread handle
    pthread_mutex_t mutex;      // Mutex for thread synchronization
    pthread_cond_t cond_start;  // Condition to signal task start
    pthread_cond_t cond_done;   // Condition to signal task completion
    int busy;                   // Flag indicating processing state (1=busy, 0=idle)
    int exit_flag;              // Flag to terminate worker thread (1=exit)
    int samples;                // Number of samples per buffer
    float* inbuf;               // Input buffer for processing
    float* outbuf;              // Output buffer for results
    int task_ready;             // Task ready flag (1=ready)
    int task_done;              // Task completion flag (1=done)
};

// Worker thread function
static void* worker_func(void* arg) {
    AudioAsyncCtx* ctx = (AudioAsyncCtx*)arg;
    pthread_mutex_lock(&ctx->mutex);
    printf("[Worker] Thread started. Waiting for tasks...\n");

    while (!ctx->exit_flag) {
        // Wait for task readiness or exit signal
        while (!ctx->task_ready && !ctx->exit_flag) {
            printf("[Worker] Entering wait state...\n");
            pthread_cond_wait(&ctx->cond_start, &ctx->mutex);
        }
        
        if (ctx->exit_flag) {
            printf("[Worker] Exit flag detected. Breaking loop\n");
            break;
        }

        printf("[Worker] Task received. Processing %d samples...\n", ctx->samples);
        // Process audio data using algorithm
        algo_process(ctx->algo, ctx->inbuf, ctx->outbuf, ctx->samples);
        
        // Update task state
        ctx->task_ready = 0;
        ctx->task_done = 1;
        printf("[Worker] Task completed. Signaling main thread\n");
        
        // Notify main thread
        pthread_cond_signal(&ctx->cond_done);
    }
    
    pthread_mutex_unlock(&ctx->mutex);
    printf("[Worker] Thread exiting\n");
    return NULL;
}

// Initialize async processing context
AudioAsyncCtx* audio_async_create() {
    printf("[Main] Creating async context...\n");
    AudioAsyncCtx* ctx = (AudioAsyncCtx*)calloc(1, sizeof(AudioAsyncCtx));
    
    // Initialize algorithm instance
    ctx->algo = algo_init();
    printf("[Main] Algorithm initialized\n");
    float param2 = -5.0f;
    algo_set_param(ctx->algo, ALGO_PARAM2, &param2, sizeof(param2));
    
    // Initialize synchronization primitives
    pthread_mutex_init(&ctx->mutex, NULL);
    pthread_cond_init(&ctx->cond_start, NULL);
    pthread_cond_init(&ctx->cond_done, NULL);
    printf("[Main] Mutex/Cond initialized\n");
    
    // Start worker thread
    pthread_create(&ctx->worker, NULL, worker_func, ctx);
    printf("[Main] Worker thread started\n");
    return ctx;
}

// Cleanup resources
void audio_async_destroy(AudioAsyncCtx* ctx) {
    printf("[Main] Destroying context...\n");
    pthread_mutex_lock(&ctx->mutex);
    
    // Signal worker thread to exit
    ctx->exit_flag = 1;
    pthread_cond_signal(&ctx->cond_start);
    pthread_mutex_unlock(&ctx->mutex);
    printf("[Main] Exit flag set. Waiting for thread...\n");
    
    // Wait for thread termination
    pthread_join(ctx->worker, NULL);
    printf("[Main] Worker thread joined\n");
    
    // Release resources
    algo_deinit(ctx->algo);
    pthread_mutex_destroy(&ctx->mutex);
    pthread_cond_destroy(&ctx->cond_start);
    pthread_cond_destroy(&ctx->cond_done);
    free(ctx->inbuf);
    free(ctx->outbuf);
    free(ctx);
    printf("[Main] Resources released\n");
}

// Process audio data (blocking call)
int audio_async_process(AudioAsyncCtx* ctx, float* buffer_in, float* buffer_out, int samples) {
    pthread_mutex_lock(&ctx->mutex);
    printf("[Main] Process request: %d samples\n", samples);
    
    // Check for concurrent access
    if (ctx->busy) {
        pthread_mutex_unlock(&ctx->mutex);
        printf("[Main] ERROR: Concurrent access detected!\n");
        return -1; 
    }
    
    ctx->busy = 1;
    ctx->samples = samples;
    
    // Reallocate buffers if size changed
    if (!ctx->inbuf || ctx->samples != samples) {
        printf("[Main] Reallocating buffers (%d samples)\n", samples);
        free(ctx->inbuf); 
        free(ctx->outbuf);
        ctx->inbuf = (float*)malloc(sizeof(float) * samples);
        ctx->outbuf = (float*)malloc(sizeof(float) * samples);
    }
    
    // Prepare task data
    memcpy(ctx->inbuf, buffer_in, sizeof(float) * samples);
    ctx->task_ready = 1;
    ctx->task_done = 0;
    printf("[Main] Task prepared. Signaling worker\n");
    
    // Wake worker thread
    pthread_cond_signal(&ctx->cond_start);
    
    // Wait for completion
    printf("[Main] Waiting for task completion...\n");
    while (!ctx->task_done) {
        pthread_cond_wait(&ctx->cond_done, &ctx->mutex);
    }
    
    // Return processed data
    memcpy(buffer_out, ctx->outbuf, sizeof(float) * samples);
    ctx->busy = 0;
    pthread_mutex_unlock(&ctx->mutex);
    printf("[Main] Task completed successfully\n");
    return 0;
}
