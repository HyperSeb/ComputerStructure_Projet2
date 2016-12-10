#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <time.h>
#include <stddef.h>
#include <stdbool.h>
#include "gridHandler.h"
#include "process.h"

union semun{
    intval; 
    structsemid_ds* buf; // buffer for IPC_STAT, IPC_SET 
    unsignedshort int* array; // arrayfor GETALL, SETALL 
    structseminfo* __buf; // buffer for IPC_INFO 
};

// global variables
int qId;
int semId;
BestAndStop* sharedStruct;
double* tableScores;
int* tableGenes;


static int getArgumentInInterval(char** argv, int index, int lowerBound, int upperBound) {
    int value = atoi(argv[index]);
    if (lowerBound > value || value > upperBound) {
        fprintf(stderr, "Invalid argument %d\n", index);
        exit(EXIT_FAILURE);
    } else {
        return value
    }
}

/* create a shared memory segment, using the id number index, the key key and 
allocating a quantity of memory given by size
*/
static void* getSharedMemory(int index, key_t key, size_t size) {
    // open the shared memory segment - create if necessary
    int memId = shmget(key, size, IPC_CREAT|IPC_EXCL|0666);
    if(memId == -1){
        printf("Shared memory segment %d exists - opening as client\n", index+1);
        
        // segment probably already exists - try as a client
        if((memId = shmget(key, size, 0)) == -1){
            fprintf(stderr, "shmget\n");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Creating shared memory segment %d\n", index+1);
    }
    void* ptr = shmat(memId, 0, 0);
    // map the shared memory segment into the current process
    if(ptr == (void*)-1){
        fprintf(stderr, "shmat\n");
        exit(EXIT_FAILURE);
    }
    //the shared memory will only be closed when all the realted 
    //will be closed, so we can flag it to be closed now
    shmctl(memId, IPC_RMID, 0);
    return ptr;
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    // checks the arguments
    if(argc != 8 && argc != 9){
        fprintf(stderr, "Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }
    int M = getArgumentInInterval(argv, 1, /*bounds*/ 3, 5),
    N = getArgumentInInterval(argv, 2, 5, 10),
    P = getArgumentInInterval(argv, 3, 1, 10),
    C = getArgumentInInterval(argv, 4, 100, 1000),
    p = getArgumentInInterval(argv, 5, 20, 80),
    m = getArgumentInInterval(argv, 6, 1, 10),
    T = getArgumentInInterval(argv, 7, 5, 20);
    
    // if we randomly generate the grid, we have to increase the size of M and N by 1
    // and to genrerate walls on the outer border
    if(argc == 8){
        ++M;
        ++N;
    }
    
    // shared memory management
    key_t key1, key2, key3, key4, keysem, keyq;
    pid_t pid;
    union semun semopts;
    
    key1 = ftok(".", 'M'); // offset of the best creature, the begin and end in the grid
    key2 = ftok(".", 'N'); // table of scores
    key3 = ftok(".", 'O'); // table of genes
    key4 = ftok(".", 'P'); // grid
    keysem = ftok(".", 'S');
    keyq = ftok(".", 'Q');
    
    Grid grid = {NULL, N, M, {0,0}, {0,0}};

    sharedStruct = (bestBegEnd*) getSharedMemory(1, key1, sizeof(BestAndStop));
    tableScores = (double*) getSharedMemory(2, key2, C * sizeof(double));
    tableGenes = (int*) getSharedMemory(3, key3, C * T * sizeof(int));
    grid->storage = (bool*) getSharedMemory(4, key4, M * N * sizeof(bool));
    
    // creating the semaphore array
    printf("Attempting to create new semaphoreset with 3 members\n");
    if((semId= semget(keysem, 3, IPC_CREAT|IPC_EXCL|0666)) == -1){
        fprintf(stderr, "Semaphore set already exists\n");
        exit(EXIT_FAILURE);
    }
    
    // creating the queue
    printf("Attempting to create new message queue\n");
    if((qId = msgget(keyq, IPC_CREAT|IPC_EXCL|0666)) == -1){
        fprintf(stderr, "message queue already exists\n");
        semctl(semId,0,IPC_RMID,0)); // we "free" the semaphores
        exit(EXIT_FAILURE);
    }
    
    // Initialisation of the shared memory
    if(argc == 8){ 
        randomGrid(&grid); // random generation of the grid and Offsets
    }else{ // we use the provided file
        if (GridFromFile(grid, argv[8]) == -1){
            semctl(semId,0,IPC_RMID,0));
            msgctl(qId, IPC_RMID, 0);
            exit(EXIT_FAILURE);;
        }
    }
    sharedStruct->best = -1;
    sharedStruct->stop = 0;

    
    // semaphore initialisation, 0 handles the "mutual exclusion" of the best index 
    // in memory 1 handles the number of generation we still have to create
    // 2 is used to count the number of processes that have ended
    semopts.val = 1;
    semctl(semId, 0, SETVAL, semopts);
    semopts.val = 0;
    semctl(semId, 1, SETVAL, semopts);
    semctl(semId, 2, SETVAL, semopts);
    
    
    // Creating the worker and listener processes, all of them will end on exit(0),
    // thus we don't need to take care of what happens after the call to the process function
    int pid = 0;
    pid = fork();
    if (pid == 0){ // if we are the listener process
        listenerProcess(grid, P, T);
    }
    for(int i = 0; i < P; ++i){
        pid = fork();
        if(pid == 0){
            workerProcess(grid, T);
        }
        /*test for error*/
    }
    masterProcess(P, C, p, m, T);
    exit(EXIT_SUCCESS);
}
