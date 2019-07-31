###########################################
# Author: Dou Hongchen, WangChao            #
# 2006-02-23 last update by Dou Hongchen    #
# 2007-03-06 last update by WangChao	    #
#############################################

NAME := swos
ifeq ($(NDK), y)
BUILD_TARGET_TYPE := static
CFLAGS += -DANDROID
else
BUILD_TARGET_TYPE := dll
endif
BINDIR = ../../../pub/base/libs/$(SW_PLATFORM)

SRC +=swtcp.c swudp.c swmutex.c swsignal.c swthrd.c swos.c

ifeq ($(SUPPORT_LIBC),y)
SRC	+= string.c
endif

#头文件搜索路径
INCDIR +=									\
-I.											\
-I../../../pub/base/include					\
-I../../../pub/base/include/swos 				\
-I../../../pub/base/include/common			
			


#核心规则
-include ../../../build/common.Makefile

copy:
	cp -f $(BINDIR)/lib$(NAME).*    $(RELEASEDIR)/rootfs/usr/lib
