#include "portaudio.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// WAV file chunk identifiers
#define RIFF_HEADER "RIFF"
#define WAVE_HEADER "WAVE"
#define FMT_CHUNK "fmt "
#define DATA_CHUNK "data"

// Audio format information structure
typedef struct {
    int sampleRate;
    short channels;
    short bitsPerSample;
} wav_format_t;

// Audio data context
typedef struct {
    unsigned char *data;
    size_t size;
    size_t pos;
    wav_format_t format;
} AudioData;

// Process buffer before writing to audio output
void preprocess_pcm_buffer(short *buffer, size_t samples)
{
    for (size_t i = 0; i < samples; ++i) {
        buffer[i] = -buffer[i]; // Amplitude inversion
    }
}
void preprocess_float_buffer(float *buffer, size_t samples)
{
    for (size_t i = 0; i < samples; ++i) {
        buffer[i] = -buffer[i]; // Amplitude inversion
    }
}

// PortAudio callback function
static int paCallback(
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData)
{
    AudioData *ctx = (AudioData *)userData;

    // Calculate bytes per frame
    int bytesPerFrame = (ctx->format.bitsPerSample / 8) * ctx->format.channels;
    size_t bytesToCopy = frameCount * bytesPerFrame;
    size_t bytesRemaining = ctx->size - ctx->pos;
    if (bytesToCopy > bytesRemaining) {
        bytesToCopy = bytesRemaining;
    }

    if (bytesToCopy > 0) {
        // Copy data to output buffer
        memcpy(output, ctx->data + ctx->pos, bytesToCopy);

        // Process audio data based on format
        if (ctx->format.bitsPerSample == 16) {
            // 16-bit PCM processing (amplitude inversion)
            size_t samples = bytesToCopy / sizeof(short);
            printf("[PROCESS] Applying amplitude inversion to %zu 16-bit samples\n", samples);
            preprocess_pcm_buffer((short *)output, samples);
        } else if (ctx->format.bitsPerSample == 32) {
            // 32-bit float processing (amplitude inversion)
            size_t samples = bytesToCopy / sizeof(float);
            printf("[PROCESS] Applying amplitude inversion to %zu 32-bit float samples\n", samples);
            preprocess_float_buffer((float *)output, samples);
        }

        ctx->pos += bytesToCopy;
    }

    // Check if playback is complete
    if (ctx->pos >= ctx->size) {
        return paComplete;
    }

    return paContinue;
}

