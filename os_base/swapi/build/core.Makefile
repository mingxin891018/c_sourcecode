#############################################
# Author: Dou Hongchen                      #
# 2007-03-08 last update by Dou Hongchen    #
# 2010-07-05 last update by lilei           #
#############################################

#############################################
# 用户设置变量说明
#############################################
# NAME : 模块名称
# BUILD_TARGET_TYPE : 编译目标类型,取值static, exe, dll
# INCDIR : 搜索目录列表(目前前加 -I)
# SRC : 源码文件
# TEST_SRC : 测试源码文件
# BINDIR : 生成库或可执行文件的路径
# DYN_LDS_WITH : 需要动态链接的库
# STATIC_LDS_WITH : 需要静态链接的库
# DEPS_TARGET : 扩展依赖对象
#
# DEBUG : 是否debug, 取值(y,n)
# PRINT_COMPILER : 是否打印编译信息, 取值(y, n)
#
# AR : 函数库打包器
# AS : 汇编器
# CC : C编译器
# CXX : C++编译器
# CPP : C预处理器
# LD : 链接器
# ARFLAGS : 函数库打包选项
# ASFLAGS : 汇编选项
# CFLAGS : C编译选项
# CXXFLAGS : C++编译选项
# CPPFLAGS : C预处理选项
# LDFLAGS : 链接选项
# RM : 删除命令，默认是"rm -f"
#############################################
##############################################
#	初始设置build输出目录
##############################################
ROOTDIR ?= $(SWAPIROOT)
SUB_OUT_DIR := $(subst $(subst /swapp/..,,$(ROOTDIR))/,,$(shell pwd))
BUILD_OUT_PATH := $(ROOTDIR)/out/$(SW_PLATFORM)/$(SUB_OUT_DIR)

# C++文件后缀
CCSUFIX = cpp
# 删除命令
RM = rm -f
ifeq ($(PRINT_COMPILER), y)
Q_ :=
else
Q_ := @
endif

# 编译条件设置
ifeq ($(DEBUG), y)
CFLAGS += -g
else
CFLAGS += -O2
endif

#栈保护
CFLAGS += -fstack-protector
#地址无关
CFLAGS += -fPIC

# C编译选项
CFLAGS += $(INCDIR) 
# C++编译选项
CXXFLAGS += $(CFLAGS)
# 编译预处理选项
CFLAGS += -MD

# 默认静态库
TARGET := lib$(NAME).a

#GOT表保护
LDFLAGS += -Wl,-zrelro
#堆栈不可执行
LDFLAGS += -Wl,-znoexecstack 

ifeq ($(findstring dll,$(BUILD_TARGET_TYPE)), dll)
TARGET := lib$(NAME).so
LDFLAGS += -shared -fPIC
endif

ifeq ($(findstring static,$(BUILD_TARGET_TYPE)), static)
TARGET := lib$(NAME).a
endif

ifeq ($(findstring exe,$(BUILD_TARGET_TYPE)), exe)
TARGET := $(NAME)
SRC += $(TEST_SRC)
endif

# 编译条件处理
OBJ += $(patsubst %.c,$(BUILD_OUT_PATH)/%.o,$(patsubst %.$(CCSUFIX),$(BUILD_OUT_PATH)/%.o,$(SRC))) 
BIN := $(BINDIR)/$(TARGET) 

# 获取配置库信息
TCFG_LIB_INFO :=
TCFG_LIB_INFO +=$(shell export LC_ALL=C && git status | awk 'NR==1' | cut -c13-30)
TCFG_LIB_INFO +=$(shell export LC_ALL=C && git log -1 --pretty=format:"%H")
TCFG_LIB_INFO +=$(shell export LC_ALL=C && git status -s ./ | awk 'NR<=10')

CFG_LIB_INFO := $(subst >,?, $(TCFG_LIB_INFO))

CFLAGS += -DSW_CFG_LIB_INFO='"$(CFG_LIB_INFO)"'

CFG_LIB_OBJ := $(BUILD_OUT_PATH)/$(NAME)cfginfo.o
CLEAN_OBJ += $(CFG_LIB_OBJ)
OBJ += $(CFG_LIB_OBJ)


