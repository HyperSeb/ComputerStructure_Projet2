#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// the value representing an obstacle in the grid
const bool obstacle = true;

/* turns the index of an element in a matrix of given width
into the index of the same element in a table representing the matrix
*/
int ind(int i,int j, int width);

/* generates a random grid of height M and width N 
(we consider here the outer "walls" to be part of the table)
so N and M must have been incremented in the main function
*/
void randomGrid(unsigned int M, unsigned int N);

/* generates a grid of height M and width N from a file named "name"
checks if there is a border of tiles and if there is no tile over the
begin/end flag
return -1 if an error occured
*/
int gridFromFile(unsigned int M, unsigned int N, char* name);
