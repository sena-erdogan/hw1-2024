CC = gcc
CFLAGS = -Wall

all: hw1

hw1: hw1.c
	$(CC) $(CFLAGS) hw1.c -o hw1

clean:
	rm -f hw1
