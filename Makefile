CC=gcc
CFLAGS=-I. -lpthread
DEPS = server.h client.h
OBJ = server.o client.o main.o

all: main

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean: 
	rm -f *.o main