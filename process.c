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
#include "gridHandler.h" //in order to acces the "ind" function
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

static void sendMessage(int qId, myMsg* msg){
	if(msgsnd(qId,msg, sizeof(myMsg) - sizeof(long), 0) < 0){
		fprintf(stderr, "sending a message failed\n");
		exit(EXIT_FAILURE);
	}
}

static void readMessage(int qId, long type, int* offset){
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


static bool moreThanHalf(int half, int full) {
    if (full == 0) {
        return true;
    } else if (full > 0){
        return 2 * half >= full;
    } else {
        return 2 * half < full;
    }
}

// assuming deltaX = -1, 0 or 1, else undefined behaviour may appear
static Position computeResultOfMove(Grid grid, Position from, int deltaX, int deltaY) {
    int verticalDirection = deltaY < 0 ? -1 : 1;
    
    if (deltaX == 0) {
        // vertical move, go forward until end or obstacle
        for (int dy = 0; dy * verticalDirection < deltaY * verticalDirection; dy += verticalDirection) {
            Position further = {from.x, from.y + dy + verticalDirection};
            
            if (getInGrid(grid, further) == obstacle) {
                Position current = {from.x, from.y + dy};
                return current;
            }
        }
        
        Position current = {from.x, from.y + deltaY};
        return current;
        
    } else if (deltaY == 0) {
        // horizontal move, the destination is either an obstacle or not
        Position further = {from.x + deltaX, from.y};
        if (getInGrid(grid, further) == obstacle) {
            return from;
        } else {
            return further;
        }
    } else {
        // diagonal move, go vertically forward,
        // if there is an obstacle in the column or in the column in the direction of deltaX, a special comportement is performed
        for (int dy = 0; dy * verticalDirection < deltaY * verticalDirection; dy += verticalDirection) {
            Position current = {from.x + (moreThanHalf(dy, deltaY) ? deltaX : 0), from.y + dy};
            
            Position further = {from.x, from.y + dy + verticalDirection},
            nextToFurther = {from.x + deltaX, from.y + dy + verticalDirection};
            
            if (getInGrid(grid, further) == obstacle && getInGrid(grid, nextToFurther) == obstacle) {
                // both are obstacle, don't go further
                return current;
                
            } else if (getInGrid(grid, further) == obstacle) {
                // the one in the column we start the move is an obstacle,
                // depending on how far we are in the move, we land on the obstacle, or continue
                if (moreThanHalf(dy + verticalDirection, deltaY)) {
                    continue;
                } else {
                    return current;
                }
                
            } else if (getInGrid(grid, nextToFurther) == obstacle) {
                // the one in the column we are going is an obstacle,
                // depending on how far we are in the move, we land on it, or fall vertically
                if (moreThanHalf(dy + verticalDirection, deltaY)) {
                    if (moreThanHalf(dy, deltaY)) {
                        return current;
                    } else {
                        return computeResultOfMove(grid, current, 0, deltaY - dy); // the rest of the move is vertical
                    }
                } else {
                    return computeResultOfMove(grid, current, 0, deltaY - dy); // the rest of the move is vertical
                }
                
            } else {
                // no obstacle, we continue
                continue;
            }
        }
        
        // no problematic obstacle were met, the move is completely done
        Position current = {from.x + deltaX, from.y + deltaY};
        return current;
    }
}

// return the final position of the creature
static Position performCreature(Grid grid, int* genome, int genomeLength, bool displayingSteps) {
    Position currentPosition = grid.start;

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
            currentPosition = nextPosition;
            
            if (displayingSteps) {
                printf("\n\n\n\n\n\n");
                displayGrid(grid, currentPosition, genome, genomeLength, geneIndex);
                sleep(1);
            }
            
            deltaY++;
            Position tmp = {currentPosition.x, currentPosition.y - 1};
            underPosition = tmp;
        } while (getInGrid(grid, underPosition) != obstacle);
    }
    
    if (displayingSteps) {
        printf("Voilà\n");
    }
    return currentPosition;
}
// show the best creature's movements on the terminal
static void showBest(Grid grid, int* genome, int genomeLength){
	performCreature(grid, genome, genomeLength, true);
}

