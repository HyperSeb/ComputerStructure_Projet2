#include<stdlib.h>
#include<stdio.h>

struct{
	long mtype;
	int offset;
} myMsg;

struct bestBegEnd {
	int best; // offset of the best creature
	size_t begin; // offset of the begin, we only need 1 value since we use a 1D table to represent a matrix
	size_t end; // offset of the end
	int stop; //stop is equal to 0 if the program can run, to 1 if the master and worker process must close
	// (not the listener) and to 2 if the programm must close
};

/* a process which goal is to listen to the user commands, to print the journey 
of the best creature so far and to close the progam
*/
void listenerProcess(int M, int N, int P, int T);

/* a process which goal is to compute the score of a creature which index is 
given by the master process through a message queue
*/
void workerProcess(int M, int N, int T);

/* a process which goal is to handle the generation/classification/mutation of 
the creatures
*/
void masterProcess(int P, int C, int p, int m, int T);
