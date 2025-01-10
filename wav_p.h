/***************************************************************************
 * Description: wav header
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2024-05-02 00:30:12
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/
#ifndef WAV_UTILS_H
#define WAV_UTILS_H

#include <stdint.h>

#define ID_LENGTH 4
#define KILO 1000
#define EXTENTION_LENGTH 16

#pragma pack(push, 1)
typedef struct {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
} GUID_t;

static const GUID_t KSDATAFORMAT_SUBTYPE_PCM = {0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};
static const GUID_t KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = {0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};

typedef struct {
    int8_t chunk_id[ID_LENGTH];  // 'RIFF'
    uint32_t chunk_size;         // 4 byte size of the traditional RIFF/WAVE file
    int8_t riff_type[ID_LENGTH]; // 'WAVE'
} riff_chunk_t, *riff_chunk_ptr;

typedef struct {                // declare FormatChunk structure
    int8_t chunk_id[ID_LENGTH]; // 'fmt '
    uint32_t chunk_size;        // 4 byte size of the 'fmt ' chunk
    uint16_t AudioFmt_tag;      // Indicates the format of audio data. If 1, means PCM; if 3, means IEEE_FLOAT; if 0xFFFE, means WAVE_FORMAT_EXTENSIBLE
    uint16_t Num_Channel;       // 1 = mono, 2 = stereo, etc.
    uint32_t SampleRate;        // 32000, 44100, 48000, etc
    uint32_t bytes_per_sec;     // bytes_per_sec = samples_per_sec * channels * bits_per_sam / 8
                                // The audio code rate, the number of bytes played per second.
                                // you can estimate the size of the buffer used. Only important for compressed formats
    uint16_t BlockAlign;        // container size (in bytes) of one set of samples
    uint16_t bits_per_sam;      // valid bits per sample 16, 24 or 32
} format_chunk_t, *format_chunk_ptr;

typedef struct {
    uint16_t cbSize;      // extra information (after cbSize) to store
    uint16_t ValidBits;   // valid bits per sample i.e. 8, 16, 24, 32
    uint32_t ChannelMask; // channel mask for channel allocation
    GUID_t SubFormat;
} extra_info_chunk_t, *extra_info_chunk_ptr; // 24 bytes

typedef struct {
    int8_t chunk_id[ID_LENGTH];
    uint32_t chunk_size;
    int8_t chunkData[];
} sub_chunk_t, *sub_chunk_ptr;

typedef struct {               // declare JunkChunk structure
    int8_t chunkId[ID_LENGTH]; // 'JUNK' or 'junk'
    uint32_t chunkSize;        // 4 byte size of the 'JUNK' chunk.
    // This must be at least 28 if the chunk is intended as a place-holder for a 'ds64' chunk.
    int8_t chunkData[]; // dummy bytes
} junk_chunk_t, *junk_chunk_ptr;

typedef struct {
    riff_chunk_t riff_chunk;
    format_chunk_t format_chunk;
    sub_chunk_t data_chunk;
} std_wav_t, *std_wav_ptr; // 44 Bytes to offset
#pragma pack(pop)

#endif // WAV_UTILS_H
