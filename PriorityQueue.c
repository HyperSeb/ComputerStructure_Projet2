
#include <stdlib.h>
#include <stdbool.h>
#include "PriorityQueue.h"

static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

// create a MinHeap with the given capacity
MinHeap* createMaxHeap(size_t capacity) {
    MaxHeap* heap = malloc(sizeof(MaxHeap));
    if (heap != NULL) {
        heap -> elements = malloc(capacity * sizeof(int));
        if (heap -> elements != NULL) {
            heap -> count = 0;
            heap -> capacity = capacity;
            return heap;
        }
        free(heap);
    }
    return NULL
}

static void maxHeapify(int* indices, size_t i, size_t end, double* array) {
    size_t l = 2 * i + 1;
    size_t r = 2 * (i + 1);
    size_t lowest = i;
    
    if (l < end && array[indices[l]] > array[indices[lowest]]) {
        lowest = l;
    }
    if (r < end && array[indices[r]] > array[indices[lowest]]) {
        lowest = r;
    }
    if (lowest != i) {
        swap(&indices(i], &indices[lowest]);
        maxHeapify(heap, lowest, end, array);
    }
}

// extract the minimum value in the heap and place it after (at elements[count])
int extractIndexForMax(MaxHeap* heap, double* array) {
    if (heap -> count == 0) {
        return;
    }
    
    swap(&heap->indices[0], &heap->indices[--count]);
    
    maxHeapify(heap -> indices, 0, count, array);
    
    return heap->indices[count]
}

static size_t parent(size_t index) {
    return (index == 0) ? 0 : (index - 1) / 2;
}

// insert a new value in the heap, return false if capacity is too low
bool insertValueIn(int value, MaxHeap* heap, double* array) {
    if (heap != NULL && ((heap -> count) < (heap -> capacity))) {
        int* indices = heap -> indices;
        
        size_t currentIndex = heap -> count++;
        size_t parentIndex = parent(current);
        
        while (array[indices[currentIndex]] > array[indices[parentIndex]]) {
            swap(&indices[currentIndex], &indices[parentIndex]);
            currentIndex = parentIndex;
            parentIndex = parent(currentIndex);
        }
        return true
    } else {
        return false
    }
}

// return whether there is still place in the heap
bool isFull(MaxHeap* heap) {
    return heap -> capacity == heap -> count;
}

// destroy a heap
void destroyMaxHeap(MaxHeap* heap) {
    if (heap != NULL) {
        if (heap -> indices != NULL) {
            free(heap -> indices);
        }
        free(heap);
    }
}
