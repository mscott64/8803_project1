CC=gcc
CFLAGS=-I. -I.. -lpthread -Wall
DEPS = server.h constants.h
OBJ = server.o

all: server

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean: 
	rm -f *.o server