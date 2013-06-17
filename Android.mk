LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS += -Wall -O2
LOCAL_MODULE := libplhw
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES := $(call all-subdir-c-files)
include $(BUILD_STATIC_LIBRARY)
