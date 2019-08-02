#############################################
# Author: NiuJiuRu                          #
# 2007-12-5 last update by NiuJiuRu	    #
#############################################

#模块名称
NAME := swxml

BUILD_TARGET_TYPE := dll

#源码文件
SRC += document.c element.c map.c membuf.c node.c nodelist.c parser.c swxml.c

#头文件搜索路径
INCDIR = -I. -I$(SWAPIROOT)/include 		\
			-I$(SWAPIROOT)/include/swos \
			-I$(SWAPIROOT)/include/swutil \
			-I$(SWAPIROOT)/include/swxml

#核心规则
-include $(SWAPIROOT)/build/common.Makefile
