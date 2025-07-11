/* **************************************************************************
 * @Description: loop play wav file
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * @Date: 2025-06-23 14:12:08
 * @Copyright (c) 2025 by @AAC Technologies, All Rights Reserved.
 **************************************************************************/

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Chunk header size for WAV parsing
#define CHUNK_HEADER_SIZE 8
// Sample size in bytes for PCM 16-bit
#define SAMPLE_SIZE_BYTES 2
// Buffer size: number of samples per write (1024 samples)
#define SAMPLES_PER_BUFFER 1024

// Structure for holding PCM buffer context
typedef struct {
    unsigned char *data; // Pointer to PCM data
    size_t size;         // Total data size in bytes
    size_t pos;          // Current read position in bytes
} pcm_chunk_ctx_t;

// Process buffer before writing to audio output (example: invert samples)
void preprocess_pcm_buffer(short *buffer, size_t samples)
{
    // Example: invert amplitude (can be replaced by other preprocessing)
    for (size_t i = 0; i < samples; ++i) {
        buffer[i] = -buffer[i];
    }
}

// Read WAV file, find data chunk, and return pointer and size of PCM data
unsigned char *load_wav(const char *filename, size_t *pcm_size)
{
    printf("[LOG] Opening WAV file: %s\n", filename);
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("[ERROR] Failed to open WAV file!\n");
        return NULL;
    }

    // Skip RIFF header and WAVE identifier (12 bytes)
    fseek(fp, 12, SEEK_SET);

    char chunkId[5] = {0};
    unsigned int chunkSize = 0;
    unsigned char *pcm_data = NULL;

    // Search for "data" chunk
    while (fread(chunkId, 1, 4, fp) == 4) {
        fread(&chunkSize, 4, 1, fp);

        printf("[LOG] Found chunk: %.4s (size: %u)\n", chunkId, chunkSize);
        if (strncmp(chunkId, "data", 4) == 0) {
            pcm_data = (unsigned char *)malloc(chunkSize);
            if (!pcm_data) {
                printf("[ERROR] Failed to allocate PCM buffer!\n");
                fclose(fp);
                return NULL;
            }
            size_t read = fread(pcm_data, 1, chunkSize, fp);
            if (read != chunkSize) {
                printf("[ERROR] Failed to read PCM data!\n");
                free(pcm_data);
                fclose(fp);
                return NULL;
            }
            *pcm_size = chunkSize;
            printf("[LOG] PCM data loaded (size: %zu bytes)\n", *pcm_size);
            break;
        } else {
            // Skip this chunk and move to next
            fseek(fp, chunkSize, SEEK_CUR);
        }
    }

    fclose(fp);
    if (!pcm_data) {
        printf("[ERROR] 'data' chunk not found in WAV file!\n");
    }
    return pcm_data;
}

// Buffer finished callback: fill next buffer with processed data
void bufferFinishedCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    pcm_chunk_ctx_t *ctx = (pcm_chunk_ctx_t *)context;

    // Calculate bytes left
    size_t bytes_left = ctx->size - ctx->pos;
    size_t chunk_bytes = SAMPLES_PER_BUFFER * SAMPLE_SIZE_BYTES;
    size_t bytes_to_write = (bytes_left >= chunk_bytes) ? chunk_bytes : bytes_left;

    // If we reach the end, start again for looping
    if (bytes_to_write == 0) {
        printf("[LOG] Looping to start of PCM data\n");
        ctx->pos = 0;
        bytes_left = ctx->size;
        bytes_to_write = (bytes_left >= chunk_bytes) ? chunk_bytes : bytes_left;
    }

    // Prepare buffer for processing (16-bit PCM)
    short *proc_buffer = (short *)malloc(bytes_to_write);
    if (!proc_buffer) {
        printf("[ERROR] Memory allocation failed for processing buffer\n");
        return;
    }
    memcpy(proc_buffer, ctx->data + ctx->pos, bytes_to_write);

    size_t samples = bytes_to_write / SAMPLE_SIZE_BYTES;

    // Preprocess audio buffer before playback
    preprocess_pcm_buffer(proc_buffer, samples);

    // Enqueue processed buffer to OpenSL ES for playback
    SLresult res = (*bq)->Enqueue(bq, proc_buffer, bytes_to_write);
    if (res != SL_RESULT_SUCCESS) {
        printf("[ERROR] Failed to enqueue buffer to audio queue (code: %d)\n", res);
        free(proc_buffer);
        return;
    }
    printf("[LOG] Enqueued %zu bytes (%zu samples) for playback at position %zu\n", bytes_to_write, samples, ctx->pos);

    ctx->pos += bytes_to_write;
    // Free buffer after it is queued (OpenSL ES copies internally)
    free(proc_buffer);
}

