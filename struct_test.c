/* **************************************************************
 * @Description: parse chime config header
 * @Date: 2024-05-27 13:59:34
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * @Copyright (c) 2024 by @AAC Technologies, All Rights Reserved. 
 **************************************************************/
#ifndef __H_PARSER__
#define __H_PARSER__

#include <stdint.h>

#pragma pack(1)
typedef struct _ObjectPos {
    int8_t id;
    float x;
    float y;
    float z;
} ObjectPos;
#pragma pack()

#pragma pack(1)
typedef struct _TimedPos {
    int16_t time;
    float x;
    float y;
    float z;
} TimedPos;
#pragma pack()

#pragma pack(1)
typedef struct _AutomationPoints
{
	int16_t count;
	TimedPos points[1];
} AutomationPoints;
#pragma pack()

#pragma pack(push, 1)
typedef struct _AACCfgFileHeader {
    char formatId[6];
    uint16_t version; // roll up when data struct is significantly changed
} AACCfgFileHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _AACCfgChunkHeader {
    uint16_t chunkType;
    uint32_t chunkDataSize;
} AACCfgChunkHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _SoundField {
    uint16_t sourceId;
    int16_t positionId;
    uint8_t isLoop;
    uint16_t loopInterval;
    uint8_t algorithmMode;
    ObjectPos listenerPos;
    ObjectPos audioSourcePos;
    uint8_t automationEnabled;
} SoundField;
#pragma pack(pop)

typedef enum ChunkType {
    CHUNK_SPEAKER_POS = 1,
    CHUNK_SOUND_FIELD,
    CHUNK_AUDIO_AUTOMATION
} ChunkType;

typedef struct Parse3dcContent {
    int set_done;
    SoundField sound_field;
    ObjectPos SpkPosData[12];
    uint8_t speaker_count;
    uint16_t point_count;
    TimedPos timeSourcePosData[500];
    int multi_position_number;
    ObjectPos multiSourcePos[6];
    ObjectPos multiListenerPos[6];
} Parse3dcContent_t, *p_Parse3dcContent_t;

typedef struct {
    int id;
    char fileName[64];
} PosToFile_t, *PosToFile_ptr;

typedef struct {
    int position_number;
    PosToFile_ptr PosToFileArray;
} xmlData_t, *xmlData_ptr;

#pragma pack(1)
typedef struct {
    AACCfgFileHeader file_header;
    AACCfgChunkHeader speaker_chunk_header;
    uint8_t speaker_count;
    ObjectPos SpkPosData[12];
    AACCfgChunkHeader sound_field_chunk_header;
    SoundField sound_field;
} ChimeConfig_t, *p_ChimeConfig_t;
#pragma pack()

#endif

#include <stdio.h>

int main() {
    ChimeConfig_t chime_config = {0};
    printf("sizeof(ChimeConfig_t) = %lu\n", sizeof(ChimeConfig_t));
    return 0;
}
