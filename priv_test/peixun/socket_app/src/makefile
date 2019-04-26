CFLAGS+=-lpthread -Wall
LDFLAGS=-I./

SERVER=server 
SERVER_O=server.o share.o 

CLIENT=client
CLIENT_O=client.o share.o

SRC=$(SERVER) $(CLIENT)
SRC_C=$(shell ls *.c)
SRC_H=$(shell ls *.h)
OBJ_C=$(patsubst %.c,%.o,$(SRC_C))

all:$(SRC) 

$(SERVER):$(SERVER_O)
	$(CC) $^  -o $@ $(CFLAGS) 
$(CLIENT):$(CLIENT_O)
	$(CC) $^  -o $@  $(CFLAGS) 

%.o:%.c $(SRC_H)
	$(CC) -c $< -o $@ $(LDFLAGS)



clean:
	rm -rf $(OBJ_C) $(SRC)  