// Load WAV file (supports non-standard formats)
unsigned char *load_wav(const char *filename, size_t *pcm_size, wav_format_t *format)
{
    printf("[INFO] Opening WAV file: %s\n", filename);
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "[ERROR] Failed to open WAV file!\n");
        return NULL;
    }
    printf("[DEBUG] File opened successfully\n");

    // Read RIFF header
    char riffHeader[12];
    if (fread(riffHeader, 1, 12, fp) != 12) {
        fprintf(stderr, "[ERROR] Failed to read RIFF header!\n");
        fclose(fp);
        return NULL;
    }
    printf("[DEBUG] RIFF header read\n");

    // Verify RIFF header
    if (strncmp(riffHeader, RIFF_HEADER, 4) != 0 ||
        strncmp(riffHeader + 8, WAVE_HEADER, 4) != 0) {
        fprintf(stderr, "[ERROR] Not a valid WAV file!\n");
        fclose(fp);
        return NULL;
    }
    printf("[DEBUG] Valid WAV header confirmed\n");

    char chunkId[5] = {0};
    uint32_t chunkSize = 0;
    unsigned char *pcm_data = NULL;
    bool fmt_found = false;
    bool data_found = false;

    printf("[DEBUG] Scanning chunks...\n");

    // Iterate through all chunks
    while (fread(chunkId, 1, 4, fp) == 4) {
        fread(&chunkSize, 4, 1, fp);
        chunkId[4] = '\0'; // Ensure string termination

        printf("[DEBUG] Found chunk: %s, size: %u bytes\n", chunkId, chunkSize);

        if (strcmp(chunkId, FMT_CHUNK) == 0) {
            printf("[INFO] Processing format chunk\n");

            // Parse format chunk
            uint16_t audioFormat, numChannels, bitsPerSample;
            uint32_t sampleRate;

            fread(&audioFormat, 2, 1, fp);
            fread(&numChannels, 2, 1, fp);
            fread(&sampleRate, 4, 1, fp);
            fseek(fp, 4, SEEK_CUR); // Skip byte rate
            fseek(fp, 2, SEEK_CUR); // Skip block align
            fread(&bitsPerSample, 2, 1, fp);

            // Validate format
            if (audioFormat != 1 && audioFormat != 3) { // 1 = PCM, 3 = IEEE float
                fprintf(stderr, "[ERROR] Only PCM and IEEE float formats are supported! Found format: %d\n",
                        audioFormat);
                fclose(fp);
                return NULL;
            }

            // Set format parameters
            format->sampleRate = sampleRate;
            format->channels = numChannels;
            format->bitsPerSample = bitsPerSample;

            fmt_found = true;
            printf("[INFO] WAV format: %d Hz, %d channels, %d bits\n",
                   sampleRate, numChannels, bitsPerSample);

            // Skip remaining bytes in format chunk if any
            if (chunkSize > 16) {
                size_t skipBytes = chunkSize - 16;
                fseek(fp, skipBytes, SEEK_CUR);
                printf("[DEBUG] Skipping %zu extra bytes in fmt chunk\n", skipBytes);
            }
        } else if (strcmp(chunkId, DATA_CHUNK) == 0) {
            printf("[INFO] Processing data chunk\n");

            // Read data chunk
            pcm_data = (unsigned char *)malloc(chunkSize);
            if (!pcm_data) {
                fprintf(stderr, "[ERROR] Failed to allocate %u bytes for PCM buffer!\n", chunkSize);
                fclose(fp);
                return NULL;
            }
            printf("[DEBUG] Allocated %u bytes for PCM data\n", chunkSize);

            size_t read = fread(pcm_data, 1, chunkSize, fp);
            if (read != chunkSize) {
                fprintf(stderr, "[ERROR] Failed to read PCM data! Expected %u, got %zu\n",
                        chunkSize, read);
                free(pcm_data);
                fclose(fp);
                return NULL;
            }

            *pcm_size = chunkSize;
            printf("[INFO] PCM data loaded (size: %zu bytes)\n", *pcm_size);
            data_found = true;
            break; // Stop searching after finding data chunk
        } else {
            // Skip unknown chunk
            printf("[INFO] Skipping unknown chunk: %s (%u bytes)\n", chunkId, chunkSize);
            fseek(fp, chunkSize, SEEK_CUR);
        }
    }

    fclose(fp);
    printf("[DEBUG] File closed\n");

    if (!fmt_found) {
        fprintf(stderr, "[ERROR] 'fmt ' chunk not found!\n");
        if (pcm_data) {
            free(pcm_data);
            printf("[DEBUG] Freed PCM data buffer\n");
        }
        return NULL;
    }

    if (!data_found) {
        fprintf(stderr, "[ERROR] 'data' chunk not found!\n");
        if (pcm_data) {
            free(pcm_data);
            printf("[DEBUG] Freed PCM data buffer\n");
        }
        return NULL;
    }

    return pcm_data;
}

