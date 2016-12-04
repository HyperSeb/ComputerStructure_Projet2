#include<stdlib.h>
#include<stdio.h>

/* turns the index of an element in a matrix of given width
into the index of the same element in a table representing the matrix
*/
int ind(int i,int j, int width);

// generates a random grid of height M and width N
void randomGrid(unsigned int M, unsigned int N);

// generates a grid of height M and width N from a file named "name"
int GridFromFile(unsigned int M, unsigned int N, char* name);
