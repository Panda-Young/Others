/* ------------------------------------------------------------------
* Copyright (C) 2009 Martin Storsjo
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
* express or implied.
* See the License for the specific language governing permissions
* and limitations under the License.
* -------------------------------------------------------------------
*/

#ifndef WAV_H_
#define WAV_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// 定义WAV文件的头部结构
typedef struct {
    char chunkId[4];
    uint32_t chunkSize;
    char format[4];
    char subchunk1Id[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char subchunk2Id[4];
    uint32_t subchunk2Size;
} WavHeader;

struct wav_reader {
	FILE *wav;
	long data_pos;
	uint32_t data_length;
	uint32_t data_left;

	int format;
	int sample_rate;
	int bits_per_sample;
	int channels;
	int byte_rate;
	int block_align;

	int streamed;
};

void* wav_read_open(const char *filename);
void wav_read_close(void* obj);

int wav_get_header(void* obj, int* format, int* channels, int* sample_rate, int* bits_per_sample, unsigned int* data_length);
int wav_read_data(void* obj, unsigned char* data, unsigned int length);

WavHeader makeWavHeader(uint32_t sampleRate, uint16_t bitDepth, uint16_t numChannels, uint32_t numSamples);
void writeWavHeader(FILE *file, WavHeader *header);


#ifdef __cplusplus
}
#endif

#endif
