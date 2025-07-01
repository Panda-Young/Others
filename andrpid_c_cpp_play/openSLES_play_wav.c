/* **************************************************************************
 * @Description: Single playback of WAV file with dynamic format configuration
 * @Version: 0.3.0
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
// Buffer size: number of frames per write (1024 frames)
#define FRAMES_PER_BUFFER 1024

// Structure for holding WAV format information
typedef struct {
    int sampleRate;
    short channels;
    short bitsPerSample;
    SLuint32 containerSize; // Container size in bits (usually same as bitsPerSample)
    SLuint32 channelMask;   // Channel configuration mask
} wav_format_t;

// Structure for holding PCM buffer context
typedef struct {
    unsigned char *data;   // Pointer to PCM data
    size_t size;           // Total data size in bytes
    size_t pos;            // Current read position in bytes
    int playback_finished; // Flag to indicate playback completion
    wav_format_t format;   // Audio format information
} pcm_chunk_ctx_t;

// Process buffer before writing to audio output
void preprocess_pcm_buffer(short *buffer, size_t samples)
{
    for (size_t i = 0; i < samples; ++i) {
        buffer[i] = -buffer[i]; // Amplitude inversion (replace if needed)
    }
}

// Load WAV file and extract PCM data and format information
unsigned char *load_wav(const char *filename, size_t *pcm_size, wav_format_t *format)
{
    printf("[LOG] Opening WAV file: %s\n", filename);
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("[ERROR] Failed to open WAV file!\n");
        return NULL;
    }

    // Read RIFF header
    char riffHeader[12];
    if (fread(riffHeader, 1, 12, fp) != 12) {
        printf("[ERROR] Failed to read RIFF header!\n");
        fclose(fp);
        return NULL;
    }

    // Verify RIFF header
    if (strncmp(riffHeader, "RIFF", 4) != 0 || strncmp(riffHeader + 8, "WAVE", 4) != 0) {
        printf("[ERROR] Not a valid WAV file!\n");
        fclose(fp);
        return NULL;
    }

    char chunkId[5] = {0};
    unsigned int chunkSize = 0;
    unsigned char *pcm_data = NULL;
    int fmt_found = 0;
    int data_found = 0;

    while (fread(chunkId, 1, 4, fp) == 4) {
        fread(&chunkSize, 4, 1, fp);

        if (strncmp(chunkId, "fmt ", 4) == 0) {
            // Parse fmt chunk
            short audioFormat, numChannels, bitsPerSample;
            unsigned int sampleRate;

            fread(&audioFormat, 2, 1, fp);
            fread(&numChannels, 2, 1, fp);
            fread(&sampleRate, 4, 1, fp);
            fseek(fp, 4, SEEK_CUR); // Skip byte rate
            fseek(fp, 2, SEEK_CUR); // Skip block align
            fread(&bitsPerSample, 2, 1, fp);

            // Validate format
            if (audioFormat != 1) { // 1 = PCM
                printf("[ERROR] Only PCM format is supported!\n");
                fclose(fp);
                return NULL;
            }

            // Set format parameters
            format->sampleRate = sampleRate;
            format->channels = numChannels;
            format->bitsPerSample = bitsPerSample;
            format->containerSize = bitsPerSample; // Usually same as bitsPerSample

            // Set channel mask based on number of channels
            if (numChannels == 1) {
                format->channelMask = SL_SPEAKER_FRONT_CENTER;
            } else if (numChannels == 2) {
                format->channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
            } else {
                printf("[WARN] Unsupported channel count: %d. Using default mask.\n", numChannels);
                format->channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
            }

            fmt_found = 1;
            printf("[LOG] WAV format: %d Hz, %d channels, %d bits\n",
                   sampleRate, numChannels, bitsPerSample);

            // Skip any remaining bytes in fmt chunk
            if (chunkSize > 16) {
                fseek(fp, chunkSize - 16, SEEK_CUR);
            }
        } else if (strncmp(chunkId, "data", 4) == 0) {
            // Read data chunk
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
            data_found = 1;
            break;
        } else {
            // Skip this chunk and move to next
            fseek(fp, chunkSize, SEEK_CUR);
        }
    }

    fclose(fp);

    if (!fmt_found) {
        printf("[ERROR] 'fmt ' chunk not found!\n");
        if (pcm_data)
            free(pcm_data);
        return NULL;
    }

    if (!data_found) {
        printf("[ERROR] 'data' chunk not found!\n");
        if (pcm_data)
            free(pcm_data);
        return NULL;
    }

    return pcm_data;
}

// Buffer queue callback for single playback
void bufferFinishedCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    pcm_chunk_ctx_t *ctx = (pcm_chunk_ctx_t *)context;

    // Calculate frame size in bytes
    int frameSize = (ctx->format.bitsPerSample / 8) * ctx->format.channels;

    // Calculate remaining data
    size_t bytes_left = ctx->size - ctx->pos;
    size_t bytes_to_write = FRAMES_PER_BUFFER * frameSize;

    // Adjust to actual available data
    if (bytes_to_write > bytes_left) {
        bytes_to_write = bytes_left;
    }

    // End of playback handling
    if (bytes_to_write == 0) {
        printf("[LOG] Playback completed\n");
        ctx->playback_finished = 1; // Set completion flag
        return;
    }

    // Allocate processing buffer
    unsigned char *proc_buffer = (unsigned char *)malloc(bytes_to_write);
    if (!proc_buffer) {
        printf("[ERROR] Memory allocation failed for %zu bytes\n", bytes_to_write);
        return;
    }

    // Copy data to processing buffer
    memcpy(proc_buffer, ctx->data + ctx->pos, bytes_to_write);

    // Only process if 16-bit PCM
    if (ctx->format.bitsPerSample == 16) {
        size_t samples = bytes_to_write / sizeof(short);
        preprocess_pcm_buffer((short *)proc_buffer, samples);
    }

    // Enqueue to audio buffer
    SLresult res = (*bq)->Enqueue(bq, proc_buffer, bytes_to_write);
    if (res != SL_RESULT_SUCCESS) {
        printf("[ERROR] Enqueue failed (code: %d)\n", res);
        free(proc_buffer);
        return;
    }

    ctx->pos += bytes_to_write;
    free(proc_buffer);
}

int main(int argc, char *argv[])
{
    printf("[LOG] ----------- Dynamic WAV Audio Player -----------\n");
    if (argc < 2) {
        printf("[ERROR] Usage: %s file.wav\n", argv[0]);
        return 1;
    }

    // Load PCM data and format info
    size_t pcm_size = 0;
    wav_format_t wav_format = {0};
    unsigned char *pcm_data = load_wav(argv[1], &pcm_size, &wav_format);
    if (!pcm_data)
        return 1;

    // Initialize OpenSL ES engine
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;
    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    SLObjectItf outputMixObject = NULL;
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    // Configure audio player based on WAV format
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 1};

    // Set PCM format dynamically - convert to milliHertz for OpenSL ES
    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        wav_format.channels,          // numChannels
        wav_format.sampleRate * 1000, // samplesPerSec (convert to milliHertz)
        wav_format.bitsPerSample,     // bitsPerSample
        wav_format.containerSize,     // containerSize
        wav_format.channelMask,       // channelMask
        SL_BYTEORDER_LITTLEENDIAN     // endianness
    };

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // Create audio player
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    SLObjectItf playerObject = NULL;
    SLPlayItf playerPlay;
    SLAndroidSimpleBufferQueueItf playerBufferQueue;
    SLresult res = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSnk, 1, ids, req);
    if (res != SL_RESULT_SUCCESS) {
        printf("[ERROR] CreateAudioPlayer failed: %d\n", res);
        (*outputMixObject)->Destroy(outputMixObject);
        (*engineObject)->Destroy(engineObject);
        free(pcm_data);
        return 1;
    }

    (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &playerBufferQueue);

    // Initialize playback context with format info
    pcm_chunk_ctx_t audioCtx = {pcm_data, pcm_size, 0, 0, wav_format};
    (*playerBufferQueue)->RegisterCallback(playerBufferQueue, bufferFinishedCallback, &audioCtx);

    // Start playback
    (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
    printf("[LOG] Starting playback...\n");

    // Manually trigger first buffer
    bufferFinishedCallback(playerBufferQueue, &audioCtx);

    // Wait for playback completion
    while (!audioCtx.playback_finished) {
        usleep(100000); // Check every 100ms
    }

    // Cleanup resources
    printf("[LOG] Releasing resources...\n");
    (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
    (*playerObject)->Destroy(playerObject);
    (*outputMixObject)->Destroy(outputMixObject);
    (*engineObject)->Destroy(engineObject);
    free(pcm_data);

    printf("[LOG] Playback finished successfully\n");
    return 0;
}

/* Compile With:
    -lOpenSLES
*/
