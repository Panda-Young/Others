# **************************************************************************
# Description: 
# Version: 0.1.0
# Author: pandapan@aactechnologies.com
# Date: 2025-07-01 10:39:11
# Copyright (c) 2025 by @AAC Technologies, All Rights Reserved.
# **************************************************************************

# Prebuilt Oboe library configuration
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := oboe
LOCAL_SRC_FILES := ../oboe-1.9.3/libs/android.$(TARGET_ARCH_ABI)/liboboe.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../oboe-1.9.3/include  # Key: Add header file path
include $(PREBUILT_SHARED_LIBRARY)

# Main application configuration
include $(CLEAR_VARS)
LOCAL_MODULE := oboe_wav_player
LOCAL_SRC_FILES := ../OBOE_play_wav.cpp  # Correct source file path
LOCAL_SHARED_LIBRARIES := oboe           # Link with Oboe library
LOCAL_LDLIBS := -llog -lOpenSLES         # Add necessary Android libraries
include $(BUILD_EXECUTABLE)
