.PHONY: all clean
all:
	@echo "complie common ..."
	make -f common.Makefile
	make -C swparameter
	make -C swutil

clean:
	make -f common.Makefile clean
	make -C swparameter clean
	make -C swutil clean

copy:
	make -f common.Makefile copy
	make -C swparameter copy
