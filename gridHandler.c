#include<stddef.h>
#include<stdbool.h>
#include "gridHandler.h"

#define ind(i,j,width) width*i + j

// génère aléatoirement la grille ET INITIALISE LES 3 CHAMPS DE OFFSET
void randomGrid(unsigned int M, unsigned int N){
	for(size_t k = 0; k < N*M; ++k){
			Grid[k] = (bool) rand()%2;
		}
		// the borders have to be Tiles
		// first and last column
		for(size_t i = 0; i < M; ++i){
			Grid[ind(i,0,N)] = true;
			Grid[ind(i,N-1,N)] = true;
		}
		// first and last line
		for(size_t j = 1; j < N-1; ++j){
			Grid[ind(0,j,N)] = true;
			Grid[ind(M-1,j,N)] = true;
		}
		// positions of the Begin and End
		size_t i1,j2, i2, j2;
		i1 = rand()%(M-2)+1;
		j2 = rand()%(N-2)+1;
		Grid[ind(i1,j1,N)] = false;
		Offsets->begin = ind(i1,j1,N);
		i2 = rand()%(M-2)+1;
		j2 = rand()%(N-2)+1;
		while (i1 == i2 && j1 == j2){ // Begin should be != from end
			i2 = rand()%(M-2)+1;
			j2 = rand()%(N-2)+1;
		}
		Grid[ind(i2,j2,N)] = false;
		Offsets->end = ind(i1,j1,N);
		Offsets->best = -1;
		Offsets->stop = 0;
	}
}
	
int GridFromFile(unsigned int M, unsigned int N, char* name){
	
}