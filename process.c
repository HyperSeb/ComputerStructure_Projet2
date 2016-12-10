#include<stddef.h>
#include<stdbool.h>
#include<time.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/msg.h>
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

// show the best creature's movements on the terminal
static void showBest(int M, int N, int* bestCreature, int T){
	
}

void listenerProcess(Grid grid, int numberOfSlaves, int genomeLength){
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
				printf("I'm sorry Dave, I'm afraid I can't do that\n");
				signal(0,1);
			} else {
				// we copy the table in order not to block the master process during T seconds
				int bestCreature[genomeLength];
				for(int j = 0; j < genomeLength; ++j){
					bestCreature[j] = TableGenes[ind(best,j,T)];
				}
				signal(0,1);
				showBest(M, N, bestCreature, T);
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
static void computeScore(Grid grid, int genomeLength, int offset){
	// c'est pas cette fonction qui gère le cas où on aurait un bestScore == 0
	// ne gere pas non plus le cas ou on changerait le meilleur
}
	
static int ind(i,j,width){
	return width*i + j;
} 
void workerProcess(Grid grid, int genomeLength){
	int offset;
	while(stop == 0){
		readMessage(1, &offset);
		if (sharedStruct->stop != 0){
			break;
		}
		computeScore(grid, genomeLength, offset);
		wait(0); // we may modify the best creature's offset
		if(TableScores[sharedStruct->best] > TableScores[offset]){
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

// modifies the genes of the creature number index
static void modifyCreature(int index, int mutationRate, int genomeLength){
	for(int j = 0; j < genomeLength; ++j){
		if((rand%99) < mutationRate){ // if the move mutates
			int prev = tableGenes[ind(index,j,genomeLength)];
			int new = rand()%8;
			while(prev == new){
				new = rand%8;
			}
			tableGenes[ind(index,j,genomeLength)] = new;
		}
	}
}

// creates a nex creature at the index
static void createCreature(int index, int genomeLength){
	for(int j = 0; j < T; ++j){
		tableGenes[ind(index,j,genomeLength)] = rand()%8;
	}
}

void masterProcess(int numberOfSlaves, int numberOfCreature, int deletionRate, int mutationRate, int genomeLength){
	MaxHeap* heap = createMaxHeap((size_t) numberOfCreature);
	
	// first generation
	wait(1);
	if(sharedStruct->stop == 2){
		destroyMaxHeap(heap);
		signal(2,1);
		return;
	}
	for(int i = 0; i < numberOfCreature; ++i){
			createCreature(i, genomeLength);
			myMsg msg; // we send a message to a worker
			msg.type = 1;
			msg.offset = i;
			sendMessage(&msg);
	}
	for(int i = 0; i < numberOfCreature; ++i){
		int offset;
		readMessage(2, &offset);
		if(offset == -1){
			destroyMaxHeap(heap);
			signal(2,1);
			return;
		}
		if(tableScores[offset] == 0.0){
			sharedStruct->stop = 1;
			for(size_t j = 0; j < numberOfSlaves; ++j){ // fake messages to be sure no worker 
				// is waiting for a message
				myMsg msg;
				msg.type = 1; // the offset doesn't matter
				sendMessage(&msg);
			}
			printf("One of the Creatures was able to reach the goal tile\n");
			printf("All you can do now is watch his journey (B) or quit (Q)\n");
			destroyMaxHeap(heap);
			signal(2,1); // we have to signal we closed
			return;
		}
		insertIndex(offset, heap, tableScores); // we insert the index in the heap
	}
	
	int beginOffset = numberOfCreature * deletionRate / 100;
	while(sharedStruct->stop != 2){
		wait(1);
		if(sharedStruct->stop == 2){
			break;
		}
		
		for(int i = beginOffset; i < numberOfCreature; ++i){
			index = extractIndexForMax(heap, tableScores);
			modifyCreature(index, mutationRate, genomeLength);
			myMsg msg; // we send a message to a worker
			msg.type = 1;
			msg.offset = index;
			sendMessage(&msg);
		}
		
		for(int i = beginOffset; i < numberOfCreature; ++i){
			int offset;
			readMessage(2, &offset);
			if(offset == -1){
				break;
			}
			if(tableScores[offset] == 0.0){
				sharedStruct->stop = 1;
				for(size_t j = 0; j < numberOfSlaves; ++j){
					myMsg msg;
					msg.type = 1; // the offset doesn't matter
					sendMessage(&msg);
				}
				printf("One of the Creatures was able to reach the goal tile\n");
				printf("All you can do now is watch his journey (B) or quit (Q)\n");
				destroyMaxHeap(heap);
				signal(2,1); // we have to signal we closed
				return;
			}
			insertIndex(offset, heap, tableScores); // we insert the index in the heap
		}
	}
	destroyMaxHeap(heap);
	signal(2,1);
	return;
}
