#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include "process.h"
#include "gridHandler.h" //in order to acces the "ind" function
#include "PriorityQueue.h"

static void wait(int offset){ // we create this function for readability
	struct sembuf buf = { offset, -1, 0};
	if((semop(semId, &buf, 1)) == -1){
		fprintf(stderr, "Wait failed\n");
		exit(EXIT_FAILURE);
	}
}

static void signal(int offset, int value){ // we will need to signal more than once sometimes
	struct sembuf buf = { offset, value,0};
	if((semop(semId, &buf, 1)) == -1){
		fprintf(stderr, "Signal failed\n");
		exit(EXIT_FAILURE);
	}
}

static void sendMessage(myMsg* msg){
	if(msgsnd(qId,msg, sizeof(myMsg) - sizeof(long), 0) < 0){
		fprintf(stderr, "sending a message failed\n");
		exit(EXIT_FAILURE);
	}
}

static void readMessage(long type, int* offset){
	if(msgrcv(qId, offset, sizeof(myMsg) - sizeof(long), type,  0) < 0){
		fprintf(stderr, "reading a message failed\n");
		exit(EXIT_FAILURE);
	}
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

static Position computeResultOfMove(Grid grid, Position from, int deltaX, int deltaY) {
    // Magic should appear here
    
    // Naive (waiting for magic)
    Position nextPosition = {from.x + deltaX, from.y + deltaY};
    
    if (getInGrid(grid, nextPosition) == obstacle) {
        nextPosition = {from.x, from.y + deltaY};
        if (getInGrid(grid, nextPosition) == obstacle) {
            return from;
        }
    }
    
    return nextPosition;
}

// return the final position of the creature
static Position performCreature(Grid grid, int* genome, int genomeLength, bool displayingSteps) {
    Position currentPosition = grid.start;

    if (displayingSteps) {
        // display start
        displayGrid(grid, grid.start, genome, genomeLength, -1);
    }

    for (int geneIndex = -1; geneIndex < genomeLength; geneIndex++) {
        int direction = geneIndex == -1 ? -1 : genome[geneIndex];
        int deltaX, deltaY;
        
        switch (direction) {
                
#define MOVE(dX, dY, intValue, character) \
            case intValue:\
                deltaX = dX;\
                deltaY = dY;\
                break;
#include "move.txt"
                
            default:
                deltaX = 0;
                deltaY = 0;
                break;
        }
        Position underPosition;
        do {
            Position nextPosition = computeResultOfMove(grid, currentPosition, deltaX, deltaY);
            if (nextPosition.x == currentPosition.x && nextPosition.y == currentPosition.y) {
                // do we display this one?
            } else {
                currentPosition = nextPosition;
                
                if (displayingSteps) {
                    sleep(1);
                    displayGrid(grid, currentPosition, genome, genomeLength, geneIndex);
                }
            }
            
            deltaY++;
            underPosition = {currentPosition.x, currentPosition.y - 1};

        } while (getInGrid(grid, underPosition) != obstacle)
    }
    
    return currentPosition;
}
// show the best creature's movements on the terminal
static void showBest(Grid grid, int* genome, int genomeLength){
	performCreature(grid, genome, genomeLength, true);
}

void listenerProcess(Grid grid, Genomes genomes, int numberOfSlaves){
	printf("type: G to request a new generation\n");
	printf("      M followed by a number to request that number of generations\n");
	printf("      B to display the best creature so far\n");
	printf("      Q to close the program\n");
	while(Offsets->stop != 2){
		char tmp = 'a';
		unsigned int number = 0;
		scanf(%c, &tmp);
		switch (tmp) {
		// we do the signal(s) if the user typed 'G' or 'M' even if Offsets->stop == 1 
		// since a signal is an atomical operation and it won't generate errors (it's just useless)
		case 'G' : 
			signal(1,1);
			break;
		case 'M' :
			if(scanf(%ud, &number) != -1){
				signal(1,number);
			} else {
				printf("your should type a number after 'M'\n");
			}
			break;
		case 'B' :
			wait(0);
			best = sharedStruct->best;
			if(best == -1){
				signal(0,1);
				printf("I'm sorry Dave, I'm afraid I can't do that\n");
			} else {
				// we copy the table in order not to block the master process during displaying
				int bestCreature[genomes.genomeLength];
				copyGenome(genomeAtIndex(genomes, best), bestCreature, genomes.genomeLength);
				signal(0,1);
				showBest(grid, bestCreature, genomeLength);
			}
			break;
		case 'Q' :
			if (sharedStruct->stop == 0){ // we have to close workers and master process
				sharedStruct->stop = 2;
				// all the workers processes will now close as soon as they get a message
				for(size_t i = 0; i < numberOfSlaves; ++i){
					myMsg msg;
					msg.type = 1; // the offset doesn't matter
					sendMessage(&msg);
				}
				// the master can be waiting for a new generation or a message
				signal(1,1); // we tell him to stop to wait (and to close)
				myMsg msg; // we send a close message to the master
				msg.type = 2;
				msg.offset = -1;
				sendMessage(&msg);
			} else {
				sharedStruct->stop = 2;
			}
			break;
		default: 
			printf("invalid command \n");
		}
	}
				
	for(int i = 0; i < numberOfSlaves+1; ++i){  // we wait untill master + all worker processes are closed
		wait(2);
	}
	// delete the semaphore/message queue, the shared memory has already been flagged for deletion
	semctl(semId,0,IPC_RMID,0));
	msgctl(qId, IPC_RMID, 0);
	exit(EXIT_SUCCES);
}

