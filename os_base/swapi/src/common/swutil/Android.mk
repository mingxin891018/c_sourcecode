LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DLINUX -DANDROID

LOCAL_MODULE := libswutil

LOCAL_SRC_FILES += \
	base64.c \
	swurl.c \
	utf8.c \
	md5.c \
	crc32.c \
	des.c \
	swhashmap.c \
	swtimer.c \
	swmem.c \
	swtxtparser.c \
	timezone.c \
	swqueue.c


LOCAL_STATIC_LIBRARIES :=

LOCAL_SHARED_LIBRARIES :=

LOCAL_C_INCLUDES := \
	$(SWAPIROOT)/pub/base/include \
	$(SWAPIROOT)/pub/base/include/swos \
	$(SWAPIROOT)/pub/common/include \
	$(SWAPIROOT)/pub/common/include/swutil \

include $(BUILD_STATIC_LIBRARY)
