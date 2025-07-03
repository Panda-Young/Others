/* **************************************************************************
 * @Description: WAV file playback using Oboe API (Android NDK)
 * @Version: 1.0.0
 * @Author: pandapan@aactechnologies.com
 * @Date: 2025-06-23 14:12:08
 * @Copyright (c) 2025 by @AAC Technologies, All Rights Reserved.
 **************************************************************************/

#include "oboe/Oboe.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <unistd.h>

using namespace oboe;

// WAV format structure
typedef struct {
    int sampleRate;
    short channels;
    short bitsPerSample;
} wav_format_t;

// PCM context
typedef struct {
    unsigned char *data;
    size_t size;
    size_t pos;
    bool playback_finished;
    wav_format_t format;
} pcm_chunk_ctx_t;

// Audio callback class
class AudioCallback : public AudioStreamDataCallback
{
public:
    explicit AudioCallback(pcm_chunk_ctx_t *ctx) : ctx_(ctx) {}

    void preprocess_pcm_buffer(short *buffer, size_t samples)
    {
        for (size_t i = 0; i < samples; ++i) {
            buffer[i] = -buffer[i];
        }
    }

    DataCallbackResult onAudioReady(AudioStream *stream, void *audioData, int32_t numFrames) override
    {
        if (!ctx_ || ctx_->playback_finished) {
            return DataCallbackResult::Stop;
        }

        const int frameSize = (ctx_->format.bitsPerSample / 8) * ctx_->format.channels;
        const size_t bytesNeeded = numFrames * frameSize;
        const size_t bytesAvailable = ctx_->size - ctx_->pos;

        if (bytesAvailable == 0) {
            printf("[LOG] Playback completed\n");
            ctx_->playback_finished = true;
            return DataCallbackResult::Stop;
        }

        const size_t bytesToCopy = (bytesNeeded > bytesAvailable) ? bytesAvailable : bytesNeeded;

        if (ctx_->format.bitsPerSample == 16) {
            memcpy(audioData, ctx_->data + ctx_->pos, bytesToCopy);

            const size_t samplesToProcess = bytesToCopy / sizeof(short);
            preprocess_pcm_buffer(static_cast<short *>(audioData), samplesToProcess);

            ctx_->pos += bytesToCopy;
        } else {
            memcpy(audioData, ctx_->data + ctx_->pos, bytesToCopy);
            ctx_->pos += bytesToCopy;
        }

        if (bytesToCopy < bytesNeeded) {
            size_t paddingBytes = bytesNeeded - bytesToCopy;
            memset(static_cast<char *>(audioData) + bytesToCopy, 0, paddingBytes);
        }

        return DataCallbackResult::Continue;
    }

private:
    pcm_chunk_ctx_t *ctx_;
};

// Error callback class
class ErrorCallback : public AudioStreamErrorCallback
{
public:
    explicit ErrorCallback(pcm_chunk_ctx_t *ctx) : ctx_(ctx) {}

    void onErrorAfterClose(AudioStream *stream, Result error) override
    {
        fprintf(stderr, "[ERROR] Oboe error: %s\n", convertToText(error));
        if (ctx_) {
            ctx_->playback_finished = true;
        }
    }

private:
    pcm_chunk_ctx_t *ctx_;
};

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

int main(int argc, char *argv[])
{
    printf("[LOG] ----------- Oboe WAV Player -----------\n");
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

    // Initialize playback context
    pcm_chunk_ctx_t audioCtx = {
        .data = pcm_data,
        .size = pcm_size,
        .pos = 0,
        .playback_finished = false,
        .format = wav_format};

    // Create Oboe audio stream
    AudioStreamBuilder builder;
    std::shared_ptr<AudioStream> stream;
    auto audioCallback = std::make_shared<AudioCallback>(&audioCtx);
    auto errorCallback = std::make_shared<ErrorCallback>(&audioCtx);

    builder.setDirection(Direction::Output)
        ->setSampleRate(wav_format.sampleRate)
        ->setChannelCount(wav_format.channels)
        ->setFormat(AudioFormat::I16)
        ->setDataCallback(audioCallback)
        ->setErrorCallback(errorCallback)
        ->setPerformanceMode(PerformanceMode::LowLatency)
        ->setSharingMode(SharingMode::Exclusive);

    // Open the stream
    Result result = builder.openStream(stream);
    if (result != Result::OK) {
        fprintf(stderr, "[ERROR] Failed to create stream: %s\n", convertToText(result));
        free(pcm_data);
        return 1;
    }

    // Start playback
    result = stream->requestStart();
    if (result != Result::OK) {
        fprintf(stderr, "[ERROR] Start failed: %s\n", convertToText(result));
        free(pcm_data);
        return 1;
    }
    printf("[LOG] Playback started...\n");

    // Wait for completion with timeout
    const int MAX_WAIT_SECONDS = 30;
    int waitCount = 0;
    while (!audioCtx.playback_finished) {
        usleep(100000); // 100ms polling
        waitCount++;

        if (waitCount > MAX_WAIT_SECONDS * 10) {
            fprintf(stderr, "[ERROR] Playback timeout reached\n");
            break;
        }
    }

    // Release resources
    printf("[LOG] Releasing resources...\n");
    stream->close();
    free(pcm_data);

    printf("[LOG] Playback finished %s\n",
           audioCtx.playback_finished ? "successfully" : "with errors");
    return 0;
}
