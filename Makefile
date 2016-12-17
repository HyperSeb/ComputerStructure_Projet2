# =========================================================================
# Makefile for the Project 2
# =========================================================================
CC=gcc
CFLAGS=--std=c99 -W -Wall --pedantic -DNDEBUG

all: evolve

evolve: gridHandler.o creature.o PriorityQueue.o process.o main.o
	$(CC) -o evolve gridHandler.o PriorityQueue.o process.o main.o -lm

gridHandler.o: gridHandler.c gridHandler.h
	$(CC) -o gridHandler.o -c gridHandler.c $(CFLAGS)

creature.o: creature.c creature.h gridHandler.h move.txt
	$(CC) -o creature.o -c creature.c $(CFLAGS)

PriorityQueue.o: PriorityQueue.c PriorityQueue.h
	$(CC) -o PriorityQueue.o -c PriorityQueue.c $(CFLAGS)

process.o: process.c creature.h PriorityQueue.h gridHandler.h move.txt
	$(CC) -o process.o -c process.c $(CFLAGS)

main.o: main.c process.h PriorityQueue.h gridHandler.h
	$(CC) -o main.o -c main.c $(CFLAGS)

clean:
	rm -f *.o
