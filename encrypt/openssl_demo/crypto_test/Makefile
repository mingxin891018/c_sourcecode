INDIR += \
-I../openssl-lib/include

DYN_LDS_WITH += \
-L../openssl-lib/lib/ -lcrypto

.PHONY:all aes_test rsa_test 3des_test des_test base64_test hmac_test md5_test_string ecdsa
all:aes_test rsa_test 3des_test des_test base64_test hmac_test md5_test_string ecdsa

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

hmac_test:
	cc hmac_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o hmac_test -Wall

md5_test_string:
	cc md5_test_string.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o md5_test_string -Wall

ecdsa:
	make -C ecdsa

dh_test:
	cc dh_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o dh_test -Wall

ecdh_test:
	cc ecdh_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o ecdh_test -Wall

ecdsa_test:
	cc ecdsa_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o ecdsa_test -Wall

dsa_test:
	cc dsa_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o dsa_test -Wall

clean:
	rm -rf aes_test rsa_test rsa_test 3des_test des_test base64_test hmac_test md5_test_string dh_test ecdh_test ecdsa_test dsa_test
	make -C ecdsa clean




