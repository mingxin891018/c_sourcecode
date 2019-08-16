INDIR += \
-I../openssl-lib/include

DYN_LDS_WITH += \
-L../openssl-lib/lib/ -lcrypto

.PHONY:all clean
all:
	cc ssl_twoway_client.c $(INDIR) $(DYN_LDS_WITH) -o ssl_twoway_client -Wall -g -lssl -lcrypto 
	cc ssl_twoway_server.c $(INDIR) $(DYN_LDS_WITH) -o ssl_twoway_server -Wall -g -lssl -lcrypto 
	make -C client_CRT
	make -C server_CRT
	make -C CA

clean:
	rm -rf ssl_twoway_server ssl_twoway_client
	make -C client_CRT clean
	make -C server_CRT clean
	make -C CA clean
