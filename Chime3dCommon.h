/* **************************************************************
 * @Description: 
 * @Date: 2024-02-06 17:35:44
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * @Copyright (c) 2024 by @AAC Technologies, All Rights Reserved. 
 **************************************************************/
#ifndef __CHIME_3D_COMMON_H__
#define __CHIME_3D_COMMON_H__

#include <stdio.h>
#include <stdint.h>

#pragma pack(1)
typedef struct net_header
{
    int16_t packet_type;
    int32_t chunk_size;
} net_header_t;
#pragma pack()

#pragma pack(1)
typedef struct _ObjectPos
{
	int8_t id;
	float x;
	float y;
	float z;
} ObjectPos;
#pragma pack()

#pragma pack(1)
typedef struct _TimedPos
{
	int16_t time;
	float x;
	float y;
	float z;
} TimedPos;
#pragma pack()

#pragma pack(1)
typedef struct _PlaybackInfo
{
	int16_t audio_id;
	int8_t channels;  // ��Դ��������
	int8_t is_loop;   // 1��ѭ�����ţ�0����ѭ��
	int16_t interval; // ѭ���������λms
	int8_t automation_enabled;
} PlaybackInfo;
#pragma pack()

#pragma pack(1)
typedef struct _AudioAssets
{
	int16_t count;
	int16_t audio_id[1];
} AudioAssets;
#pragma pack()

#pragma pack(1)
typedef struct _AutomationPoints
{
	int16_t audio_id;
	int16_t count;
	TimedPos points[1];
} AutomationPoints;
#pragma pack()

enum {
	PT_UPDATE_POS_SPEAKER = 1,
	PT_UPDATE_POS_AUDIO_SOURCE,
	PT_UPDATE_POS_LISTENER,
	PT_UPDATE_ALGORITHM_MODE,
	PT_UPDATE_AUTOMATION,
	PT_ENABLE_AUTOMATION,
	PT_CHANGE_SPEAKER_COUNT,
	PT_PLAYER_START = 10,
	PT_PLAYER_STOP,
	PT_REPORT_AUDIO_ASSETS = 100
};

#endif // __CHIME_3D_COMMON_H__
