/* **************************************************************
 * @Description: wav_utils.c
 * @Date: 2024-02-26 18:26:08
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * @Copyright (c) 2024 by @AAC Technologies, All Rights Reserved.
 **************************************************************/

#include "wav_utils.h"
#include "log.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int IsEqualGUID(const GUID_t *guid1, const GUID_t *guid2)
{
    return (guid1->Data1 == guid2->Data1 &&
            guid1->Data2 == guid2->Data2 &&
            guid1->Data3 == guid2->Data3 &&
            memcmp(guid1->Data4, guid2->Data4, 8) == 0);
}

int read_and_print_data(format_chunk_t *format_chunk, sub_chunk_t *data_chunk, FILE *fp)
{
    data_chunk->data = calloc(data_chunk->chunk_size, 1);
    if (data_chunk->data == NULL) {
        LOGE("Alloccate data memory failed!");
        return -1;
    }

    if (format_chunk->AudioFmt_tag == 1) { // PCM format
        if (format_chunk->bits_per_sam == 8) {
            unsigned char *data = (unsigned char *)data_chunk->data;
            for (int i = 0; i < data_chunk->chunk_size; i++) {
                fread(&data[i], sizeof(unsigned char), 1, fp);
                LOGD2("%8hhu ", data[i]);
                if ((i + 1) % 8 == 0) {
                    LOGD2("\n");
                }
            }
        } else if (format_chunk->bits_per_sam == 16) {
            short *data = (short *)data_chunk->data;
            for (int i = 0; i < data_chunk->chunk_size / sizeof(short); i++) {
                fread(&data[i], sizeof(short), 1, fp);
                LOGD2("%8hd ", data[i]);
                if ((i + 1) % 8 == 0) {
                    LOGD2("\n");
                }
            }
        } else if (format_chunk->bits_per_sam == 24) {
            unsigned char *data = (unsigned char *)data_chunk->data;
            for (int i = 0; i < data_chunk->chunk_size / 3; i++) {
                unsigned char bytes[3];
                fread(bytes, 1, 3, fp);
                int value = (bytes[0] << 0) | (bytes[1] << 8) | (bytes[2] << 16);
                if (bytes[2] & 0x80) {
                    value |= 0xFF000000;
                }
                LOGD2("%8d ", value);
                if ((i + 1) % 8 == 0) {
                    LOGD2("\n");
                }
            }
        } else if (format_chunk->bits_per_sam == 32) {
            int *data = (int *)data_chunk->data;
            for (int i = 0; i < data_chunk->chunk_size / sizeof(int); i++) {
                fread(&data[i], sizeof(int), 1, fp);
                LOGD2("%8d ", data[i]);
                if ((i + 1) % 8 == 0) {
                    LOGD2("\n");
                }
            }
        } else if (format_chunk->bits_per_sam == 64) {
            long long *data = (long long *)data_chunk->data;
            for (int i = 0; i < data_chunk->chunk_size / sizeof(long long); i++) {
                fread(&data[i], sizeof(long long), 1, fp);
                LOGD2("%8lld ", data[i]);
                if ((i + 1) % 8 == 0) {
                    LOGD2("\n");
                }
            }
        } else {
            LOGE("ERROR: PCM format unsupported bits per sample %d!\n", format_chunk->bits_per_sam);
            return -2;
        }
    } else if (format_chunk->AudioFmt_tag == 3) { // IEEE float format
        if (format_chunk->bits_per_sam == 32) {   // 32bit float
            float *data = (float *)data_chunk->data;
            for (int i = 0; i < data_chunk->chunk_size / sizeof(float); i++) {
                fread(&data[i], sizeof(float), 1, fp);
                LOGD2("%.8f ", data[i]);
                if ((i + 1) % 8 == 0) {
                    LOGD2("\n");
                }
            }
        } else if (format_chunk->bits_per_sam == 64) { // 64bit double
            double *data = (double *)data_chunk->data;
            for (int i = 0; i < data_chunk->chunk_size / sizeof(double); i++) {
                fread(&data[i], sizeof(double), 1, fp);
                LOGD2("%.8f ", data[i]);
                if ((i + 1) % 8 == 0) {
                    LOGD2("\n");
                }
            }
        } else {
            LOGE("ERROR: IEEE float format Unsupported bits per sample %d!", format_chunk->bits_per_sam);
            return -2;
        }
    } else if (format_chunk->AudioFmt_tag == 0xFFFE) { // Extensible format
        if (IsEqualGUID(&format_chunk->SubFormat, &KSDATAFORMAT_SUBTYPE_PCM)) {
            if (format_chunk->ValidBits == 8) {
                unsigned char *data = (unsigned char *)data_chunk->data;
                for (int i = 0; i < data_chunk->chunk_size; i++) {
                    fread(&data[i], sizeof(unsigned char), 1, fp);
                    LOGD2("%8hhu ", data[i]);
                    if ((i + 1) % 8 == 0) {
                        LOGD2("\n");
                    }
                }
            } else if (format_chunk->ValidBits == 16) {
                short *data = (short *)data_chunk->data;
                for (int i = 0; i < data_chunk->chunk_size / sizeof(short); i++) {
                    fread(&data[i], sizeof(short), 1, fp);
                    LOGD2("%8hd ", data[i]);
                    if ((i + 1) % 8 == 0) {
                        LOGD2("\n");
                    }
                }
            } else if (format_chunk->ValidBits == 24) {
                unsigned char *data = (unsigned char *)data_chunk->data;
                for (int i = 0; i < data_chunk->chunk_size / 3; i++) {
                    unsigned char bytes[3];
                    fread(bytes, 1, 3, fp);
                    int value = (bytes[0] << 0) | (bytes[1] << 8) | (bytes[2] << 16);
                    if (bytes[2] & 0x80) {
                        value |= 0xFF000000;
                    }
                    LOGD2("%8d ", value);
                    if ((i + 1) % 8 == 0) {
                        LOGD2("\n");
                    }
                }
            } else if (format_chunk->ValidBits == 32) {
                int *data = (int *)data_chunk->data;
                for (int i = 0; i < data_chunk->chunk_size / sizeof(int); i++) {
                    fread(&data[i], sizeof(int), 1, fp);
                    LOGD2("%8d ", data[i]);
                    if ((i + 1) % 8 == 0) {
                        LOGD2("\n");
                    }
                }
            } else if (format_chunk->ValidBits == 64) {
                long long *data = (long long *)data_chunk->data;
                for (int i = 0; i < data_chunk->chunk_size / sizeof(long long); i++) {
                    fread(&data[i], sizeof(long long), 1, fp);
                    LOGD2("%8lld ", data[i]);
                    if ((i + 1) % 8 == 0) {
                        LOGD2("\n");
                    }
                }
            } else {
                LOGE("ERROR: Unsupported Extensible PCM ValidBits %d!", format_chunk->ValidBits);
                return -2;
            }

        } else if (IsEqualGUID(&format_chunk->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
            if (format_chunk->ValidBits == 32) { // 32bit float
                float *data = (float *)data_chunk->data;
                for (int i = 0; i < data_chunk->chunk_size / sizeof(float); i++) {
                    fread(&data[i], sizeof(float), 1, fp);
                    LOGD2("%.8f ", data[i]);
                    if ((i + 1) % 8 == 0) {
                        LOGD2("\n");
                    }
                }
            } else if (format_chunk->ValidBits == 64) { // 64bit double
                double *data = (double *)data_chunk->data;
                for (int i = 0; i < data_chunk->chunk_size / sizeof(double); i++) {
                    fread(&data[i], sizeof(double), 1, fp);
                    LOGD2("%.8f ", data[i]);
                    if ((i + 1) % 8 == 0) {
                        LOGD2("\n");
                    }
                }
            } else {
                LOGE("ERROR: Unsupported Extensible IEEE float ValidBits %d!", format_chunk->ValidBits);
                return -2;
            }
        } else {
            LOGE("Unsupported Extensible SubFormat:\t%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                 format_chunk->SubFormat.Data1, format_chunk->SubFormat.Data2,
                 format_chunk->SubFormat.Data3, format_chunk->SubFormat.Data4[0],
                 format_chunk->SubFormat.Data4[1], format_chunk->SubFormat.Data4[2],
                 format_chunk->SubFormat.Data4[3], format_chunk->SubFormat.Data4[4],
                 format_chunk->SubFormat.Data4[5], format_chunk->SubFormat.Data4[6],
                 format_chunk->SubFormat.Data4[7]);
            return -2;
        }
    } else {
        LOGE("ERROR: Unsupported audio format %d!", format_chunk->AudioFmt_tag);
        return -2;
    }

    free(data_chunk->data);
    return 0;
}

int read_wav_info(FILE **fp, wav_ptr *wav)
{
    riff_chunk_ptr riff_chunk = &((*wav)->riff_chunk);
    format_chunk_ptr format_chunk = &((*wav)->format_chunk);
    sub_chunk_ptr data_chunk = &((*wav)->data_chunk);

    if (fread(riff_chunk, sizeof(riff_chunk_t), 1, *fp) == 0) {
        LOGE("ERROR: Read RIFF chunk failed!");
        return -3;
    }
    if (strncmp(riff_chunk->riff_type, "WAVE", ID_LENGTH) != 0) {
        LOGE("ERROR: This is not a wave file!");
        return -4;
    }
    LOGI("ChunkID:         %c%c%c%c", riff_chunk->chunk_id[0], riff_chunk->chunk_id[1],
         riff_chunk->chunk_id[2], riff_chunk->chunk_id[3]);
    LOGI("ChunkSize:       %u Bytes", riff_chunk->chunk_size);
    LOGI("type:            %c%c%c%c", riff_chunk->riff_type[0], riff_chunk->riff_type[1],
         riff_chunk->riff_type[2], riff_chunk->riff_type[3]);

    while (strncmp(format_chunk->chunk_id, "fmt ", ID_LENGTH) != 0) {
        if (fread(format_chunk->chunk_id, sizeof(char), ID_LENGTH, *fp) == 0) {
            LOGE("ERROR: Read format chunk id failed!");
            return -5;
        }
        if (fread(&format_chunk->chunk_size, sizeof(uint32_t), 1, *fp) == 0) {
            LOGE("ERROR: Read format chunk size failed!");
            return -6;
        }
        if (feof(*fp) != 0) { // feof need to be checked before fseek
            LOGE("ERROR: Can't find format_chunk!");
            return -7;
        }
        if (fseek(*fp, format_chunk->chunk_size, SEEK_CUR) != 0) {
            LOGE("ERROR: Read format chunk failed!");
            return -8;
        }
        LOGI("ChunkID:         %c%c%c%c", format_chunk->chunk_id[0], format_chunk->chunk_id[1],
             format_chunk->chunk_id[2], format_chunk->chunk_id[3]);
        LOGI("ChunkSize:       %u Bytes", format_chunk->chunk_size);
    }
    fseek(*fp, 0 - format_chunk->chunk_size, SEEK_CUR);
    if (fread(&format_chunk->AudioFmt_tag, format_chunk->chunk_size, 1, *fp) == 0) {
        LOGE("ERROR: Read format chunk failed!");
        return -9;
    }
    LOGI("AudioFormat:     %d", format_chunk->AudioFmt_tag);
    LOGI("NumberChannel:   %d", format_chunk->Num_Channel);
    LOGI("SampleRate:      %.1f kHz", (float)format_chunk->SampleRate / KILO);
    LOGI("bytes_per_sec:   %u", format_chunk->bytes_per_sec);
    LOGI("BlockAlign:      %d", format_chunk->BlockAlign);
    LOGI("bits_per_sam:    %d", format_chunk->bits_per_sam);
    if (format_chunk->AudioFmt_tag == 0xFFFE) {
        LOGI("cbSize:      %d", format_chunk->cbSize);
        LOGI("ValidBits:   %d", format_chunk->ValidBits);
        LOGI("ChannelMask: %d", format_chunk->ChannelMask);
        LOGI("SubFormat:   %08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
             format_chunk->SubFormat.Data1, format_chunk->SubFormat.Data2,
             format_chunk->SubFormat.Data3, format_chunk->SubFormat.Data4[0],
             format_chunk->SubFormat.Data4[1], format_chunk->SubFormat.Data4[2],
             format_chunk->SubFormat.Data4[3], format_chunk->SubFormat.Data4[4],
             format_chunk->SubFormat.Data4[5], format_chunk->SubFormat.Data4[6],
             format_chunk->SubFormat.Data4[7]);
    } else {
        LOGD2("\n");
    }

    while (strncmp(data_chunk->chunk_id, "data", ID_LENGTH) != 0) {
        if (fread(data_chunk->chunk_id, sizeof(char), ID_LENGTH, *fp) != ID_LENGTH) {
            LOGE("ERROR: Read chunk id failed!");
            return -10;
        }
        if (fread(&data_chunk->chunk_size, sizeof(uint32_t), 1, *fp) == 0) {
            LOGE("ERROR: Read chunk size failed!");
            return -11;
        }
        if (feof(*fp) != 0) { // feof need to be checked before fseek
            LOGE("ERROR: Can't find data chunk!");
            return -12;
        }
        fseek(*fp, data_chunk->chunk_size, SEEK_CUR);
        LOGI("ChunkID:         %c%c%c%c", data_chunk->chunk_id[0], data_chunk->chunk_id[1],
             data_chunk->chunk_id[2], data_chunk->chunk_id[3]);
        LOGI("ChunkSize:       %u Bytes", data_chunk->chunk_size);
    }
    if (strncmp(data_chunk->chunk_id, "data", ID_LENGTH) == 0) {
        fseek(*fp, 0 - data_chunk->chunk_size, SEEK_CUR);
    } else {
        LOGE("ERROR: Can't find data_chunk!");
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        LOGW("Usage: %s [wave file] <enable_data_debug>", argv[0]);
        return -1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        LOGE("Failed open file %s. Due to %s!", argv[1], strerror(errno));
        return -2;
    }

    wav_ptr wav = (wav_ptr)malloc(sizeof(wav_t));
    if (wav == NULL) {
        LOGE("Alloccate wav memory failed!");
        if (fp != NULL) {
            fclose(fp);
            fp = NULL;
        }
        if (wav != NULL) {
            free(wav);
            wav = NULL;
        }
        return -3;
    }

    if (read_wav_info(&fp, &wav) != 0) {
        if (fp != NULL) {
            fclose(fp);
            fp = NULL;
        }
        if (wav != NULL) {
            free(wav);
            wav = NULL;
        }
        return -4;
    }

    int enable_data_debug = argv[2] ? atoi(argv[2]) : 0;
    if (enable_data_debug) {
        if (read_and_print_data(&(wav->format_chunk), &(wav->data_chunk), fp) != 0) {
            if (fp != NULL) {
                fclose(fp);
                fp = NULL;
            }
            if (wav != NULL) {
                free(wav);
                wav = NULL;
            }
            return -5;
        }
    }

    if (fp != NULL) {
        if (fclose(fp) != 0) {
            LOGE("Failed close file %s. Due to %s!", argv[1], strerror(errno));
        }
        fp = NULL;
    }

    if (wav != NULL) {
        free(wav);
        wav = NULL;
    }
    return 0;
}
