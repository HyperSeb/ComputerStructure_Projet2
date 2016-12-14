
#include <stdlib.h>
#include <stdbool.h>
#include "PriorityQueue.h"

static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

MaxHeap* createMaxHeap(size_t capacity) {
    MaxHeap* heap = malloc(sizeof(MaxHeap));
    if (heap != NULL) {
        heap -> indices = malloc(capacity * sizeof(int));
        if (heap -> indices != NULL) {
            heap -> count = 0;
            heap -> capacity = capacity;
            return heap;
        }
        free(heap);
    }
    return NULL;
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
        swap(&indices[i], &indices[lowest]);
        maxHeapify(indices, lowest, end, array);
    }
}

void extractIndexForMax(MaxHeap* heap, double* array) {
    if (heap -> count == 0) {
        return;
    }
    
    heap -> count -= 1;
    swap(&heap->indices[0], &heap->indices[heap -> count]);
    
    maxHeapify(heap -> indices, 0, heap -> count, array);
}

static size_t parent(size_t index) {
    return (index == 0) ? 0 : (index - 1) / 2;
}

bool insertIndex(int value, MaxHeap* heap, double* array) {
    if (heap != NULL && ((heap -> count) < (heap -> capacity))) {
        int* indices = heap -> indices;
        
        size_t currentIndex = heap -> count++;
        size_t parentIndex = parent(currentIndex);
        
        indices[heap -> count] = value;
        
        while (array[indices[currentIndex]] > array[indices[parentIndex]]) {
            swap(&indices[currentIndex], &indices[parentIndex]);
            currentIndex = parentIndex;
            parentIndex = parent(currentIndex);
        }
        return true;
    } else {
        return false;
    }
}

bool isFull(MaxHeap* heap) {
    return heap -> capacity == heap -> count;
}

void destroyMaxHeap(MaxHeap* heap) {
    if (heap != NULL) {
        if (heap -> indices != NULL) {
            free(heap -> indices);
        }
        free(heap);
    }
}