.PHONY:	all clean distclean tags deps release $(DEPS_TARGET) copy

all: $(DEPS_TARGET) $(BIN) 

$(BUILD_OUT_PATH)/$(NAME)cfginfo.o:
	$(Q_)if [ 1 = 1 ]; then \
	echo 'static char* $(NAME)cfginfo =' > $(BUILD_OUT_PATH)/$(NAME)cfginfo.c;	\
	echo '"'$(CFG_LIB_INFO)'";' >> $(BUILD_OUT_PATH)/$(NAME)cfginfo.c;	\
	echo 'char * sw_get_$(NAME)_cfginfo(void)' >> $(BUILD_OUT_PATH)/$(NAME)cfginfo.c;	\
	echo '{return $(NAME)cfginfo; }' >> $(BUILD_OUT_PATH)/$(NAME)cfginfo.c;	\
	fi;
	$(Q_)$(CC) -fno-short-enums -MD -fpic -c $(BUILD_OUT_PATH)/$(NAME)cfginfo.c -o $(CFG_LIB_OBJ)

## 增加依赖文件
ifeq ($(BUILD_OUT_PATH), $(wildcard $(BUILD_OUT_PATH)))
include $(wildcard $(addsuffix /*.d,$(shell find $(BUILD_OUT_PATH) -type d)))
endif


## 重定义隐含规则
#%.d: %.c
#    @$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
#    sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
#    $(RM) -f $@.$$$$

$(BUILD_OUT_PATH)/%.o: %.c
	@echo '<$(CC)>[$(DEBUG)] Compiling object file "$@" ...'
	$(Q_)mkdir -p $(dir $@)
	$(Q_)${CC} $(CFLAGS) -c $< -o $@

$(BUILD_OUT_PATH)/%.o: %.C
	@echo '<$(CC)>[$(DEBUG)] Compiling object file "$@" ...'
	$(Q_)mkdir -p $(dir $@)
	$(Q_)${CC} $(CFLAGS) -c $< -o $@

$(BUILD_OUT_PATH)/%.o: %.$(CCSUFIX)
	@echo '<$(CXX)>[$(DEBUG)] Compiling object file "$@" ...'
	$(Q_)mkdir -p $(dir $@)
	$(Q_)${CXX} $(CXXFLAGS) -c $< -o $@

$(BUILD_OUT_PATH)/%.o: %.S
	@echo '<$(CC)>[$(DEBUG)] Compiling object file "$@" ...'
	$(Q_)mkdir -p $(dir $@)
	$(Q_)${CC} $(CFLAGS)  -c $< -o $@

clean:
	@echo remove all objects
	$(Q_)$(RM) -rf $(BUILD_OUT_PATH) $(BIN) $(CLEAN_OBJ) *.d



distclean:
	@echo remove all objects and deps
	$(Q_)$(RM) -rf $(BUILD_OUT_PATH) $(BIN) $(CLEAN_OBJ) *.d

# Rebuild
rebuild: distclean all

$(BIN): $(OBJ) 
ifeq ($(BINDIR), $(wildcard $(BINDIR)))
	@echo
else
	mkdir -p $(BINDIR)
endif
ifeq ($(findstring exe,$(BUILD_TARGET_TYPE)), exe)
	@echo '<$(LD)>creating binary "$(BIN)"'
	$(Q_)$(LD)  $(LDFLAGS) $(OBJ)  $(DYN_LDS_WITH) $(STATIC_LDS_WITH) -o $(BIN) && chmod a+x $(BIN)
else
ifeq ($(findstring dll,$(BUILD_TARGET_TYPE)), dll)
	@echo '<$(LD)>creating dll "$(BIN)"'
	$(Q_)$(LD)  $(LDFLAGS) $(OBJ) $(STATIC_LDS_WITH) $(DYN_LDS_WITH) -o $(BIN) && chmod a+x $(BIN)
else
	@echo '<$(AR)>creating static lib "$(BIN)"'
	$(Q_)$(AR) rc $@ $^
endif
endif
	@echo '... done'
	@echo $(CFG_LIB_INFO)
	@echo

#copy:
#	cp -rf $(BIN) $(RELEASEDIR)/rootfs/usr/lib
