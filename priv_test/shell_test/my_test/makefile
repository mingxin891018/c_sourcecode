include test.mk
name := "dddddddddddd"
#date := `date +%Y%m%d%H%M%S`
date := $(shell date +%Y%m%d%H%M%S)
LS := $(shell ls)
.PHONY:all
all:
	echo "00000"

ifeq ($(TEST1),y)
	@echo "11111111"
else
	@echo "22222222"
endif
	@echo $(date)
	@echo $(LS)


