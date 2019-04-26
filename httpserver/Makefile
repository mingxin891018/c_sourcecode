CFLAGS+=-lpthread -Wall
CFLAGS+=-lm
LDFLAGS=-I./

BIN=main
SRC_C=$(shell ls *.c)
SRC_H=$(shell ls *.h)
OBJ_C=$(patsubst %.c,%.o,$(SRC_C))

all:$(BIN) 

$(BIN):$(OBJ_C)
	$(CC) $^  -o $@  $(CFLAGS) 

%.o:%.c $(SRC_H)
	$(CC) -c $< -o $@ $(LDFLAGS)



clean:
	rm -rf $(OBJ_C) $(BIN)

