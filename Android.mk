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
	max17135.c \
	tps65185.c \
	i2cdev.c \
	pbtn.c \
	util.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libplutil
include $(BUILD_STATIC_LIBRARY)
