#define _SVID_SOURCE

#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include <math.h>
#include "process.h"
#include "gridHandler.h"
#include "creature.h"
#include "PriorityQueue.h"

static void wait(int semId, int offset){ // we create this function for readability
	struct sembuf buf = { offset, -1, 0};
	if((semop(semId, &buf, 1)) == -1){
		fprintf(stderr, "Wait failed\n");
		exit(EXIT_FAILURE);
	}
}

static void signal(int semId, int offset, int value){ // we will need to signal more than once sometimes
	struct sembuf buf = { offset, value,0};
	if((semop(semId, &buf, 1)) == -1){
		fprintf(stderr, "Signal failed\n");
		exit(EXIT_FAILURE);
	}
}

static void sendMessage(int qId, long type, int offset){
	myMsg msg;
	msg.type = type;
	msg.offset = offset;
	if(msgsnd(qId, &msg, sizeof(myMsg) - sizeof(long), 0) < 0){
		fprintf(stderr, "sending a message failed\n");
		exit(EXIT_FAILURE);
	}
}

static int readMessage(int qId, long type){
	myMsg temp;
	if(msgrcv(qId, &temp, sizeof(myMsg) - sizeof(long), type,  0) < 0){
		fprintf(stderr, "reading a message failed\n");
		exit(EXIT_FAILURE);
	}
	return temp.offset;
}

int* genomeAtIndex(Genomes genomes, int index) {
	if (0 <= index && index < genomes.numberOfCreatures) {
		return genomes.storage + index * genomes.genomeLength;
	} else {
		fprintf(stderr, "genomeAtIndex accessed out of the bounds");
		return NULL;
	}
}

static void copyGenome(int* from, int* to, int genomeLength) {
	for (int i = 0; i < genomeLength; i++) {
		to[i] = from[i];
	}
}

void listenerProcess(Grid grid, Genomes genomes, int numberOfSlaves, int qId, int semId, BestAndStop * sharedStruct){
	bool stopAll = false;
	printf("type: G to request a new generation\n");
	printf("      M followed by a number to request that number of generations\n");
	printf("      B to display the best creature so far\n");
	printf("      Q to close the program\n");
	while(true){
		char tmp = 'a';
		unsigned int number = 0;
		scanf(" %c", &tmp);
		switch (tmp) {
		// we do the signal(s) if the user typed 'G' or 'M' even if Offsets->stop == true 
		// since a signal is an atomical operation and it won't generate errors (it's just useless)
		case 'G' : 
			signal(semId, 1, 1);
			break;
		case 'M' :
			if(scanf("%ud", &number) != -1){
				signal(semId,1,number);
			} else {
				printf("your should type a number after 'M'\n");
			}
			break;
		case 'B' :
			wait(semId, 0);
			int best = sharedStruct->best;
			if(best == -1){
				signal(semId, 0, 1);
				printf("I'm sorry Dave, I'm afraid I can't do that\n");
			} else {
				// we copy the table in order not to block the master process during displaying
				int bestCreature[genomes.genomeLength];
				copyGenome(genomeAtIndex(genomes, best), bestCreature, genomes.genomeLength);
				signal(semId, 0, 1);
				showBest(grid, bestCreature, genomes.genomeLength);
			}
			break;
		case 'Q' :
			if (!(sharedStruct->stop)){ // we have to close workers and master process
				sharedStruct->stop = true;
				// all the workers processes will now close as soon as they get a message
				for(int i = 0; i < numberOfSlaves; ++i){
					sendMessage(qId, 1, 1); // the offset doesn't matter
				}
				// the master can be waiting for a new generation or a message
				signal(semId, 1, 1); // we tell him to stop to wait (and to close)
				sendMessage(qId, 2, -1); // we send a close message to the master
			}
			stopAll = true;
			break;
		default: 
			printf("invalid command \n");
		}
		if (stopAll){
			break;
		}
	}
				
	for(int i = 0; i < numberOfSlaves+1; ++i){  // we wait untill master + all worker processes are closed
		wait(semId, 2);
	}
	// delete the semaphore/message queue, the shared memory has already been flagged for deletion
	semctl(semId,0,IPC_RMID,0);
	msgctl(qId, IPC_RMID, 0);
	return;
}
	
