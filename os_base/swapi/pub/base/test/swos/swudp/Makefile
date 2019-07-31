#############################################
# Author: Dou Hongchen, WangChao            #
# 2006-02-23 last update by Dou Hongchen    #
# 2007-03-06 last update by WangChao	    #
#############################################


NAME := test_udp
BUILD_TARGET_TYPE := exe
BINDIR = .
CFLAGS += -DSUPPORT_TEST
SRC += $(wildcard ./*.c) ../../skeleton.c


#头文件搜索路径
INCDIR = -I.								\
			-I../../					\
			-I../../../include/swbase 		\
			-I../../../include/swos 		\
			-I../../../include
			

DYN_LDS_WITH += -L$(SWAPIROOT)/pub/base/libs/$(SW_PLATFORM) -lswbase -lswos

#核心规则
-include ../../../../../build/common.Makefile

copy:
	cp -f $(BINDIR)/$(NAME) $(RELEASEDIR)/rootfs/usr/local/bin
