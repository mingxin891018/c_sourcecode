INDIR += \
-I../openssl-lib/include

DYN_LDS_WITH += \
-L../openssl-lib/lib/ -lcrypto

.PHONY:all aes_test
all:aes_test rsa_test

aes_test:
	cc aes_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o aes_test -Wall

rsa_test:
	cc rsa_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o rsa_test -Wall


clean:
	rm -rf aes_test rsa_test rsa_private.pem rsa_public.pem