void workerProcess(Grid grid, Genomes genomes, double* scores, int qId, int semId, BestAndStop * sharedStruct){
	int offset;
	while(!(sharedStruct->stop)){
		offset = readMessage(qId, 1);
		if (sharedStruct->stop){
			break;
		}
		scores[offset] = computeScore(grid, genomeAtIndex(genomes, offset), genomes.genomeLength);
		
		wait(semId, 0); // we may modify the best creature's offset
		if(scores[sharedStruct->best] > scores[offset] || sharedStruct->best == -1){
			sharedStruct->best = offset;
		}
		signal(semId, 0, 1);
		sendMessage(qId, 2, offset); // tells the master the math is done
	}
	signal(semId, 2, 1); // signals we closed
	exit(EXIT_SUCCESS);
}

// modifies the genome
static void modifyCreature(int mutationRate, int* genome, int genomeLength){
	for(int j = 0; j < genomeLength; ++j){
		if((rand() % 100) < mutationRate){ // if the move mutates
			int newGene;
			do {
				newGene = randomGene();
			} while(genome[j] == newGene);
			genome[j] = newGene;
		}
	}
}

// creates a new genome
static void createCreature(int* genome, int genomeLength){
	for(int j = 0; j < genomeLength; ++j){
		genome[j] = randomGene();
	}
}

/* insert the index of a creature in the heap now that its corresponding score is computed
returns 0 if it is fine or if the heap is full and -1 if the masterprocess has to stop because 
the user pressed 'Q' or a perfect creature was detected */
static int fillHeapWithWorkersResults(MaxHeap* heap, double* scores, int numberOfSlaves, int qId, BestAndStop* sharedStruct) {
	while (!isFull(heap)) {
		int offset = readMessage(qId, 2);
		if(offset == -1){
			return -1;
		}
		if(scores[offset] == 0.0){
			sharedStruct->stop = true;
			for(int j = 0; j < numberOfSlaves; ++j){ // fake messages to be sure no worker 
				// is waiting for a message
				sendMessage(qId, 1, 1); // the offset doesn't matter
			}
			printf("One of the Creatures was able to reach the goal tile\n");
			printf("All you can do now is watch his journey (B) or quit (Q)\n");
			return -1;
		}
		insertIndex(offset, heap, scores); // we insert the index in the heap
	}
	return 0;
}

void masterProcess(int numberOfSlaves, int deletionRate, int mutationRate, Genomes genomes, double* scores,
 			int qId, int semId, BestAndStop * sharedStruct){
	MaxHeap* heap = createMaxHeap((size_t) genomes.numberOfCreatures);
	
	// first generation
	wait(semId, 1);
	if(sharedStruct->stop){
		destroyMaxHeap(heap);
		signal(semId, 2, 1);
		exit(EXIT_SUCCESS);
	}
	for(int i = 0; i < genomes.numberOfCreatures; ++i){
		createCreature(genomeAtIndex(genomes, i), genomes.genomeLength);
		sendMessage(qId, 1, i);// we send a message to a worker
	}
	if (fillHeapWithWorkersResults(heap, scores, numberOfSlaves, qId, sharedStruct) != 0) {
		destroyMaxHeap(heap);
		signal(semId, 2, 1);
		exit(EXIT_SUCCESS);
	}
	printf("generation n°1 is complete\n");
	
	// other generations
	int numberToReplace = (genomes.numberOfCreatures * deletionRate) / 100;
	
	for(int genNumbr = 1; !(sharedStruct->stop); printf("generation n°%d is complete\n", ++genNumbr)){
		wait(semId, 1);
		if(sharedStruct->stop){
			break;
		}
		
		// sort the heap untill we know the best creatures
		for(int i = 0; i < numberToReplace; ++i){
			extractIndexForMax(heap, scores);
		}
		// creates the new creatures
		for(int i = 0; i < numberToReplace; ++i){
			int currentIndex = (heap->indices)[heap->count + i],
			motherIndex = (heap->indices)[rand() % heap->count];
			
			copyGenome(genomeAtIndex(genomes, motherIndex), genomeAtIndex(genomes, currentIndex), genomes.genomeLength);
			modifyCreature(mutationRate, genomeAtIndex(genomes, currentIndex), genomes.genomeLength);
			
			sendMessage(qId, 1, currentIndex);// we send a message to a worker
		}
		
		if (fillHeapWithWorkersResults(heap, scores, numberOfSlaves, qId, sharedStruct) != 0) {
			destroyMaxHeap(heap);
			signal(semId, 2,1); // we have to signal we closed
			exit(EXIT_SUCCESS);
		}
	}
	destroyMaxHeap(heap);
	signal(semId, 2, 1);
	exit(EXIT_SUCCESS);
}
