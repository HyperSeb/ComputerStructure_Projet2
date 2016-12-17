# =========================================================================
# Makefile for the Project 2
# =========================================================================
CC=gcc
CFLAGS=--std=c99 -W -Wall --pedantic -DNDEBUG
LDFLAGS=-lm
all: evolve

evolve: gridHandler.o creature.o PriorityQueue.o process.o main.o
	$(CC) -o $@ $^ $(LDFLAGS)

gridHandler.o: gridHandler.c gridHandler.h
	$(CC) -o $@ -c $< $(CFLAGS)

creature.o: creature.c creature.h gridHandler.h move.txt
	$(CC) -o $@ -c $< $(CFLAGS)

PriorityQueue.o: PriorityQueue.c PriorityQueue.h
	$(CC) -o $@ -c $< $(CFLAGS)

process.o: process.c creature.h PriorityQueue.h gridHandler.h move.txt
	$(CC) -o $@ -c $< $(CFLAGS)

main.o: main.c process.h PriorityQueue.h gridHandler.h
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f *.o
