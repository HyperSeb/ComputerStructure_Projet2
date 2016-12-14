# =========================================================================
# Makefile for the Project 2
# =========================================================================
CC=gcc
CFLAGS=--std=c99 -W -Wall --pedantic -DNDEBUG

all: evolve

evolve: gridHandler.o PriorityQueue.o process.o main.o
	$(CC) -o evolve gridHandler.o PriorityQueue.o process.o main.o -lm

gridHandler.o: gridHandler.c
	$(CC) -o gridHandler.o -c gridHandler.c $(CFLAGS)

PriorityQueue.o: PriorityQueue.c
	$(CC) -o PriorityQueue.o -c PriorityQueue.c $(CFLAGS)

process.o: process.c PriorityQueue.h gridHandler.h move.txt
	$(CC) -o process.o -c process.c $(CFLAGS)

main.o: main.c process.h PriorityQueue.h gridHandler.h
	$(CC) -o main.o -c main.c $(CFLAGS)

clean:
	rm -f *.o
