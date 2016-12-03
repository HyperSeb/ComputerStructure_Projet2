#include<stddef.h>
#include<stdbool.h>
#include<time.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/msg.h>
#include "process.h"

#define ind(i,j,width) width*i + j

static void wait(int offset){
	struct sembuf buf = { offset, -1, 0};
	semop(semId, &buf, 1); // we ignore the errors since we have the certitude
//to trigger some when we close the program
}

static void signal(int offset){
	struct sembuf buf = { offset, 1,0};
	semop(semId, &buf, 1);// we ignore the errors since we have the certitude
//to trigger some when we close the program
}

static void sendMessage(myMsg* msg){
	msgsnd(qId,msg, sizeof(myMsg) - sizeof(long), 0);// we ignore the errors 
//since we have to certitude to trigger some when we close the program
}

static void readMessage(long type, int* offset){
    msgrcv(qId, offset, sizeof(myMsg) - sizeof(long), type,  0));// we ignore the errors 
//since we have to certitude to trigger some when we close the program
}

// show the best creature's movements on the terminal
static void showBest(int M, int N, int* bestCreature, int T){
	
}

// computes the score of the creature and possibly replace the best creature's index
static void computeScore(int M, int N, int T, int offset){
	// c'est pas cette fonction qui gère le cas où on aurait un bestScore == 0
}

void listenerProcess(int M, int N, int P, int T){
	printf("type: G to request a new generation\n");
	printf("      M followed by a number to request that number of generations\n");
	printf("      B to display the best creature so far\n");
	printf("      Q to close the program\n");
	while(true){
		char tmp = 'a';
		unsigned int number = 0;
		scanf(%c, &tmp);
		switch (tmp) {
		case 'G' :
			signal(1);
			break;
		case 'M' :
			scanf(%ud, &number);
			for(unsigned int i = 0; i < number; i++){
				signal(1);
			}
			break;
		
		case 'B' :
			wait(0);
			best = Offsets->best;
			if(best == -1){
				printf("we haven't evaluated any creatures yet\n")
			}
			// we copy the table in order not to block the master process during T seconds
			int bestCreature[T];
			for(int j = 0; j < T; ++j){
				bestCreature[j] = TableGenes[ind(best,j,T)];
			}
			signal(0);
			showBest(M, N, bestCreature, T);
			break;
		
		case 'Q' :
			// delete the semaphore/message queue immediately and the shared memory 
			// when all processes will be closed
			for (int i = 0; i < 4; i++) {
				shmctl(memId[i], IPC_RMID, 0);
			}
			semctl(semId,0,IPC_RMID,0));
			msgctl(qId, IPC_RMID, 0); // will fail if the master already closed 
			// the worker processes but we don't care
			break;
		
		default: 
			printf("invalid command \n");
		}
	}
	exit(0)
}

void workerProcess(int M, int N, int T){
	int offset;
	while(true){
		readMessage(1, &offset);
		computeScore(M, N, T, offset);
		myMsg msg; // tells the master the math is done
		msg.type = 2;
		msg.offset = offset;
		sendMessage(&msg);
	}
	exit(0)
}

void masterProcess(int P, int C, int p, int m){
	int beginOffset = 0;
	while(true){
		wait(1);
		for(int i = beginOffset; i < C; ++i){
			myMsg msg; // we send a message to a worker
			msg.type = 1;
			msg.offset = i;
			sendMessage(&msg);
		}
		for(int i = beginOffset; i < C; ++i){
			int offset;
			readMessage(2, &offset);
			if(TableScores[offset] == 0.0){ 
				msgctl(qId, IPC_RMID, 0); // we need to close the worker processes
				printf("One of the processes was able to reach the goal tile\n");
				printf("All you can do now is watch his journey (B) or quit (Q)\n");
				return;
			}
			// sort the new element
		}
		wait(0); // we could destroy the creature that was best at a moment
		beginOffset = C * p / 100;
		// handles the mutations between beginOffset and C-1
		signal(0);
	}
}
