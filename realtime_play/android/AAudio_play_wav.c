/* **************************************************************************
 * @Description: WAV file playback using AAudio API (Android NDK)
 * @Version: 1.2.0
 * @Author: pandapan@aactechnologies.com
 * @Date: 2025-06-23 14:12:08
 * @Copyright (c) 2025 by @AAC Technologies, All Rights Reserved.
 **************************************************************************/

#include <aaudio/AAudio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Reuse existing WAV parsing definitions
#define CHUNK_HEADER_SIZE 8
#define FRAMES_PER_BUFFER 1024

// WAV format structure (unchanged)
typedef struct {
    int sampleRate;
    short channels;
    short bitsPerSample;
    int containerSize; // Convert to int for AAudio compatibility
    int channelMask;   // AAudio uses different channel masks
} wav_format_t;

// PCM context with AAudio-specific additions
typedef struct {
    unsigned char *data;
    size_t size;
    size_t pos;
    bool playback_finished; // Use bool for state clarity
    wav_format_t format;
} pcm_chunk_ctx_t;

// PCM processing function (reused)
void preprocess_pcm_buffer(short *buffer, size_t samples)
{
    for (size_t i = 0; i < samples; ++i) {
        buffer[i] = -buffer[i];
    }
}
void preprocess_float_buffer(float *buffer, size_t samples)
{
    for (size_t i = 0; i < samples; ++i) {
        buffer[i] = -buffer[i];
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
            if (audioFormat != 1 && audioFormat != 3) { // 1 = PCM, 3 = IEEE float
                printf("[ERROR] Only PCM and IEEE float formats are supported!\n");
                fclose(fp);
                return NULL;
            }

            // Set format parameters
            format->sampleRate = sampleRate;
            format->channels = numChannels;
            format->bitsPerSample = bitsPerSample;
            format->containerSize = bitsPerSample; // Usually same as bitsPerSample
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

// AAudio data callback (replaces OpenSL ES callback)
aaudio_data_callback_result_t audioCallback(
    AAudioStream *stream,
    void *userData,
    void *audioData,
    int32_t numFrames)
{
    pcm_chunk_ctx_t *ctx = (pcm_chunk_ctx_t *)userData;
    if (!ctx) {
        fprintf(stderr, "[ERROR] Null context in callback\n");
        return AAUDIO_CALLBACK_RESULT_STOP;
    }

    const int frameSize = (ctx->format.bitsPerSample / 8) * ctx->format.channels;
    const size_t bytesNeeded = numFrames * frameSize;
    const size_t bytesAvailable = ctx->size - ctx->pos;

    // Check playback completion
    if (bytesAvailable == 0) {
        printf("[LOG] Playback completed\n");
        ctx->playback_finished = true;
        return AAUDIO_CALLBACK_RESULT_STOP; // Signal AAudio to stop
    }

    // Calculate actual bytes to copy
    const size_t bytesToCopy = (bytesNeeded > bytesAvailable) ? bytesAvailable : bytesNeeded;

    if (ctx->format.bitsPerSample == 16) {
        memcpy(audioData, ctx->data + ctx->pos, bytesToCopy);
        const size_t samplesToProcess = bytesToCopy / sizeof(short);
        preprocess_pcm_buffer((short *)audioData, samplesToProcess);
    } else if (ctx->format.bitsPerSample == 32) {
        memcpy(audioData, ctx->data + ctx->pos, bytesToCopy);
        const size_t samplesToProcess = bytesToCopy / sizeof(float);
        preprocess_float_buffer((float *)audioData, samplesToProcess);
    } else {
        memcpy(audioData, ctx->data + ctx->pos, bytesToCopy);
    }
    ctx->pos += bytesToCopy;

    // Zero-pad if partial frame
    if (bytesToCopy < bytesNeeded) {
        size_t paddingBytes = bytesNeeded - bytesToCopy;
        memset((char *)audioData + bytesToCopy, 0, paddingBytes);
    }

    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

// Error callback for AAudio
void errorCallback(
    AAudioStream *stream,
    void *userData,
    aaudio_result_t error)
{
    fprintf(stderr, "[ERROR] AAudio error: %s (%d)\n", AAudio_convertResultToText(error), error);
    // Set playback finished flag on error
    pcm_chunk_ctx_t *ctx = (pcm_chunk_ctx_t *)userData;
    if (ctx) {
        ctx->playback_finished = true;
    }
}

int main(int argc, char *argv[])
{
    printf("[LOG] ----------- AAudio WAV Player -----------\n");
    if (argc < 2) {
        printf("[ERROR] Usage: %s file.wav\n", argv[0]);
        return 1;
    }

    // Load WAV file
    size_t pcm_size = 0;
    wav_format_t wav_format = {0};
    unsigned char *pcm_data = load_wav(argv[1], &pcm_size, &wav_format);
    if (!pcm_data)
        return 1;

    // Initialize playback context EARLY
    pcm_chunk_ctx_t audioCtx = {
        .data = pcm_data,
        .size = pcm_size,
        .pos = 0,
        .playback_finished = false,
        .format = wav_format};
    aaudio_format_t aaudioFormat;
    if (wav_format.bitsPerSample == 32) {
        aaudioFormat = AAUDIO_FORMAT_PCM_FLOAT;
    } else {
        aaudioFormat = AAUDIO_FORMAT_PCM_I16;
    }
    // Initialize AAudio builder
    AAudioStreamBuilder *builder;
    AAudio_createStreamBuilder(&builder);

    // Configure audio parameters
    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setSampleRate(builder, wav_format.sampleRate);
    AAudioStreamBuilder_setChannelCount(builder, wav_format.channels);
    AAudioStreamBuilder_setFormat(builder, aaudioFormat);

    // Set correct frames per callback
    AAudioStreamBuilder_setFramesPerDataCallback(builder, FRAMES_PER_BUFFER);

    // FIX: Set callbacks with user data BEFORE opening stream
    AAudioStreamBuilder_setDataCallback(builder, audioCallback, &audioCtx);
    AAudioStreamBuilder_setErrorCallback(builder, errorCallback, &audioCtx);

    // Create AAudio stream
    AAudioStream *stream;
    aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &stream);
    if (result != AAUDIO_OK) {
        fprintf(stderr, "[ERROR] Stream open failed: %s\n", AAudio_convertResultToText(result));
        AAudioStreamBuilder_delete(builder);
        free(pcm_data);
        return 1;
    }

    // Set buffer size AFTER stream creation
    AAudioStream_setBufferSizeInFrames(stream, FRAMES_PER_BUFFER * 2); // Double buffering

    // Start playback
    result = AAudioStream_requestStart(stream);
    if (result != AAUDIO_OK) {
        fprintf(stderr, "[ERROR] Start failed: %s\n", AAudio_convertResultToText(result));
        goto cleanup;
    }
    printf("[LOG] Playback started...\n");

    // Wait for completion with timeout
    const int MAX_WAIT_SECONDS = 30;
    int waitCount = 0;
    while (!audioCtx.playback_finished) {
        usleep(100000); // 100ms polling
        waitCount++;

        // Timeout after 30 seconds
        if (waitCount > MAX_WAIT_SECONDS * 10) {
            fprintf(stderr, "[ERROR] Playback timeout reached\n");
            break;
        }
    }

cleanup:
    // Release resources
    printf("[LOG] Releasing resources...\n");
    if (stream) {
        AAudioStream_requestStop(stream);
        AAudioStream_close(stream);
    }
    AAudioStreamBuilder_delete(builder);
    free(pcm_data);

    printf("[LOG] Playback finished %s\n",
           audioCtx.playback_finished ? "successfully" : "with errors");
    return 0;
}

/* Compile With:
    -lAAudio
*/