void listenerProcess(Grid grid, Genomes genomes, int numberOfSlaves, int qId, int semId, BestAndStop * sharedStruct){
	printf("type: G to request a new generation\n");
	printf("      M followed by a number to request that number of generations\n");
	printf("      B to display the best creature so far\n");
	printf("      Q to close the program\n");
	while(sharedStruct->stop != 2){
		char tmp = 'a';
		unsigned int number = 0;
		scanf(" %c", &tmp);
		switch (tmp) {
		// we do the signal(s) if the user typed 'G' or 'M' even if Offsets->stop == 1 
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
			if (sharedStruct->stop == 0){ // we have to close workers and master process
				sharedStruct->stop = 2;
				// all the workers processes will now close as soon as they get a message
				for(size_t i = 0; i < numberOfSlaves; ++i){
					myMsg msg;
					msg.type = 1; // the offset doesn't matter
					sendMessage(qId, &msg);
				}
				// the master can be waiting for a new generation or a message
				signal(semId, 1, 1); // we tell him to stop to wait (and to close)
				myMsg msg; // we send a close message to the master
				msg.type = 2;
				msg.offset = -1;
				sendMessage(qId, &msg);
			} else {
				sharedStruct->stop = 2;
			}
			break;
		default: 
			printf("invalid command \n");
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

// computes the score of the creature
static double computeScore(Grid grid, int* genome, int genomeLength) {
	// c'est pas cette fonction qui gère le cas où on aurait un bestScore == 0
	// ne gere pas non plus le cas ou on changerait le meilleur
    Position finalPosition = performCreature(grid, genome, genomeLength, false);
    
    int dX = finalPosition.x - grid.finish.x;
    int dY = finalPosition.y - grid.finish.y;
    
    return sqrt(dX * dX + dY * dY);
}
	
void workerProcess(Grid grid, Genomes genomes, double* scores, int qId, int semId, BestAndStop * sharedStruct){
	int offset;
	while(sharedStruct->stop == 0){
		readMessage(qId, 1, &offset);
		if (sharedStruct->stop != 0){
			break;
		}
		scores[offset] = computeScore(grid, genomeAtIndex(genomes, offset), genomes.genomeLength);
		
		wait(semId, 0); // we may modify the best creature's offset
		if(scores[sharedStruct->best] > scores[offset] || sharedStruct->best == -1){
			sharedStruct->best = offset;
		}
		signal(semId, 0, 1);
		myMsg msg; // tells the master the math is done
		msg.type = 2;
		msg.offset = offset;
		sendMessage(qId, &msg);
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
				newGene = rand()%8;
			} while(genome[j] == newGene);
			genome[j] = newGene;
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
// -1 the masterprocess has to stop because the user pressed 'Q' or a perfect creature was detected
static int fillHeapWithWorkersResults(MaxHeap* heap, double* scores, int numberOfSlaves, int qId, BestAndStop* sharedStruct) {
	while (!isFull(heap)) {
		int offset;
		readMessage(qId, 2, &offset);
		if(offset == -1){
			return -1;
		}
		if(scores[offset] == 0.0){
			sharedStruct->stop = 1;
			for(size_t j = 0; j < numberOfSlaves; ++j){ // fake messages to be sure no worker 
				// is waiting for a message
				myMsg msg;
				msg.type = 1; // the offset doesn't matter
				sendMessage(qId, &msg);
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
	if(sharedStruct->stop == 2){
		destroyMaxHeap(heap);
		signal(semId, 2, 1);
		exit(EXIT_SUCCESS);
	}
	for(int i = 0; i < genomes.numberOfCreatures; ++i){
		createCreature(genomeAtIndex(genomes, i), genomes.genomeLength);
		myMsg msg = {/*type*/ 1, /*offset*/ i}; // we send a message to a worker
		sendMessage(qId, &msg);
	}
	if (fillHeapWithWorkersResults(heap, scores, numberOfSlaves, qId, sharedStruct) != 0) {
		destroyMaxHeap(heap);
		signal(semId, 2, 1);
		exit(EXIT_SUCCESS);
	}
	
	// other generations
	int numberToReplace = (genomes.numberOfCreatures * deletionRate) / 100;
	
	while(sharedStruct->stop != 2){
		wait(semId, 1);
		if(sharedStruct->stop == 2){
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
			
			myMsg msg = {1, currentIndex}; // we send a message to a worker
			sendMessage(qId, &msg);
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
