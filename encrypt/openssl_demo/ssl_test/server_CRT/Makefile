INDIR += \
-I../../openssl-lib/include

DYN_LDS_WITH += \
-L../../openssl-lib/lib/ -lcrypto

TARGET=cert_test
.PHONY:all clean
all:
	gcc cert_test.c $(INDIR) $(DYN_LDS_WITH) -o $(TARGET) -g -lssl -lcrypto -Wall


clean:
	rm -rf $(TARGET) *.crt *.key