int main(int argc, char *argv[])
{
    printf("[LOG] ----------- WAV Audio Player (OpenSL ES) -----------\n");
    if (argc < 2) {
        printf("[ERROR] Usage: %s file.wav\n", argv[0]);
        return 1;
    }
    printf("[LOG] Loading WAV PCM data...\n");
    size_t pcm_size = 0;
    unsigned char *pcm_data = load_wav(argv[1], &pcm_size);
    if (!pcm_data) {
        printf("[ERROR] Could not load WAV file.\n");
        return 1;
    }

    // Audio engine and objects
    printf("[LOG] Initializing OpenSL ES engine...\n");
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;
    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    SLObjectItf outputMixObject = NULL;
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    // Configure buffer queue and PCM format (hardcoded for 44100Hz, mono, 16bit)
    printf("[LOG] Configuring audio player...\n");
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 1};
    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,           // formatType
        1,                           // numChannels (mono)
        SL_SAMPLINGRATE_44_1,        // samplesPerSec
        SL_PCMSAMPLEFORMAT_FIXED_16, // bitsPerSample
        SL_PCMSAMPLEFORMAT_FIXED_16, // containerSize
        SL_SPEAKER_FRONT_CENTER,     // channelMask
        SL_BYTEORDER_LITTLEENDIAN    // endianness
    };
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    SLObjectItf playerObject = NULL;
    SLPlayItf playerPlay;
    SLAndroidSimpleBufferQueueItf playerBufferQueue;
    (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSnk, 1, ids, req);
    (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &playerBufferQueue);

    // Prepare playback context
    pcm_chunk_ctx_t audioCtx = {pcm_data, pcm_size, 0};

    // Register buffer queue callback
    printf("[LOG] Registering buffer callback...\n");
    (*playerBufferQueue)->RegisterCallback(playerBufferQueue, bufferFinishedCallback, &audioCtx);

    // Start playback state
    (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);

    // Enqueue the first processed buffer
    printf("[LOG] Enqueue initial buffer for playback...\n");
    size_t first_chunk = (pcm_size >= SAMPLES_PER_BUFFER * SAMPLE_SIZE_BYTES) ? SAMPLES_PER_BUFFER * SAMPLE_SIZE_BYTES : pcm_size;
    short *proc_buffer = (short *)malloc(first_chunk);
    memcpy(proc_buffer, pcm_data, first_chunk);
    preprocess_pcm_buffer(proc_buffer, first_chunk / SAMPLE_SIZE_BYTES);
    (*playerBufferQueue)->Enqueue(playerBufferQueue, proc_buffer, first_chunk);
    audioCtx.pos += first_chunk;
    free(proc_buffer);

    printf("[LOG] Playback started. Press Ctrl+C to stop.\n");

    // Main loop: keep process alive
    while (1) {
        sleep(1);
    }

    // Cleanup (not reached here)
    (*playerObject)->Destroy(playerObject);
    (*outputMixObject)->Destroy(outputMixObject);
    (*engineObject)->Destroy(engineObject);
    free(pcm_data);
    printf("[LOG] Exiting player.\n");
    return 0;
}

/* Compile With:
    -lOpenSLES
*/
