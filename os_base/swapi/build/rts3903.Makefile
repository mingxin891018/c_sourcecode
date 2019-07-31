#hisilicon define, gcc 4.4.0
CC:=$(TOOL_PREFIX)gcc
CXX:=$(TOOL_PREFIX)g++
LINK:=$(TOOL_PREFIX)gcc
AR:=$(TOOL_PREFIX)ar
LD:=$(TOOL_PREFIX)g++


ifndef BINDIR
BINDIR := ../../pub/$(NAME)/libs/$(SW_PLATFORM)
endif

ifndef BUILD_TARGET_TYPE
BUILD_TARGET_TYPE := dll
endif

INCDIR += \
        -I$(SDKDIR)/pub/include

CFLAGS += -DLINUX -D_REENTRANT -D_GNU_SOURCE #-march=armv7-a -mcpu=cortex-a9 -mfloat-abi=softfp -mfpu=vfpv3-d16 -fno-strict-aliasing

ifeq ($(findstring exe,$(BUILD_TARGET_TYPE)), exe)
DYN_LDS_WITH += -lpthread
DYN_LDS_WITH += -lm -lrt -ldl
endif
