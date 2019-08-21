INDIR += \
-I../../openssl-lib/include

DYN_LDS_WITH += \
-L../../openssl-lib/lib/ -lcrypto

.PHONY:all ecdsa_test
all:ecdsa_test

ecdsa_test:
	cc ecdsa_test.c $(INDIR) $(DYN_LDS_WITH) -lcrypto -o ecdsa_test -Wall

clean:
	rm -rf ecdsa_test public_key.txt private_key.txt sinature.txt




