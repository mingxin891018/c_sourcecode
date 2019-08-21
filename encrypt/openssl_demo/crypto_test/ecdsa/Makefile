INDIR += \
-I./__install/include

DYN_LDS_WITH += \
-L./__install/lib/ -lcrypto

.PHONY:all ecdsa_test
all:ecdsa_test

ecdsa_test:
	cc ecdsa_test.c $(INDIR) $(DYN_LDS_WITH) -o ecdsa_test -Wall

clean:
	rm -rf ecdsa_test public_key.txt private_key.txt sinature.txt




