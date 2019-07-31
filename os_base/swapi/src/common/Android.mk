LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DLINUX -DANDROID

OPENSSL_VER ?= openssl-0.9.8h

$(warning ----- $(OPENSSL_VER))

ifeq ($(OPENSSL_VER), openssl-1.0.1f)
	LOCAL_MODULE := libswcommon_ssl_101f
else
	LOCAL_MODULE := libswcommon
endif

ifeq ($(SUPPORT_SWAPI30), y)
LOCAL_CFLAGS += -DSUPPORT_SWAPI30
LOCAL_CFLAGS += -DSUPPORT_HTTPS
endif

ifeq ($(findstring y,$(SUPPORT_SECURE_LOG)), y)
LOCAL_CFLAGS += -DSUPPORT_SECURE_LOG
endif

LOCAL_SRC_FILES += \
	swfile/swhttpfile_priv.c \
	swfile/swrawfile.c \
	swfile/swhttpfile.c \
	swfile/swftpfile.c \
	swfile/swfile.c \
	swftp/swftpclient.c \
	swhttp/swhttpclient.c \
	swhttp/swhttpserver.c \
	swhttp/swhttpauth.c \
	swxml/document.c \
	swxml/element.c \
	swxml/map.c \
	swxml/membuf.c \
	swxml/node.c \
	swxml/nodelist.c \
	swxml/parser.c \
	swxml/swxml.c

ifeq ($(SUPPORT_SWAPI30), y)
LOCAL_SRC_FILES += \
	swhttp/swhttpsclient.c
endif

LOCAL_SRC_FILES += \
	swutil/base64.c \
	swutil/swurl.c \
	swutil/utf8.c \
	swutil/md5.c \
	swutil/crc32.c \
	swutil/des.c \
	swutil/swhashmap.c \
	swutil/swtimer.c \
	swutil/swmem.c \
	swutil/swtxtparser.c \
	swutil/timezone.c \
	swutil/swgethostbyname.c        \
	swutil/swqueue.c

LOCAL_STATIC_LIBRARIES :=

LOCAL_C_INCLUDES := \
	$(SWAPIROOT)/pub/base/include \
	$(SWAPIROOT)/pub/base/include/swos \
	$(SWAPIROOT)/pub/common/include/  \
	$(SWAPIROOT)/pub/common/include/$(OPENSSL_VER) \
	$(SWAPIROOT)/pub/common/include/swutil \
	$(SWAPIROOT)/pub/common/include/swxml \
	
LOCAL_PRELINK_MODULE := false

#export USE_SHARED=y
ifeq ($(USE_SHARED), y)
LOCAL_SHARED_LIBRARIES := libswbase liblog
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
else
include $(BUILD_STATIC_LIBRARY)
endif
