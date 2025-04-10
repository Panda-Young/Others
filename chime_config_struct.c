/* **************************************************************
 * @Description: parse chime config header
 * @Date: 2024-05-27 13:59:34
 * @Version: 0.1.0
 * @Author: Panda-Young
 * @Copyright (c) 2024 by @Panda-Young, All Rights Reserved.
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
typedef struct _AutomationPoints {
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

typedef enum {
    CHUNK_SPEAKER_POS = 1,
    CHUNK_SOUND_FIELD,
    CHUNK_AUDIO_AUTOMATION
} ChunkType_t,
    *ChunkType_ptr;

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

void set_speaker_pos(ChimeConfig_t *chime_config, int id, float x, float y, float z)
{
    for (int i = 0; i < chime_config->speaker_count; i++) {
        if (chime_config->SpkPosData[i].id == id) {
            chime_config->SpkPosData[i].x = x;
            chime_config->SpkPosData[i].y = y;
            chime_config->SpkPosData[i].z = z;
            break;
        }
    }
}

void check_info(ChimeConfig_t *chime_config)
{
    printf("file_header.formatId = %6s\n", chime_config->file_header.formatId);
    printf("file_header.version = %d\n\n", chime_config->file_header.version);
    printf("speaker_chunk_header.chunkType = %d\n", chime_config->speaker_chunk_header.chunkType);
    printf("speaker_chunk_header.chunkDataSize = %d\n", chime_config->speaker_chunk_header.chunkDataSize);
    printf("speaker_count = %d\n", chime_config->speaker_count);
    for (int i = 0; i < chime_config->speaker_count; i++) {
        printf("speaker[%d].id = %d x = %f y = %f z = %f\n",
               i, chime_config->SpkPosData[i].id, chime_config->SpkPosData[i].x,
               chime_config->SpkPosData[i].y, chime_config->SpkPosData[i].z);
    }
    printf("\n");
    printf("sound_field_chunk_header.chunkType = %d\n", chime_config->sound_field_chunk_header.chunkType);
    printf("sound_field_chunk_header.chunkDataSize = %d\n", chime_config->sound_field_chunk_header.chunkDataSize);
    printf("sound_field.sourceId = %d\n", chime_config->sound_field.sourceId);
    printf("sound_field.positionId = %d\n", chime_config->sound_field.positionId);
    printf("sound_field.isLoop = %d\n", chime_config->sound_field.isLoop);
    printf("sound_field.loopInterval = %d\n", chime_config->sound_field.loopInterval);
    printf("sound_field.algorithmMode = %d\n", chime_config->sound_field.algorithmMode);
    printf("sound_field.listenerPos.id = %d x = %f y = %f z = %f\n",
           chime_config->sound_field.listenerPos.id, chime_config->sound_field.listenerPos.x,
           chime_config->sound_field.listenerPos.y, chime_config->sound_field.listenerPos.z);
    printf("sound_field.audioSourcePos.id = %d x = %f y = %f z = %f\n",
           chime_config->sound_field.audioSourcePos.id, chime_config->sound_field.audioSourcePos.x,
           chime_config->sound_field.audioSourcePos.y, chime_config->sound_field.audioSourcePos.z);
    printf("sound_field.automationEnabled = %d\n", chime_config->sound_field.automationEnabled);
}

int main(int argc, char *argv[])
{
    ChimeConfig_t chime_config = {0};
    printf("sizeof(ChimeConfig_t) = %lu\n", sizeof(ChimeConfig_t));
    if (argc == 2) {
        FILE *fp = fopen(argv[1], "rb+");
        if (fp == NULL) {
            printf("open file error\n");
            return -1;
        }
        fread(&chime_config, sizeof(ChimeConfig_t), 1, fp);
        printf("\n\noriginal data:\n");
        check_info(&chime_config);
        set_speaker_pos(&chime_config, 0, 2.4f, 0.0f, 2.7f);
        set_speaker_pos(&chime_config, 1, -2.4f, 0.0f, 2.7f);
        set_speaker_pos(&chime_config, 3, 2.4f, 0.0f, -1.0f);
        set_speaker_pos(&chime_config, 4, -2.4f, 0.0f, -1.0f);
        printf("\n\nmodified data:\n");
        check_info(&chime_config);
        fseek(fp, 0, SEEK_SET);
        fwrite(&chime_config, sizeof(ChimeConfig_t), 1, fp);
        fflush(fp);
        fclose(fp);
    }

    return 0;
}
