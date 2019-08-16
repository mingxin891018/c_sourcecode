INDIR += \
-I../openssl-lib/include

DYN_LDS_WITH += \
-L../openssl-lib/lib/ -lcrypto

.PHONY:all aes_test
all:aes_test rsa_test 3des_test des_test base64_test

aes_test:
	cc aes_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o aes_test -Wall

rsa_test:
	cc rsa_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o rsa_test -Wall

3des_test:
	cc 3des_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o 3des_test -Wall

des_test:
	cc des_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o des_test -Wall

base64_test:
	cc base64_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o base64_test -Wall

clean:
	rm -rf aes_test rsa_test rsa_test 3des_test des_test base64_test




