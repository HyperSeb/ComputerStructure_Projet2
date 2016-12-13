#ifndef process_h
#define process_h

#include <stdlib.h>
#include <stdio.h>
#include "gridHandler.h"

struct {
    long mtype;
    int offset;
} myMsg;

struct BestAndStop {
    int best; // offset of the best creature
    int stop; //stop is equal to 0 if the program can run, to 1 if the master and worker process must close
    // (not the listener) and to 2 if the programm must close
};
typedef struct BestAndStop BestAndStop;

struct Genomes {
    int* storage;
    int numberOfCreatures;
    int genomeLength;
};
typedef struct Genomes Genomes;

/* return a pointer to the genome of a the creature
*/
int* genomeAtIndex(Genomes genomes, int index);

/* a process which goal is to listen to the user commands, to print the journey 
of the best creature so far and to close the progam
*/
void listenerProcess(Grid grid, Genomes genomes, int numberOfSlaves, int qId, int semId, BestAndStop * sharedStruct);

/* a process which goal is to compute the score of a creature which index is 
given by the master process through a message queue
*/
void workerProcess(Grid grid, Genomes genomes, double* scores, int qId, int semId, BestAndStop * sharedStruct);

/* a process which goal is to handle the generation/classification/mutation of 
the creatures
*/
void masterProcess(int numberOfSlaves, int deletionRate, int mutationRate, Genomes genomes, double* scores
                    , int qId, int semId, BestAndStop * sharedStruct);

#endif /*process_h*/
