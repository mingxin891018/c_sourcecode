INDIR += \
-I../../openssl-lib/include

DYN_LDS_WITH += \
-L../../openssl-lib/lib/ -lcrypto

.PHONY:all
all:dh_test

dh_test:
	cc dh_test.c base64_test.c  server2.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o dh_test -Wall

clean:
	rm -rf dh_test




