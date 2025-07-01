# **************************************************************************
# Description: 
# Version: 0.1.0
# Author: pandapan@aactechnologies.com
# Date: 2025-07-01 10:39:11
# Copyright (c) 2025 by @AAC Technologies, All Rights Reserved.
# **************************************************************************

# 预编译Oboe库配置
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := oboe
LOCAL_SRC_FILES := ../oboe/libs/android.$(TARGET_ARCH_ABI)/liboboe.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../oboe/include  # 关键：添加头文件路径
include $(PREBUILT_SHARED_LIBRARY)

# 主程序配置
include $(CLEAR_VARS)
LOCAL_MODULE := oboe_wav_player
LOCAL_SRC_FILES := ../OBOE_play_wav.cpp  # 修正源文件路径
LOCAL_SHARED_LIBRARIES := oboe           # 链接Oboe库
LOCAL_LDLIBS := -llog -lOpenSLES         # 添加必要的Android库
include $(BUILD_EXECUTABLE)
