/***************************************************************************
 * Description:
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2024-10-28 22:00:35
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

 #if defined(_MSC_VER)
 #ifndef _CRT_SECURE_NO_WARNINGS
 #define _CRT_SECURE_NO_WARNINGS
 #endif
 #endif

#include "wav_p.h"
#include "log.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef IsEqualGUID
int IsEqualGUID(const GUID_t *guid1, const GUID_t *guid2)
{
    return (guid1->Data1 == guid2->Data1 &&
            guid1->Data2 == guid2->Data2 &&
            guid1->Data3 == guid2->Data3 &&
            memcmp(guid1->Data4, guid2->Data4, 8) == 0);
}
#endif

#ifdef _WIN32
void enable_virtual_terminal_processing()
{
    // Enable ANSI escape code for windows console
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;

    if (GetConsoleMode(hConsole, &mode)) {
        SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}
#endif

int get_wav_header(const char *filename, std_wav_t *wav_header)
{
    extra_info_chunk_t extra_info_chunk = {0};
    uint32_t std_fmt_size = sizeof(format_chunk_t) - ID_LENGTH - sizeof(uint32_t);
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        LOGE("Failed open file %s. Due to %s!", filename, strerror(errno));
        return -1;
    }

    if (fread(&wav_header->riff_chunk, sizeof(riff_chunk_t), 1, fp) == 0) {
        LOGE("ERROR: Read RIFF chunk failed!");
        goto error;
    }
    if (strncmp((const char *)wav_header->riff_chunk.chunk_id, "RIFF", ID_LENGTH) != 0) {
        LOGE("ERROR: This is not a RIFF file!");
        goto error;
    }
    LOGI("ChunkID:         %c%c%c%c",
         wav_header->riff_chunk.chunk_id[0], wav_header->riff_chunk.chunk_id[1],
         wav_header->riff_chunk.chunk_id[2], wav_header->riff_chunk.chunk_id[3]);
    LOGI("ChunkSize:       %u Bytes", wav_header->riff_chunk.chunk_size);
    LOGI("type:            %c%c%c%c\n",
         wav_header->riff_chunk.riff_type[0], wav_header->riff_chunk.riff_type[1],
         wav_header->riff_chunk.riff_type[2], wav_header->riff_chunk.riff_type[3]);

    if (strncmp((const char *)wav_header->riff_chunk.riff_type, "WAVE", ID_LENGTH) != 0) {
        LOGE("ERROR: This is not a wave file!");
        goto error;
    }

    do {
        if (fread(&wav_header->format_chunk.chunk_id, sizeof(int8_t), ID_LENGTH, fp) == 0) {
            LOGE("ERROR: Read format chunk id failed!");
            goto error;
        }
        if (fread(&wav_header->format_chunk.chunk_size, sizeof(uint32_t), 1, fp) == 0) {
            LOGE("ERROR: Read format chunk size failed!");
            goto error;
        }
        LOGI("ChunkID:         %c%c%c%c",
             wav_header->format_chunk.chunk_id[0], wav_header->format_chunk.chunk_id[1],
             wav_header->format_chunk.chunk_id[2], wav_header->format_chunk.chunk_id[3]);
        LOGI("ChunkSize:       %u Bytes", wav_header->format_chunk.chunk_size);
        if (strncmp((const char *)wav_header->format_chunk.chunk_id, "fmt ", ID_LENGTH) == 0) {
            break;
        } else {
            LOGD2("");
            memset(wav_header->format_chunk.chunk_id, 0, ID_LENGTH);
            if (fseek(fp, wav_header->format_chunk.chunk_size, SEEK_CUR) != 0) {
                LOGE("ERROR: Seek format chunk failed!");
                goto error;
            }
            wav_header->format_chunk.chunk_size = 0;
        }
    } while (feof(fp) != -1);

    if (strncmp((const char *)wav_header->format_chunk.chunk_id, "fmt ", ID_LENGTH) != 0) {
        LOGE("ERROR: Can't find format_chunk!");
        goto error;
    }
    if (fread(&wav_header->format_chunk.AudioFmt_tag, std_fmt_size, 1, fp) == 0) {
        LOGE("ERROR: Read format chunk failed!");
        goto error;
    }
    LOGI("AudioFmt_tag:    %u", wav_header->format_chunk.AudioFmt_tag);
    LOGI("Num_Channel:     %u", wav_header->format_chunk.Num_Channel);
    LOGI("SampleRate:      %u", wav_header->format_chunk.SampleRate);
    LOGI("bytes_per_sec:   %u", wav_header->format_chunk.bytes_per_sec);
    LOGI("BlockAlign:      %u", wav_header->format_chunk.BlockAlign);
    LOGI("bits_per_sam:    %u", wav_header->format_chunk.bits_per_sam);
    if (wav_header->format_chunk.chunk_size > std_fmt_size) {
        if (wav_header->format_chunk.AudioFmt_tag == 0xFFFE) {
            if (fread(&extra_info_chunk, sizeof(extra_info_chunk_t), 1, fp) == 0) {
                LOGE("ERROR: Read extra info chunk failed!");
                goto error;
            }
            LOGI("cbSize:          %u", extra_info_chunk.cbSize);
            LOGI("ValidBits:       %u", extra_info_chunk.ValidBits);
            LOGI("ChannelMask:     0x%X", extra_info_chunk.ChannelMask);
            LOGI("SubFormat:       0x%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                 extra_info_chunk.SubFormat.Data1,
                 extra_info_chunk.SubFormat.Data2, extra_info_chunk.SubFormat.Data3,
                 extra_info_chunk.SubFormat.Data4[0], extra_info_chunk.SubFormat.Data4[1],
                 extra_info_chunk.SubFormat.Data4[2], extra_info_chunk.SubFormat.Data4[3],
                 extra_info_chunk.SubFormat.Data4[4], extra_info_chunk.SubFormat.Data4[5],
                 extra_info_chunk.SubFormat.Data4[6], extra_info_chunk.SubFormat.Data4[7]);
            if (wav_header->format_chunk.chunk_size > std_fmt_size + sizeof(extra_info_chunk_t)) {
                if (fseek(fp, wav_header->format_chunk.chunk_size - std_fmt_size - sizeof(extra_info_chunk_t),
                          SEEK_CUR) != 0) {
                    LOGE("ERROR: Seek extra info chunk failed!");
                    goto error;
                }
            }
        } else {
            LOGD2("");
            if (fseek(fp, wav_header->format_chunk.chunk_size - std_fmt_size, SEEK_CUR) != 0) {
                LOGE("ERROR: Seek extra info chunk failed!");
                goto error;
            }
        }
    } else {
        LOGD2("");
    }

    do {
        if (fread(&wav_header->data_chunk, sizeof(sub_chunk_t), 1, fp) == 0) {
            LOGE("ERROR: Read data chunk failed!");
            goto error;
        }
        LOGI("ChunkID:         %c%c%c%c",
             wav_header->data_chunk.chunk_id[0], wav_header->data_chunk.chunk_id[1],
             wav_header->data_chunk.chunk_id[2], wav_header->data_chunk.chunk_id[3]);
        LOGI("ChunkSize:       %u Bytes\n", wav_header->data_chunk.chunk_size);
        if (strncmp((const char *)wav_header->data_chunk.chunk_id, "data", ID_LENGTH) == 0) {
            break;
        } else {
            memset(wav_header->data_chunk.chunk_id, 0, ID_LENGTH);
            if (fseek(fp, wav_header->data_chunk.chunk_size, SEEK_CUR) != 0) {
                LOGE("ERROR: Seek data chunk failed!");
                goto error;
            }
            wav_header->data_chunk.chunk_size = 0;
        }
    } while (feof(fp) != -1);

    if (strncmp((const char *)wav_header->data_chunk.chunk_id, "data", ID_LENGTH) != 0) {
        LOGE("ERROR: Can't find data_chunk!");
        goto error;
    }
    if (fseek(fp, wav_header->data_chunk.chunk_size, SEEK_CUR) != 0) {
        LOGE("ERROR: Seek data chunk failed!");
        goto error;
    } else {
        fseek(fp, 0 - wav_header->data_chunk.chunk_size, SEEK_CUR);
    }

    if (fp != NULL) {
        if (fclose(fp) != 0) {
            LOGE("Failed to close file %s. Due to %s!", filename, strerror(errno));
            return -1;
        }
        fp = NULL;
    }
    return 0;

error:
    if (fp != NULL) {
        if (fclose(fp) != 0) {
            LOGE("Failed to close file %s. Due to %s!", filename, strerror(errno));
            return -1;
        }
        fp = NULL;
    }
    return -2;
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    enable_virtual_terminal_processing();
#endif
    if (argc < 2) {
        LOGW2("Usage: %s [wave file] <enable_data_debug>", argv[0]);
        return 0;
    }

    std_wav_t wav_header = {0};
    int result = get_wav_header(argv[1], &wav_header);
    if (result != 0) {
        LOGE("Failed to read WAV header with error code %d", result);
        return result;
    }

    return 0;
}
