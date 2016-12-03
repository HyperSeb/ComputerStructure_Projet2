
#include <stdlib.h>
#include <stdbool.h>
#include "PriorityQueue.h"

static void swap(int* a, int* b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

// create a MinHeap with the given capacity
MinHeap* createMinHeap(size_t capacity) {
  MinHeap* heap = malloc(sizeof(MinHeap));
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

static void minHeapify(int* array, size_t i, size_t end) {
  size_t l = 2 * i + 1;
  size_t r = 2 * (i + 1);
  size_t lowest = i;
  
  if (l < end && array[l] < array[lowest]) {
    lowest = l;
  }
  if (r < end && array[r] < array[lowest]) {
    lowest = r;
  }
  if (lowest != i) {
    swap(&array[i], &array[lowest]);
    maxHeapify(array, lowest, end);
  }
}

// extract the minimum value in the heap and place it after (at elements[count])
void extractMin(MinHeap* heap) {
  if (heap -> count == 0) {
    return;
  }
  
  swap(&heap->elements[0], &heap->elements[--count]);
  
  minHeapify(heap -> elements, 0, count);
}

static size_t parent(size_t index) {
  return (index == 0) ? 0 : (index - 1) / 2;
}

// insert a new value in the heap, return false if capacity is too low
bool insertValueIn(int value, MinHeap* heap) {
  if (heap != NULL && ((heap -> count) < (heap -> capacity))) {
    int* elements = heap -> elements;
    
    size_t currentIndex = heap -> count++;
    size_t parentIndex = parent(current);
    
    while (elements[currentIndex] < elements[parentIndex]) {
      swap(&elements[currentIndex], &elements[parentIndex]);
      currentIndex = parentIndex;
      parentIndex = parent(currentIndex);
    }
    return true
  } else {
    return false
  }
}

// return whether there is still place in the heap
bool isFull(MinHeap* heap) {
  return heap -> capacity == heap -> count;
}

// destroy a heap
void destroyMinHeap(MinHeap* heap) {
  if (heap != NULL) {
    if (heap -> elements != NULL) {
      free(heap -> elements);
    }
    free(heap);
  }
}