// computes the score of the creature
static double computeScore(Grid grid, int* genome, int genomeLength) {
	// c'est pas cette fonction qui gère le cas où on aurait un bestScore == 0
	// ne gere pas non plus le cas ou on changerait le meilleur
    Position finalPosition = performCreature(grid, genome, genomeLength, false);
    
    int dX = finalPosition.x - grid.finish.x;
    int dY = finalPosition.y - grid.finish.y;
    
    return sqrt(dX * dX + dY * dY);
}
	
void workerProcess(Grid grid, Genomes genomes, double* scores){
	int offset;
	while(sharedStruct->stop == 0){
		readMessage(1, &offset);
		if (sharedStruct->stop != 0){
			break;
		}
		scores[offset] = computeScore(grid, genomeAtIndex(genomes, offset), genomeLength);
		
		wait(0); // we may modify the best creature's offset
		if(scores[sharedStruct->best] > scores[offset]){
			sharedStruct->best = offset;
		}
		signal(0,1);
		myMsg msg; // tells the master the math is done
		msg.type = 2;
		msg.offset = offset;
		sendMessage(&msg);
	}
	signal(2,1); // signals we closed
	exit(EXIT_SUCCESS);
}

// modifies the genome
static void modifyCreature(int mutationRate, int* goodGenome, int* badGenome, int genomeLength){
	copyGenome(goodGenome, badGenome, genomeLength)
	for(int j = 0; j < genomeLength; ++j){
		if((rand() % 100) < mutationRate){ // if the move mutates
			int newGenome;
			do {
				newGenome = rand()%8;
			} while(genome[j] == newGenome)
			genome[j] = newGenome;
		}
	}
}

// creates a new genome
static void createCreature(int* genome, int genomeLength){
	for(int j = 0; j < genomeLength; ++j){
		genome[j] = rand()%8;
	}
}

//  0 fine, the heap is full
// -1 offset == -1
// -2 best manages to finish
static int fillHeapWithWorkersResults(MaxHeap* heap, double* scores, int numberOfSlaves) {
	while (!isFull(heap)) {
		int offset;
		readMessage(2, &offset);
		if(offset == -1){
			return -1;
		}
		if(scores[offset] == 0.0){
			sharedStruct->stop = 1;
			for(size_t j = 0; j < numberOfSlaves; ++j){ // fake messages to be sure no worker 
				// is waiting for a message
				myMsg msg;
				msg.type = 1; // the offset doesn't matter
				sendMessage(&msg);
			}
			printf("One of the Creatures was able to reach the goal tile\n");
			printf("All you can do now is watch his journey (B) or quit (Q)\n");
			return -2;
		}
		insertIndex(offset, heap, scores); // we insert the index in the heap
	}
	return 0;
}

void masterProcess(int numberOfSlaves, int deletionRate, int mutationRate, Genomes genomes, double* scores){
	MaxHeap* heap = createMaxHeap((size_t) genomes.numberOfCreatures);
	
	// first generation
	wait(1);
	if(sharedStruct->stop == 2){
		destroyMaxHeap(heap);
		signal(2,1);
		return;
	}
	for(int i = 0; i < genomes.numberOfCreature; ++i){
		createCreature(genomeAtIndex(genomes, i), genomes.genomeLength);
		myMsg msg = {/*type*/ 1, /*offset*/ i}; // we send a message to a worker
		sendMessage(&msg);
	}
	if (fillHeapWithWorkersResults(heap, scores, numberOfSlaves) != 0) {
		destroyMaxHeap(heap);
		signal(2,1);
		return;
	}
	
	// other generations
	int beginBadCreatures = genomes.numberOfCreatures * (100-deletionRate) / 100,
	endGoodCreatures = genomes.numberOfCreatures * deletionRate / 100;
	while(sharedStruct->stop != 2){
		wait(1);
		if(sharedStruct->stop == 2){
			break;
		}
		
		// sort the heap untill we know the best creatures
		for(int i = genomes.numberOfCreatures-1; i >= endGoodCreatures; --i){
			extractIndexForMax(heap, scores);
		}
		// creates the new creatures
		for(int i = 0; i < endGoodCreatures; ++i){
			modifyCreature(mutationRate, genomeAtIndex(genomes, getTableElement(i)), 
				genomeAtIndex(genomes, getTableElement(beginBadCreatures + i)), genomeLength);
			myMsg msg = {1, getTableElement(beginBadCreatures + i)}; // we send a message to a worker
			sendMessage(&msg);
		}
		
		if (fillHeapWithWorkersResults(heap, scores, numberOfSlaves) == -2) {
			destroyMaxHeap(heap);
			signal(2,1); // we have to signal we closed
			return;
		}
	}
	destroyMaxHeap(heap);
	signal(2,1);
	return;
}