// Print PortAudio device information
void print_device_info(PaDeviceIndex device)
{
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(device);
    if (!deviceInfo) {
        printf("[WARN] Failed to get device info for index %d\n", device);
        return;
    }

    printf("[INFO] Audio Device #%d: %s\n", device, deviceInfo->name);
    printf("       Input channels: %d, Output channels: %d\n",
           deviceInfo->maxInputChannels, deviceInfo->maxOutputChannels);
    printf("       Default sample rate: %.0f Hz\n", deviceInfo->defaultSampleRate);
    printf("       Default low latency: %.3f sec\n", deviceInfo->defaultLowOutputLatency);
    printf("       Default high latency: %.3f sec\n", deviceInfo->defaultHighOutputLatency);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "[ERROR] Usage: %s <wav_file.wav>\n", argv[0]);
        return 1;
    }

    printf("\n===== PortAudio WAV Player =====\n");
    printf("[INFO] Starting WAV playback for: %s\n", argv[1]);

    // Load PCM data and format information
    size_t pcm_size = 0;
    wav_format_t wav_format = {0};
    unsigned char *pcm_data = load_wav(argv[1], &pcm_size, &wav_format);
    if (!pcm_data) {
        fprintf(stderr, "[ERROR] Failed to load WAV file\n");
        return 1;
    }

    // Initialize PortAudio
    printf("[INFO] Initializing PortAudio...\n");
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "[ERROR] PortAudio init failed: %s\n", Pa_GetErrorText(err));
        free(pcm_data);
        return 1;
    }
    printf("[INFO] PortAudio initialized successfully\n");

    // Get default output device
    printf("[INFO] Getting default output device...\n");
    PaDeviceIndex device = Pa_GetDefaultOutputDevice();
    if (device == paNoDevice) {
        fprintf(stderr, "[ERROR] No default output device available\n");
        Pa_Terminate();
        free(pcm_data);
        return 1;
    }
    printf("[INFO] Using default output device #%d\n", device);
    print_device_info(device);

    // Set audio context
    AudioData audioCtx = {
        .data = pcm_data,
        .size = pcm_size,
        .pos = 0,
        .format = wav_format};
    printf("[DEBUG] Audio context initialized\n");

    // Set output parameters
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(device);
    PaStreamParameters outputParams = {
        .device = device,
        .channelCount = wav_format.channels,
        .sampleFormat = (wav_format.bitsPerSample == 16) ? paInt16 : (wav_format.bitsPerSample == 32) ? paFloat32
                                                                                                      : paInt8,
        .suggestedLatency = deviceInfo->defaultLowOutputLatency,
        .hostApiSpecificStreamInfo = NULL};
    printf("[INFO] Output parameters:\n");
    printf("       Channels: %d\n", outputParams.channelCount);
    printf("       Sample format: %s\n",
           (outputParams.sampleFormat == paInt16) ? "16-bit integer" : "8-bit integer");
    printf("       Suggested latency: %.3f sec\n", outputParams.suggestedLatency);

    // Open audio stream
    printf("[INFO] Opening audio stream...\n");
    PaStream *stream;
    err = Pa_OpenStream(
        &stream,
        NULL,                         // No input
        &outputParams,                // Output parameters
        wav_format.sampleRate,        // Sample rate
        paFramesPerBufferUnspecified, // Auto buffer size
        paClipOff,                    // Disable clipping
        paCallback,                   // Callback function
        &audioCtx                     // User data
    );

    if (err != paNoError) {
        fprintf(stderr, "[ERROR] Failed to open stream: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        free(pcm_data);
        return 1;
    }
    printf("[INFO] Audio stream opened successfully\n");

    // Start playback
    printf("[INFO] Starting playback...\n");
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "[ERROR] Failed to start stream: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(stream);
        Pa_Terminate();
        free(pcm_data);
        return 1;
    }

    // Calculate duration
    float duration = (float)pcm_size /
                     (wav_format.sampleRate * wav_format.channels * (wav_format.bitsPerSample / 8.0f));

    printf("[INFO] Playing: %s (%.1f seconds)\n", argv[1], duration);

    // Wait for playback completion
    printf("[DEBUG] Entering playback loop...\n");
    while (Pa_IsStreamActive(stream)) {
        Pa_Sleep(100);
        // Print progress every 500ms
        static int counter = 0;
        if (++counter % 5 == 0) {
            float progress = (float)audioCtx.pos / audioCtx.size * 100;
            printf("[PROGRESS] %.1f%% complete\r", progress);
            fflush(stdout);
        }
    }
    printf("\n[INFO] Playback completed\n");

    // Cleanup resources
    printf("[INFO] Stopping stream...\n");
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "[WARN] Error stopping stream: %s\n", Pa_GetErrorText(err));
    }

    printf("[INFO] Closing stream...\n");
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "[WARN] Error closing stream: %s\n", Pa_GetErrorText(err));
    }

    printf("[INFO] Terminating PortAudio...\n");
    Pa_Terminate();

    printf("[INFO] Freeing PCM data...\n");
    free(pcm_data);

    printf("[SUCCESS] Playback finished\n");
    return 0;
}

/* Compile Command:
    Linux:
        gcc portAudio_play_wav.c  -lportaudio -o portAudio_play_wav
    Windows:
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
        cl portAudio_play_wav.c ^
        /I "portaudio-19.7.0\include" ^
        /link /LIBPATH:"portaudio-19.7.0\libs\x64\Release" portaudio_x64.lib
*/
