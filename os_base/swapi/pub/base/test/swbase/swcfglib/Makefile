#############################################
# Author: Dou Hongchen, WangChao            #
# 2006-02-23 last update by Dou Hongchen    #
# 2007-03-06 last update by WangChao	    #
#############################################


NAME := test_cfglib
BUILD_TARGET_TYPE := exe
BINDIR = .
CFLAGS += -DSUPPORT_TEST
SRC += test_cfglib.c ../../skeleton.c


#头文件搜索路径
INCDIR = -I.								\
			-I../../include					\
			-I../../../include/swbase 		\
			-I../../../include/system/linux 		\
			-I../../../include
			

ifeq ($(findstring exe,$(BUILD_TARGET_TYPE)), exe)
DYN_LDS_WITH += -L../../../libs/$(SW_PLATFORM) 	\
-lswbase \
-lswos  
endif


#核心规则
-include ../../../build/common.Makefile

copy:
	cp -f $(BINDIR)/$(NAME) $(RELEASEDIR)/rootfs/usr/local/bin
