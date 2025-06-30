/***************************************************************************
 * Description: loop play
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2024-05-01 23:52:18
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "wav_utils.h"

#define E_OK 0
#define E_FAIL -1
#define PROCESS_BUFFER_DURATION_MS 20
#define bit_to_Byte 8
#define LOGALL LOGE

typedef int32_t bool_t;

enum {
    false = 0,
    true = 1,
};

int FileCurrentOffset = 0, FileTotalFrameCount = 0;
int mTotalSilenceFrameCount = 2000, mCurrentSilenceOffset = 0;

int mRepeatCount = 1000; // loop play 1000 times
int bytes_per_frame = 0;
int16_t *mAudio3dChimeProcessBuffer = NULL;
int16_t *mAudioSourceBuffer = NULL;
bool_t mRepeatPlayback = true;
int silence_time = 1000;
FILE *fp_dst = NULL;
char *OUT_FILE_NAME = "\\out_player.pcm";

// this function is executed in RxMixer Thread, need protect common resource
int32_t prepare_data(uint32_t frame_count)
{
    uint32_t processed_frame_count = 0;
    int32_t silence_frames = 0;

    while (processed_frame_count < frame_count) {
        if (FileCurrentOffset < FileTotalFrameCount) {
            if (FileTotalFrameCount - FileCurrentOffset >= (int32_t)frame_count) {
                processed_frame_count = frame_count;
                memcpy(mAudio3dChimeProcessBuffer, (int16_t *)mAudioSourceBuffer + FileCurrentOffset * 2, processed_frame_count * bytes_per_frame);
                if (fp_dst == NULL) {
                    LOGE("fp_dst is NULL\n");
                    return E_FAIL;
                }
                FileCurrentOffset += processed_frame_count; // next frame
            } else {
                processed_frame_count = FileTotalFrameCount - FileCurrentOffset;
                memcpy(mAudio3dChimeProcessBuffer, (int16_t *)mAudioSourceBuffer + FileCurrentOffset * 2, processed_frame_count * bytes_per_frame);
                if (fp_dst == NULL) {
                    LOGE("fp_dst is NULL\n");
                    return E_FAIL;
                }
                FileCurrentOffset += processed_frame_count; // next frame
            }
        } else if (FileCurrentOffset == FileTotalFrameCount) {
            LOGD("player all source file is wrote to mixer\n");
            // process silence
            if (mRepeatPlayback) {
                if (mTotalSilenceFrameCount != 0 && (mRepeatCount > 1 || mRepeatCount < 0)) {
                    // write silence to dst buffer
                    LOGD("will write silence to dst buffer mCurrentSilenceOffset %d mTotalSilenceFrameCount %d processed_frame_count %d\n", mCurrentSilenceOffset, mTotalSilenceFrameCount, processed_frame_count);
                    if (mCurrentSilenceOffset < mTotalSilenceFrameCount) {
                        // space left in buffer is smaller than silence buffer
                        if ((frame_count - processed_frame_count) < (mTotalSilenceFrameCount - mCurrentSilenceOffset)) {
                            LOGE("space left %d in buffer is smaller than silence buffer %d\n", frame_count - processed_frame_count, mTotalSilenceFrameCount - mCurrentSilenceOffset);
                            silence_frames = frame_count - processed_frame_count;
                            processed_frame_count += silence_frames;
                            mCurrentSilenceOffset += silence_frames;
                        } else {
                            silence_frames = mTotalSilenceFrameCount - mCurrentSilenceOffset;
                            processed_frame_count += silence_frames;
                            mCurrentSilenceOffset = 0;
                            FileCurrentOffset = 0; // go back to start of audio file
                            if (mRepeatCount > 0) {
                                mRepeatCount--;
                            }
                            memcpy(mAudio3dChimeProcessBuffer + processed_frame_count, (int16_t *)mAudioSourceBuffer + FileCurrentOffset * 2, (frame_count - processed_frame_count) * bytes_per_frame);
                            FileCurrentOffset += frame_count - processed_frame_count; // next frame
                            processed_frame_count = frame_count;
                        }
                    } else {
                        // set silence time will update mTotalSilenceFrameCount during loop
                        // if current silence count is greater than or equal to mTotalSilenceFrameCount, then stop silence and read from file
                        mCurrentSilenceOffset = 0;
                        FileCurrentOffset = 0; // go back to start of audio file
                        if (mRepeatCount > 0)
                            mRepeatCount--;
                    }
                } else {
                    FileCurrentOffset = 0; // go back to start of audio file
                    if (mRepeatCount > 0) {
                        mRepeatCount--;
                    }
                }
            }
            if (!mRepeatPlayback || (mRepeatPlayback && mRepeatCount == 0)) {
                LOGD("%s stop player\n", __func__);
                break;
            }
        }
    }
}

int read_wav_header(FILE *fp_source, wav_ptr wav)
{
    riff_chunk_ptr riff_chunk = &(wav->riff_chunk);
    format_chunk_ptr format_chunk = &(wav->format_chunk);
    sub_chunk_ptr data_chunk = &(wav->data_chunk);

    if (fread(riff_chunk, sizeof(riff_chunk_t), 1, fp_source) == 0) {
        LOGE("ERROR: Read RIFF chunk failed!\n");
        fclose(fp_source);
        free(wav);
        return -3;
    }
    if (strncmp(riff_chunk->riff_type, "WAVE", ID_LENGTH) != 0) {
        LOGE("ERROR: This is not a wav file!\n");
        fclose(fp_source);
        free(wav);
        return -4;
    }
    LOGI("ChunkID:\t%c%c%c%c\n", riff_chunk->chunk_id[0], riff_chunk->chunk_id[1],
         riff_chunk->chunk_id[2], riff_chunk->chunk_id[3]);
    LOGI("ChunkSize:\t%u Bytes\n", riff_chunk->chunk_size);
    LOGI("type:\t\t%c%c%c%c\n\n", riff_chunk->riff_type[0], riff_chunk->riff_type[1],
         riff_chunk->riff_type[2], riff_chunk->riff_type[3]);

    while (strncmp(format_chunk->chunk_id, "fmt ", ID_LENGTH) != 0) {
        if (fread(format_chunk->chunk_id, sizeof(char), ID_LENGTH, fp_source) == 0) {
            LOGE("ERROR: Read format chunk id failed!\n");
            fclose(fp_source);
            free(wav);
            return -5;
        }
        if (fread(&format_chunk->chunk_size, sizeof(uint32_t), 1, fp_source) == 0) {
            LOGE("ERROR: Read format chunk size failed!\n");
            fclose(fp_source);
            free(wav);
            return -6;
        }
        if (feof(fp_source) != 0) { // feof need to be checked before fseek
            LOGE("ERROR: Can't find format_chunk!\n");
            fclose(fp_source);
            free(wav);
            return -7;
        }
        if (fseek(fp_source, format_chunk->chunk_size, SEEK_CUR) != 0) {
            LOGE("ERROR: Read format chunk failed!\n");
            fclose(fp_source);
            free(wav);
            return -8;
        }
        LOGI("ChunkID:\t%c%c%c%c\n", format_chunk->chunk_id[0], format_chunk->chunk_id[1],
             format_chunk->chunk_id[2], format_chunk->chunk_id[3]);
        LOGI("ChunkSize:\t%u Bytes\n", format_chunk->chunk_size);
        if (strncmp(format_chunk->chunk_id, "fmt ", ID_LENGTH) != 0) {
            LOGI("\n");
        }
    }
    fseek(fp_source, 0 - format_chunk->chunk_size, SEEK_CUR);
    fread(&format_chunk->AudioFmt_tag, format_chunk->chunk_size, 1, fp_source);
    LOGI("AudioFormat:\t%d\n", format_chunk->AudioFmt_tag);
    LOGI("NumberChannel:\t%d\n", format_chunk->Num_Channel);
    LOGI("SampleRate:\t%.1f kHz\n", (float)format_chunk->SampleRate / KILO);
    LOGI("bytes_per_sec:\t%u\n", format_chunk->bytes_per_sec);
    LOGI("BlockAlign:\t%d\n", format_chunk->BlockAlign);
    LOGI("bits_per_sam:\t%d\n", format_chunk->bits_per_sam);
    if (format_chunk->AudioFmt_tag == 0xFFFE) {
        LOGI("cbSize:\t\t%d\n", format_chunk->cbSize);
        LOGI("ValidBits:\t%d\n", format_chunk->ValidBits);
        LOGI("ChannelMask:\t%d\n", format_chunk->ChannelMask);
        LOGI("SubFormat:\t%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
             format_chunk->SubFormat.Data1, format_chunk->SubFormat.Data2,
             format_chunk->SubFormat.Data3, format_chunk->SubFormat.Data4[0],
             format_chunk->SubFormat.Data4[1], format_chunk->SubFormat.Data4[2],
             format_chunk->SubFormat.Data4[3], format_chunk->SubFormat.Data4[4],
             format_chunk->SubFormat.Data4[5], format_chunk->SubFormat.Data4[6],
             format_chunk->SubFormat.Data4[7]);
        LOGD2("\n");
    }

    while (strncmp(data_chunk->chunk_id, "data", ID_LENGTH) != 0) {
        fread(data_chunk->chunk_id, sizeof(char), ID_LENGTH, fp_source);
        fread(&data_chunk->chunk_size, sizeof(uint32_t), 1, fp_source);
        fseek(fp_source, data_chunk->chunk_size, SEEK_CUR);
        LOGI("ChunkID:\t%c%c%c%c\n", data_chunk->chunk_id[0], data_chunk->chunk_id[1],
             data_chunk->chunk_id[2], data_chunk->chunk_id[3]);
        LOGI("ChunkSize:\t%u Bytes\n\n", data_chunk->chunk_size);
    }

    if (strncmp(data_chunk->chunk_id, "data", ID_LENGTH) == 0) {
        fseek(fp_source, 0 - data_chunk->chunk_size, SEEK_CUR);
    } else {
        LOGE("ERROR: Can't find data_chunk!\n");
    }
    return 0;
}

// 48000 16bit 2 channel
int main(int argc, char *argv[])
{
    int sample_rate = 48000;
    int channels = 2;
    int bits_per_sample = 16;
    int bytes_per_sec = sample_rate * bits_per_sample / bit_to_Byte * channels;
    bytes_per_frame = bits_per_sample / bit_to_Byte * channels;
    int frame_count = bytes_per_sec / 1000 * PROCESS_BUFFER_DURATION_MS / bytes_per_frame; // 20ms
    mTotalSilenceFrameCount = silence_time * sample_rate / 1000;

    if (argc < 2) {
        LOGW("Usage: %s <input file>\n", argv[0]);
        return E_FAIL;
    }
    LOGD("bytes_per_sec %d, bytes_per_frame %d, frame_count %d, mTotalSilenceFrameCount %d\n", bytes_per_sec, bytes_per_frame, frame_count, mTotalSilenceFrameCount);

    FILE *fp_source = fopen(argv[1], "rb");
    if (fp_source == NULL) {
        LOGE("fopen %s failed. Due to %s\n", argv[1], strerror(errno));
        return E_FAIL;
    }

    char out_path[SHRT_MAX] = "c:\\Users\\young\\Desktop\\";
    strcat(out_path, OUT_FILE_NAME);

    fp_dst = fopen(out_path, "wb");
    if (fp_dst == NULL) {
        LOGE("fopen %s failed. Due to %s\n", argv[2], strerror(errno));
        fclose(fp_source);
        return E_FAIL;
    }

    wav_ptr wav = (wav_ptr)malloc(sizeof(wav_t));
    if (wav == NULL) {
        LOGE("Alloccate wav memory failed!\n");
        fclose(fp_source);
        return -3;
    }
    read_wav_header(fp_source, wav);
    FileTotalFrameCount = wav->data_chunk.chunk_size / bytes_per_frame;

    mAudioSourceBuffer = (int16_t *)malloc(wav->data_chunk.chunk_size);
    if (mAudioSourceBuffer == NULL) {
        LOGE("malloc mAudioSourceBuffer failed");
        fclose(fp_source);
        free(wav);
        return E_FAIL;
    }
    fread(mAudioSourceBuffer, wav->data_chunk.chunk_size, 1, fp_source);

    mAudio3dChimeProcessBuffer = (int16_t *)malloc(bytes_per_sec / 1000 * PROCESS_BUFFER_DURATION_MS); // 20ms
    if (mAudio3dChimeProcessBuffer == NULL) {
        LOGE("malloc mAudio3dChimeProcessBuffer failed");
        return E_FAIL;
    }

    int aa = 500;
    while (aa--) {
        prepare_data(frame_count);
        int writen = fwrite(mAudio3dChimeProcessBuffer, bytes_per_sec / 1000 * PROCESS_BUFFER_DURATION_MS, 1, fp_dst);
        if (!writen) {
            LOGE("fwrite failed.\n");
        } else {
            LOGD("fwrite silence success\n");
        }
        usleep(20 * 1000);
    }

    if (mAudioSourceBuffer) {
        free(mAudioSourceBuffer);
        mAudioSourceBuffer = NULL;
    }

    if (mAudio3dChimeProcessBuffer) {
        free(mAudio3dChimeProcessBuffer);
        mAudio3dChimeProcessBuffer = NULL;
    }
    if (wav != NULL) {
        free(wav);
        wav = NULL;
    }

    if (fp_source) {
        fclose(fp_source);
    }
    if (fp_dst) {
        fflush(fp_dst);
        fclose(fp_dst);
    }

    return 0;
}

/*Compile Command: gcc test.c -lshell32 -lole32 -luuid -o test */
