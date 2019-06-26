all:
	gcc -o ssl_twoway_client ssl_twoway_client.c -Wall -g -lssl -lcrypto -lpthread
	gcc -o ssl_twoway_server ssl_twoway_server.c -Wall -g -lssl -lcrypto -lpthread


clean:
	rm -rf ssl_oneway_server ssl_oneway_client ssl_client ssl_server ssl_twoway_client ssl_twoway_server

