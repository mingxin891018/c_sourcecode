CFLAGS+=-Wall
#栈保护
CFLAGS += -fstack-protector
#地址无关
CFLAGS += -fPIC
# 编译条件设置
ifeq ($(DEBUG), y)
CFLAGS += -g
else
CFLAGS += -O2
endif

#头文件搜索路径
INCDIR += -I./ 
#链接的库
STATIC_LIB += ./libssl.a ./libcrypto.a
SHARED_LID += -ldl -lpthread
#GOT表保护
LDFLAGS += -Wl,-zrelro
#堆栈不可执行
LDFLAGS += -Wl,-znoexecstack 

CC=gcc


TARGET=rsa_test
TARGETDIR=./output
BUILD_TARGET_TYPE=exe
TARGET_O=$(TARGET).o
SRC_C=$(shell ls *.c)
#SRC_H=$(shell ls *.h)
OBJ_C=$(patsubst %.c,%.o,$(SRC_C))

ifeq ($(findstring dll,$(BUILD_TARGET_TYPE)), dll)
LDFLAGS += -shared -fpic
OUTFILE=$(TARGET).so
endif
ifeq ($(findstring static,$(BUILD_TARGET_TYPE)), static)
OUTFILE=$(TARGET).a
endif

ifeq ($(findstring exe,$(BUILD_TARGET_TYPE)), exe)
OUTFILE=$(TARGET)
endif

%.o:%.c
	@echo '<$(CC)>[$(DEBUG)] Compiling object file "$@" ...'
	$(CC) -c $< -o $@ $(CFLAGS) $(INCDIR)


ifeq ($(PRINT_COMPILER), y)
Q_ :=
else
Q_ := @
endif


$(TARGET): $(TARGET_O)
ifeq ($(TARGETDIR), $(wildcard $(TARGETDIR)))
	@echo
else
	mkdir -p $(TARGETDIR)
endif

ifeq ($(findstring exe,$(BUILD_TARGET_TYPE)), exe)
	@echo '<$(LD)>creating binary "$(TARGET)"'
	$(Q_)$(CC)  $(LDFLAGS) $(TARGET_O) $(STATIC_LIB) $(SHARED_LID) -o $(TARGET) && chmod a+x $(OUTFILE)
else
ifeq ($(findstring dll,$(BUILD_TARGET_TYPE)), dll)
	@echo '<$(LD)>creating dll "$(TARGET)"'
	$(Q_)$(CC)  $(LDFLAGS) $(TARGET_O) $(STATIC_LIB) $(SHARED_LID) -o $(OUTFILE) && chmod a+x $(OUTFILE)
else
	@echo '<$(AR)>creating static lib "$(TARGET)"'
	$(Q_)$(AR) rc $(OUTFILE) $^
endif
endif
	@echo '... done'
	@echo $(CFG_LIB_INFO)
	@
	@echo


clean:
	rm -rf $(OBJ_C) $(OUTFILE) *.pem
