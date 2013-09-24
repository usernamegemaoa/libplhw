LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS += -Wall -O2
LOCAL_MODULE := libplhw
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES := \
	adc11607.c \
	cpld.c \
	dac5820.c \
	eeprom.c \
	gpioex.c \
	hvpmic.c \
	i2cdev.c \
	pbtn.c \
	util.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../plsdk/libplutil
include $(BUILD_STATIC_LIBRARY)
