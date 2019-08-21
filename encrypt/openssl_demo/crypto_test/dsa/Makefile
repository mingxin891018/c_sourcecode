INDIR += \
-I../../openssl-lib/include

DYN_LDS_WITH += \
-L../../openssl-lib/lib/ -lcrypto

.PHONY:all dsa_test
all:dsa_test

dsa_test:
	cc dsa_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o dsa_test -Wall

clean:
	rm -rf dsa_test




