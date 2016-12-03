#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/msg.h>
#include<time.h>
#include<stddef.h>
#include<stdbool.h>
#include"gridHandler.h"
#include"process.h"



// global variables
int qId;
int semId;
int memId1, memId2, memId3, memId4;
bestBegEnd* Offsets;
double* TableScores;
int* TableGenes; 
bool* Grid;

int main(int argc, char* argv[])
{
	srand(time(NULL));
	// checks the arguments
	if(argc != 8 && argc != 9){
		fprintf(stderr, "Invalid number of arguments\n");
		exit(Exit);
	}else{
		int M = atoi(argv[1]), N = atoi(argv[2]), P = atoi(argv[3]), C = atoi(argv[4]), p = atoi(argv[5]),
		m = atoi(argv[6]), T = atoi(argv[7]);
		if(M < 3 || M > 5){
			fprintf(stderr, "Invalid argument 1\n");
			exit(1);
		}
		if( N < 5 ||  N > 10){
			fprintf(stderr, "Invalid argument 2\n");
			exit(1);
		}
		if(P < 1 || P > 10){
			fprintf(stderr, "Invalid argument 3\n");
			exit(1);
		}
		if(C < 100 || C > 1000){
			fprintf(stderr, "Invalid argument number 4\n");
			exit(1);
		}
		if(p < 20 || p > 80){
			fprintf(stderr, "Invalid argument number 5\n");
			exit(1);
		}
		if(m < 1 || m > 10){
			fprintf(stderr, "Invalid argument number 6\n");
			exit(1);
		}
		if(T < 5 || T > 20){
			fprintf(stderr, "Invalid argument 7\n");
			exit(1);
		}
	}
	
	// shared memory management
	key_t key1, key2, key3, key4, keysem, keyq;
	pid_t pid;
	int id, cntr;
	union semun semopts;

	key1 = ftok(".", 'M'); // offset of the best creature, the begin and end in the grid
	key2 = ftok(".", 'N'); // table of scores
	key3 = ftok(".", 'O'); // table of genes
	key4 = ftok(".", 'P'); // grid
	keysem = ftok(".", 'S');
	keyq = ftok(".", 'Q');
	

	// open the shared memory segment 1 - create if necessary
	if((memId1 = shmget(key1, sizeof(bestBegEnd), IPC_CREAT|IPC_EXCL|0666)) == -1){
		printf("Shared memory segment 1 exists - opening as client\n");
		
		// segment probably already exists - try as a client
		if((memId1 = shmget(key1, sizeof(bestBegEnd), 0)) == -1){
		fprintf(stderr, "shmget\n");
		exit(1);
		}
	}else{
		printf("Creating shared memory segment 1\n");
	}
	// map the shared memory segment into the current process
	if((Offsets = (bestBegEnd*)shmat(memId1, 0, 0)) == (bestBegEnd*)-1){
		fprintf(stderr, "shmat\n");
		exit(1);
	}
	
	// open the shared memory segment 2 - create if necessary
	if((memId2 = shmget(key2, C*sizeof(double), IPC_CREAT|IPC_EXCL|0666)) == -1){
		printf("Shared memory segment 2 exists - opening as client\n");
		
		if((memId2 = shmget(key2, C*sizeof(double), 0)) == -1){
		fprintf(stderr, "shmget\n");
		exit(1);
		}
	}else{
		printf("Creating shared memory segment 2\n");
	}
	// map the shared memory segment into the current process
	if((TableScores = (double*)shmat(memId2, 0, 0)) == (double*)-1){
		fprintf(stderr, "shmat\n");
		exit(1);
	}
	
	// open the shared memory segment 3 - create if necessary
	if((memId3 = shmget(key3, C*T*sizeof(int), IPC_CREAT|IPC_EXCL|0666)) == -1){
		printf("Shared memory segment 3 exists - opening as client\n");
		
		if((memId3 = shmget(key3, C*T*sizeof(int), 0)) == -1){
		fprintf(stderr, "shmget\n");
		exit(1);
		}
	}else{
		printf("Creating shared memory segment 3\n");
	}
	// map the shared memory segment into the current process
	if((TableGenes = (int*)shmat(memId3, 0, 0)) == (int*)-1){
		fprintf(stderr, "shmat\n");
		exit(1);
	}
	
	// open the shared memory segment 4 - create if necessary
	if((memId4 = shmget(key4, M*N*sizeof(bool), IPC_CREAT|IPC_EXCL|0666)) == -1){
		printf("Shared memory segment 4 exists - opening as client\n");
		
		if((memId4 = shmget(key4, M*N*sizeof(bool), 0)) == -1){
		fprintf(stderr, "shmget\n");
		exit(1);
		}
	}else{
		printf("Creating shared memory segment 4\n");
	}
	// map the shared memory segment into the current process
	if((Grid = (bool*)shmat(memId4, 0, 0)) == (bool*)-1){
		fprintf(stderr, "shmat\n");
		exit(1);
	}
	
	// creating the semaphore array
	printf("Attempting to create new semaphoreset with 2 members\n");
	if((semId= semget(keysem, 2, IPC_CREAT|IPC_EXCL|0666)) == -1){
		fprintf(stderr, "Semaphore set already exists\n");
		exit(1);
	}
	
	// creating the queue
	printf("Attempting to create new message queue\n");
	if((qId = msgget(keyq, IPC_CREAT|IPC_EXCL|0666)) == -1){
		fprintf(stderr, "message queue already exists\n");
		exit(1);
	}
	
	// Initialisation of the shared memory
	if(argc == 8){ 
	randomGrid(M, N); // random generation of the grid and Offsets
	}else{ // we use the provided file
	GridFromFile(M, N, argv[8]);
	}
	
	// semaphore initialisation, 0 handles the "mutual exclusion" between the listener 
	// and the master, 1 handles the number of generation we still have to create
	semopts.val = 1;
	semctl(semId, 0, SETVAL, semopts);
	semopts.val = 0;
	semctl(semId, 1, SETVAL, semopts);
	
	
	// Creating the worker and listener processes, all of them will end on exit(0),
	// thus we don't need to take care of what happens after the call to the process function
	int pid = 0;
	pId = fork();
	if (pId == 0){ // if we are the listener process
		listenerProcess(int M, int N, int P, int T);
	}
	for(size_t i = 0; i < P; ++i){
		pid = fork();
		if(pid == 0){
			workerProcess(int M, int N, int T);
		}
	}
	masterProcess(int P, int C, int p, int m);
	exit(0)
}