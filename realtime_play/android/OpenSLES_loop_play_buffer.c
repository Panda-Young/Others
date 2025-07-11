/* **************************************************************************
 * @Description: loop play buffer example
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * @Date: 2025-06-23 16:07:47
 * @Copyright (c) 2025 by @AAC Technologies, All Rights Reserved.
 **************************************************************************/

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <math.h> // Include math header for waveform generation
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Include time header for random seed
#include <unistd.h>

// Audio parameter configuration
#define SAMPLE_RATE 44100      // 44.1kHz
#define CHANNELS 1             // Mono
#define SAMPLE_SIZE_BYTES 2    // 16-bit PCM
#define SAMPLES_PER_BUFFER 512 // Number of samples processed each time

// Audio buffer context structure
typedef struct {
    const short *data; // PCM data pointer
    size_t size;       // Total data size (bytes)
    size_t pos;        // Current read position (bytes)
} pcm_buffer_ctx_t;

// Preprocessing function example (can be replaced with custom algorithm)
void process_audio_buffer(short *buffer, size_t samples)
{
    // Example: Invert audio phase (replace with your algorithm when in use)
    for (size_t i = 0; i < samples; ++i) {
        buffer[i] = -buffer[i];
    }
}

// Audio queue callback function
void buffer_callback(SLAndroidSimpleBufferQueueItf bq, void *ctx_ptr)
{
    pcm_buffer_ctx_t *ctx = (pcm_buffer_ctx_t *)ctx_ptr;
    const size_t chunk_bytes = SAMPLES_PER_BUFFER * SAMPLE_SIZE_BYTES;

    // Calculate the number of bytes available to read (support loop playback)
    size_t bytes_left = ctx->size - ctx->pos;
    size_t bytes_to_copy = (bytes_left >= chunk_bytes) ? chunk_bytes : bytes_left;

    if (bytes_to_copy == 0) { // Reached the end of the buffer, reset position
        ctx->pos = 0;
        bytes_to_copy = (ctx->size >= chunk_bytes) ? chunk_bytes : ctx->size;
    }

    // Create processing buffer
    short *proc_buffer = (short *)malloc(chunk_bytes);
    if (!proc_buffer) {
        printf("[ERROR] Failed to allocate processing buffer\n");
        return;
    }

    // Copy and process audio data
    memcpy(proc_buffer, ctx->data + ctx->pos / SAMPLE_SIZE_BYTES, bytes_to_copy);
    if (bytes_to_copy < chunk_bytes) { // Fill remaining space
        memset((char *)proc_buffer + bytes_to_copy, 0, chunk_bytes - bytes_to_copy);
    }
    process_audio_buffer(proc_buffer, SAMPLES_PER_BUFFER);

    // Submit processed audio data
    SLresult res = (*bq)->Enqueue(bq, proc_buffer, chunk_bytes);
    if (res != SL_RESULT_SUCCESS) {
        printf("[ERROR] Enqueue failed: %d\n", res);
    } else {
        ctx->pos = (ctx->pos + bytes_to_copy) % ctx->size; // Update position (loop)
    }
    free(proc_buffer);
}

// Generate white noise sample (-32768 to 32767)
short generate_white_noise()
{
    return (short)(rand() % 65536 - 32768);
}

// Generate sine wave sample (440Hz A4 pitch)
short generate_sine_wave(size_t index)
{
    double frequency = 440.0; // A4 pitch
    double sample_rate = (double)SAMPLE_RATE;
    double sin_value = sin(2 * M_PI * frequency * index / sample_rate);
    return (short)(sin_value * 32767);
}

int main()
{
    printf("[INFO] Initializing audio player with random samples...\n");

    // Initialize random seed
    srand((unsigned int)time(NULL));

    // Create an audio buffer containing sound (1 second duration)
    const size_t buffer_samples = SAMPLE_RATE * CHANNELS; // 44100 samples
    short *audio_buffer = (short *)malloc(buffer_samples * SAMPLE_SIZE_BYTES);

    // Fill the buffer (mix white noise and sine wave)
    for (size_t i = 0; i < buffer_samples; ++i) {
        // Mix noise and sine wave (ratio adjustable)
        short noise = (short)(generate_white_noise() * 0.2); // 20% white noise
        short tone = generate_sine_wave(i);                  // 440Hz sine wave

        // Limit within 16-bit range to prevent overflow
        int mixed = noise + tone;
        if (mixed > 32767)
            mixed = 32767;
        if (mixed < -32768)
            mixed = -32768;

        audio_buffer[i] = (short)mixed * 0.175;
    }

    // Initialize OpenSL engine
    SLObjectItf engine_obj, output_mix, player_obj;
    SLEngineItf engine;
    SLresult res = slCreateEngine(&engine_obj, 0, NULL, 0, NULL, NULL);
    (*engine_obj)->Realize(engine_obj, SL_BOOLEAN_FALSE);
    (*engine_obj)->GetInterface(engine_obj, SL_IID_ENGINE, &engine);

    // Create output mixer
    (*engine)->CreateOutputMix(engine, &output_mix, 0, NULL, NULL);
    (*output_mix)->Realize(output_mix, SL_BOOLEAN_FALSE);

    // Configure audio source
    SLDataLocator_AndroidSimpleBufferQueue bufq_loc = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM pcm_format = {
        SL_DATAFORMAT_PCM,
        CHANNELS,
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER,
        SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audio_src = {&bufq_loc, &pcm_format};

    // Configure audio sink
    SLDataLocator_OutputMix outmix_loc = {
        SL_DATALOCATOR_OUTPUTMIX, output_mix};
    SLDataSink audio_sink = {&outmix_loc, NULL};

    // Create player
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    (*engine)->CreateAudioPlayer(engine, &player_obj, &audio_src, &audio_sink,
                                 1, ids, req);
    (*player_obj)->Realize(player_obj, SL_BOOLEAN_FALSE);

    // Get playback and buffer queue interfaces
    SLPlayItf player;
    SLAndroidSimpleBufferQueueItf player_bufq;
    (*player_obj)->GetInterface(player_obj, SL_IID_PLAY, &player);
    (*player_obj)->GetInterface(player_obj, SL_IID_BUFFERQUEUE, &player_bufq);

    // Set playback context
    pcm_buffer_ctx_t ctx = {
        .data = audio_buffer,
        .size = buffer_samples * SAMPLE_SIZE_BYTES,
        .pos = 0};

    // Register callback and start playback
    (*player_bufq)->RegisterCallback(player_bufq, buffer_callback, &ctx);
    (*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING);

    // Submit initial buffer block
    buffer_callback(player_bufq, &ctx);

    printf("[INFO] Playback started. You should hear a 440Hz tone with background noise.\n");
    printf("[INFO] Press Ctrl+C to exit...\n");
    while (1)
        sleep(1); // Keep the program running

    // Cleanup resources (this part will not actually execute)
    (*player_obj)->Destroy(player_obj);
    (*output_mix)->Destroy(output_mix);
    (*engine_obj)->Destroy(engine_obj);
    free(audio_buffer);
    return 0;
}
