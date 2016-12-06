
#ifndef PriorityQueue_h
#define PriorityQueue_h

#include <stdbool.h>

struct MinHeap {
    int* indices;
    size_t count;
    size_t capacity;
};

typedef struct MinHeap MinHeap;

// create a MinHeap with the given capacity
MinHeap* createMinHeap(size_t capacity);

// extract the minimum value in the heap and place it after (at elements[count])
int extractIndexForMin(MinHeap* heap, double* array);

// insert a new value in the heap, return false if capacity is too low
void insertIndex(int index, MinHeap* heap, double* array);

// return whether there is still place in the heap
bool isFull(MinHeap* heap);

// destroy a heap
void destroyMinHeap(MinHeap* heap);

#endif /* PriorityQueue_h */
