#include<stdlib.h>
#include<stdio.h>

/* turns the index of an element in a matrix of given width
into the index of the same element in a table representing the matrix
*/
int ind(int i,int j, int width);

// generates a random grid of height M and width N 
// (we consider here the outer "walls" to be part of the table)
// so N and M must have been incremented in the main function
void randomGrid(unsigned int M, unsigned int N);

// generates a grid of height M and width N from a file named "name"
void GridFromFile(unsigned int M, unsigned int N, char* name);
