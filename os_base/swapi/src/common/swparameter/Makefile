#############################################
# Author: Dou Hongchen, WangChao            #
# 2006-02-23 last update by Dou Hongchen    #
# 2007-03-06 last update by WangChao	    #
#############################################

#模块名称和版本号
NAME := swparameter
#BUILD_TARGET_TYPE := static   3798m 8v9producttest need static
ifeq ($(NDK), y)
BUILD_TARGET_TYPE := static
else
BUILD_TARGET_TYPE := dll
endif
BINDIR := ../../../pub/common/libs/$(SW_PLATFORM)

SRC +=									\
swifiledepot.c			\
swparameter.c				

#头文件搜索路径
INCDIR += \
-I.										\
-I../../../pub/base/include					\
-I../../../pub/base/include/swos	            \
-I../../../pub/common/include			\
-I$(SWPARTSROOT)/pub/include/$(OPENSSL_VER) 		\
-I../../../pub/common/include/swutil			\

ifeq ($(SUPPORT_3716MV310),y)
CFLAGS +=-DSUPPORT_3716MV310
endif
ifeq ($(SUPPORT_3716MV410),y)
CFLAGS +=-DSUPPORT_3716MV410
endif
ifeq ($(SUPPORT_3798M),y)
CFLAGS +=-DSUPPORT_3798M
endif

#核心规则
-include $(SWAPIROOT)/build/common.Makefile

copy:
	cp -f $(BINDIR)/lib$(NAME).so  $(RELEASEDIR)/rootfs/usr/local/lib
