/*
 * Example: Play a WAV file with PortAudio, processing each block before output.
 * - Reads and processes 1024 samples per write.
 * - Detailed debug logs and comments included.
 * - No playback loop: file is played once and exits.
 *
 * Requirements:
 *   - PortAudio (https://www.portaudio.com/)
 *   - Standard 16-bit PCM WAV file (mono or stereo)
 *
 * Compile: (Windows, MSVC)
 *   cl play_wave_portaudio.c /I"path\to\portaudio\include" /link /LIBPATH:"path\to\portaudio\lib" portaudio_x86.lib
 *
 * Compile: (MinGW)
 *   gcc play_wave_portaudio.c -lportaudio
 *
 * Usage:
 *   play_wave_portaudio.exe <wav_file_path>
 */

#include "algo_example.h"
#include "audio_async_blocking.h"
#include "portaudio.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 1024 // Number of samples per block (not frames!)

// WAV file header for 16-bit PCM
typedef struct {
    char riff[4];
    uint32_t chunk_size;
    char wave[4];
    char fmt[4];
    uint32_t fmt_length;
    uint16_t audio_format;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data[4];
    uint32_t data_size;
} WAVHeader;

// Process a block of audio samples (in-place)
// sampleCount = number of int16_t samples (not frames; for stereo: frames*2)
void process_block(int16_t *samples, size_t sampleCount)
{
    for (size_t i = 0; i < sampleCount; ++i) {
        // Example: halve the volume
        samples[i] /= 2;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <wav_file>\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("[ERROR] Failed to open file: %s\n", filename);
        return 1;
    }

    // Read WAV header
    WAVHeader header;
    if (fread(&header, sizeof(WAVHeader), 1, file) != 1) {
        printf("[ERROR] Failed to read WAV header.\n");
        fclose(file);
        return 1;
    }
    printf("[INFO] WAV file info:\n");
    printf("  Channels: %d\n", header.channels);
    printf("  Sample rate: %d\n", header.sample_rate);
    printf("  Bits per sample: %d\n", header.bits_per_sample);
    printf("  Data size: %d bytes\n", header.data_size);

    // Check format
    if (memcmp(header.riff, "RIFF", 4) != 0 || memcmp(header.wave, "WAVE", 4) != 0 ||
        memcmp(header.fmt, "fmt ", 4) != 0 || memcmp(header.data, "data", 4) != 0) {
        printf("[ERROR] Not a standard WAV file.\n");
        fclose(file);
        return 1;
    }
    if (header.audio_format != 1 || header.bits_per_sample != 16) {
        printf("[ERROR] Only 16-bit PCM WAV files are supported.\n");
        fclose(file);
        return 1;
    }

    // Calculate total samples
    size_t totalSamples = header.data_size / (header.bits_per_sample / 8);
    size_t samplesPerFrame = header.channels;
    printf("[INFO] Total samples: %zu (frames: %zu)\n", totalSamples, totalSamples / samplesPerFrame);

    // Initialize PortAudio
    PaError paErr;
    paErr = Pa_Initialize();
    if (paErr != paNoError) {
        printf("[ERROR] PortAudio initialization failed: %s\n", Pa_GetErrorText(paErr));
        fclose(file);
        return 1;
    }

    // Open PortAudio output stream
    PaStream *stream;
    PaStreamParameters outputParams;
    outputParams.device = Pa_GetDefaultOutputDevice();
    if (outputParams.device == paNoDevice) {
        printf("[ERROR] No default audio output device.\n");
        Pa_Terminate();
        fclose(file);
        return 1;
    }
    outputParams.channelCount = header.channels;
    outputParams.sampleFormat = paInt16;
    outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = NULL;

    paErr = Pa_OpenStream(
        &stream,
        NULL, // no input
        &outputParams,
        header.sample_rate,
        BLOCK_SIZE / samplesPerFrame, // frames per buffer
        paClipOff,
        NULL,
        NULL);
    if (paErr != paNoError) {
        printf("[ERROR] Failed to open PortAudio stream: %s\n", Pa_GetErrorText(paErr));
        Pa_Terminate();
        fclose(file);
        return 1;
    }

    paErr = Pa_StartStream(stream);
    if (paErr != paNoError) {
        printf("[ERROR] Failed to start PortAudio stream: %s\n", Pa_GetErrorText(paErr));
        Pa_CloseStream(stream);
        Pa_Terminate();
        fclose(file);
        return 1;
    }
    printf("[INFO] Started audio playback.\n");

    // Allocate buffer for one block
    int16_t *block = (int16_t *)malloc(BLOCK_SIZE * sizeof(int16_t));
    if (!block) {
        printf("[ERROR] Out of memory for block buffer.\n");
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        Pa_Terminate();
        fclose(file);
        return 1;
    }

    void *ctx = audio_async_create();
    float *in_buffer = (float *)malloc(BLOCK_SIZE * sizeof(float));
    float *out_buffer = (float *)malloc(BLOCK_SIZE * sizeof(float));

    // Playback loop (not repeating, plays WAV file once)
    size_t samplesReadTotal = 0;
    while (samplesReadTotal < totalSamples) {
        size_t samplesToRead = BLOCK_SIZE;
        if (samplesReadTotal + samplesToRead > totalSamples) {
            samplesToRead = totalSamples - samplesReadTotal;
        }
        size_t actuallyRead = fread(block, sizeof(int16_t), samplesToRead, file);
        if (actuallyRead == 0) {
            printf("[DEBUG] End of file reached or read error.\n");
            break;
        }
        printf("[DEBUG] Read %zu samples from file.\n", actuallyRead);

        // Optional: zero out remainder if we got less than requested (should only happen at EOF)
        if (actuallyRead < BLOCK_SIZE) {
            memset(block + actuallyRead, 0, (BLOCK_SIZE - actuallyRead) * sizeof(int16_t));
        }

        // Process the block
        // process_block(block, actuallyRead);
        for (size_t i = 0; i < actuallyRead; i++) {
            in_buffer[i] = (float)block[i] / 32768.0f;
        }
        audio_async_process(ctx, in_buffer, out_buffer, actuallyRead);
        for (size_t i = 0; i < actuallyRead; i++) {
            block[i] = (int16_t)(out_buffer[i] * 32768.0f);
        }

        // Write to PortAudio stream
        paErr = Pa_WriteStream(stream, block, actuallyRead / samplesPerFrame);
        if (paErr != paNoError) {
            printf("[ERROR] PortAudio write failed: %s\n", Pa_GetErrorText(paErr));
            break;
        }
        printf("[DEBUG] Wrote %zu samples to audio output.\n", actuallyRead);

        samplesReadTotal += actuallyRead;
    }

    printf("[INFO] Playback finished. Total samples played: %zu\n", samplesReadTotal);

    // Cleanup
    free(block);
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    fclose(file);
    audio_async_destroy(ctx);
    free(in_buffer);
    free(out_buffer);

    return 0;
}
