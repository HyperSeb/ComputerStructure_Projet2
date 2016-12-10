#include <stdlib.h>
#include <stdio.h>
#include "gridHandler"

struct {
    long mtype;
    int offset;
} myMsg;

struct BestAndStop {
    int best; // offset of the best creature
    int stop; //stop is equal to 0 if the program can run, to 1 if the master and worker process must close
    // (not the listener) and to 2 if the programm must close
};

/* a process which goal is to listen to the user commands, to print the journey 
of the best creature so far and to close the progam
*/
void listenerProcess(Grid grid, int numberOfSlaves, int genomeLength);

/* a process which goal is to compute the score of a creature which index is 
given by the master process through a message queue
*/
void workerProcess(Grid grid, int genomeLength);

/* a process which goal is to handle the generation/classification/mutation of 
the creatures
*/
void masterProcess(int numberOfSlaves, int numberOfCreature, int deletionRate, int mutationRate, int genomeLength);
