#define _SVID_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>
#include <stddef.h>
#include <stdbool.h>
#include "gridHandler.h"
#include "process.h"

union semun {
    int val; 
    struct semid_ds* buf; // buffer for IPC_STAT, IPC_SET 
    unsigned short int* array; // arrayfor GETALL, SETALL 
    struct seminfo* __buf; // buffer for IPC_INFO 
};

/* create a shared memory segment, using the id number index, the key key and 
allocating a quantity of memory given by size
*/
static void* getSharedMemory(int index, key_t key, size_t size) {
    // open the shared memory segment - create if necessary
    int memId = shmget(key, size, IPC_CREAT|IPC_EXCL|0666);
    if(memId == -1){
        printf("Shared memory segment %d exists - opening as client\n", index);
        
        // segment probably already exists - try as a client
        if((memId = shmget(key, size, 0)) == -1){
            fprintf(stderr, "shmget\n");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Creating shared memory segment %d\n", index);
    }
    void* ptr = shmat(memId, 0, 0);
    // map the shared memory segment into the current process
    if(ptr == (void*)-1){
        fprintf(stderr, "shmat\n");
        exit(EXIT_FAILURE);
    }
    //the shared memory will only be closed when all the realted 
    //processes will be closed, so we can flag it to be closed now
    shmctl(memId, IPC_RMID, 0);
    return ptr;
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    // checks the arguments
    if(argc != 8 && argc != 9){
        fprintf(stderr, "Invalid number of arguments, expected 7 or 8 arguments: \n");
        int index = 0;
#define ARGUMENT(shortName, lowerBound, upperBound, longName, description)\
        fprintf(stderr, "argument n°%d, " #shortName " between " #lowerBound " and " #upperBound " which is the " description "\n", ++index);
#include "arguments.txt"
        fprintf(stderr, "optional argument n°%d, the name of a file which contains the characteristics of the grid\n", ++index);
        exit(EXIT_FAILURE);
    }

    int index = 0;
    bool badArguments = false;
#define ARGUMENT(shortName, lowerBound, upperBound, longName, description)\
    int shortName = atoi(argv[++index]);\
    if (lowerBound > shortName || shortName > upperBound) {\
        fprintf(stderr, "Invalid value '%s' for argument n°%d, " #shortName " (which is the " description ") is expected to be between " #lowerBound " and " #upperBound "\n", argv[index], index);\
        badArguments = true;\
    }
#include "arguments.txt"
    if (badArguments) {
        exit(EXIT_FAILURE);
    }
    
    // shared memory management
    key_t key1, key2, key3, key4, keysem, keyq;
    int qId, semId;
    BestAndStop* sharedStruct;
    union semun semopts;
    
    key1 = ftok(".", 'M'); // offset of the best creature, the begin and end in the grid
    key2 = ftok(".", 'N'); // table of scores
    key3 = ftok(".", 'O'); // table of genes
    key4 = ftok(".", 'P'); // grid
    keysem = ftok(".", 'S');
    keyq = ftok(".", 'Q');
    
    Grid grid = {NULL, N, M, {0,0}, {0,0}};
    Genomes genomes = {NULL, C, T};
    double* scores = NULL;

    sharedStruct = (BestAndStop*) getSharedMemory(1, key1, sizeof(BestAndStop));
    scores = (double*) getSharedMemory(2, key2, C * sizeof(double));
    genomes.storage = (int*) getSharedMemory(3, key3, C * T * sizeof(int));
    grid.storage = (bool*) getSharedMemory(4, key4, M * N * sizeof(bool));
    
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
        semctl(semId,0,IPC_RMID,0); // we "free" the semaphores
        exit(EXIT_FAILURE);
    }
    
    // Initialisation of the shared memory
    if(argc == 8){ 
        fillGridRandomly(&grid); // random generation of the grid and Offsets
    }else{ // we use the provided file
        if (fillGridWithFile(&grid, argv[8]) == -1){
            semctl(semId,0,IPC_RMID,0);
            msgctl(qId, IPC_RMID, 0);
            exit(EXIT_FAILURE);
        }
    }
    sharedStruct->best = -1;
    sharedStruct->stop = 0;

    
    // semaphore initialisation, 0 handles the "mutual exclusion" of the best index 
    // in shared memory, 1 handles the number of generation we still have to create,
    // 2 is used to count the number of processes that have ended
    semopts.val = 1;
    semctl(semId, 0, SETVAL, semopts);
    semopts.val = 0;
    semctl(semId, 1, SETVAL, semopts);
    semctl(semId, 2, SETVAL, semopts);
    
    
    // Creating the worker and master processes, all of them will end on exit(),
    // thus their processes never reach what is after the call to their "process function"
    pid_t pid = 0;
    pid = fork();
    if (pid < 0){ // if the fork doesn't works, we close
        fprintf(stderr, "the master's fork didn't worked\n");
        semctl(semId,0,IPC_RMID,0);
        msgctl(qId, IPC_RMID, 0);
        exit(EXIT_FAILURE);
    }
    if (pid == 0){ // if we are the master process
        masterProcess(P, p, m, genomes, scores, qId, semId, sharedStruct);
    }
    for(int i = 0; i < P; ++i){
        pid = fork();
        if (pid < 0){ // the previously created processes will end since 
            // the semaphores/waitqueues are destroyed
            fprintf(stderr, "one of the worker's fork didn't worked\n");
            semctl(semId,0,IPC_RMID,0);
            msgctl(qId, IPC_RMID, 0);
            exit(EXIT_FAILURE);
        }
        if(pid == 0){
            workerProcess(grid, genomes, scores, qId, semId, sharedStruct);
        }
    }
    listenerProcess(grid, genomes, P, qId, semId, sharedStruct);
    exit(EXIT_SUCCESS);
}
