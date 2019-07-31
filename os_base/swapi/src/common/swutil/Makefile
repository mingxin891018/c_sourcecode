#############################################
# Author: Dou Hongchen, WangChao            #
# 2006-02-23 last update by Dou Hongchen    #
# 2007-03-06 last update by WangChao	    #
#############################################

#模块名称和版本号
NAME := swutil
BUILD_TARGET_TYPE := dll

BINDIR = ../../../pub/common/libs/$(SW_PLATFORM)

#源码文件
SRC += 										\
base64.c 									\
swurl.c 									\
utf8.c 										\
md5.c 										\
crc32.c 									\
des.c 										\
swhashmap.c 								\
swtimer.c 									\
swmem.c 									\
swtxtparser.c								\
timezone.c									\
swgethostbyname.c									\
swqueue.c       \
hmac_sha256.c


#头文件搜索路径
INCDIR +=									\
-I.											\
-I../../../pub/base/include					\
-I../../../pub/base/include/swos			\
-I../../../pub/common/include 			\
-I../../../pub/common/include/swutil		

#核心规则
-include ../../../build/common.Makefile

copy:
	cp -f $(BINDIR)/lib$(NAME).so	  $(RELEASEDIR)/rootfs/usr/lib


