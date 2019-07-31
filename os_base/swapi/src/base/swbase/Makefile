#############################################
# Author: Bian Zhonglin                     #
# Create: 2018/08/23                        #
#############################################

#模块名称和版本号
NAME := swbase

BUILD_TARGET_TYPE := dll

BINDIR = ../../../pub/base/libs/$(SW_PLATFORM)

#源码文件
SRC += $(wildcard ./*.c)

#头文件搜索路径
INCDIR += 						\
-I. 							\
-I${SWAPIROOT}/pub/base/include 	\
-I${SWAPIROOT}/pub/base/include/swos \
-I${SWAPIROOT}/pub/common/include


#核心规则
-include ${ROOTDIR}/swapi/build/common.Makefile
