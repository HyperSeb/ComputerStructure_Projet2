#include<stddef.h>
#include<stdbool.h>
#include "gridHandler.h"

int ind(i,j,width){
	return width*i + j;
} 

void randomGrid(unsigned int M, unsigned int N){
	for(size_t k = 0; k < N*M; ++k){
		grid[k] = (rand()%3) == 0; // one position in 3 is an obstacle
	}
	// the borders have to be Tiles
	// first and last column
	for(size_t i = 0; i < M; ++i){
		grid[ind(i,0,N)] = true;
		grid[ind(i,N-1,N)] = true;
	}
	// first and last line
	for(size_t j = 1; j < N-1; ++j){
		grid[ind(0,j,N)] = true;
		grid[ind(M-1,j,N)] = true;
	}
	// positions of the Begin and End
	size_t i1,j2, i2, j2;
	i1 = rand()%(M-2)+1;
	j2 = rand()%(N-2)+1;
	grid[ind(i1,j1,N)] = false;
	sharedStruct->begin = ind(i1,j1,N);
	i2 = rand()%(M-2)+1;
	j2 = rand()%(N-2)+1;
	while (i1 == i2 && j1 == j2){ // Begin should be != from end
		i2 = rand()%(M-2)+1;
		j2 = rand()%(N-2)+1;
	}
	grid[ind(i2,j2,N)] = false;
	sharedStruct->end = ind(i1,j1,N);
	sharedStruct->best = -1;
	sharedStruct->stop = 0;
}
	
int gridFromFile(unsigned int M, unsigned int N, char* name){
	FILE* gridFile = fopen(path, "r");
    if (gridFile == NULL) {
        fprintf(stderr, "Could not open file \n");
		return -1;
    }
	int tmpX, tmpY, tmpI;
	//begin tile
	if(fscanf(gridFile, "%d %d", &tmpX, &tmpY) == -1){
		fprintf(stderr, "unable to read the starting position present in file \n");
		return -1;
	}
	if(tmpX >= (N-2) || tmpX <= 0 || tmpY >= (M-2) || tmpY <= 0){
		fprintf(stderr, "invalid starting position present in file \n");
		return -1;
	}
	tmpI = M - 1 - tmpY;
	sharedStruct->begin = ind(tmpI, tmpX, N);
	
	//end tile
	if(fscanf(gridFile, "%d %d", &tmpX, &tmpY) == -1){
		fprintf(stderr, "unable to read the starting position present in file \n");
		return -1;
	}
	if(tmpX >= (N-2) || tmpX <= 0 || tmpY >= (M-2) || tmpY <= 0){
		fprintf(stderr, "invalid goal position present in file \n");
		return -1;
	}
	tmpI = M - 1 - tmpY;
	sharedStruct->end = ind(tmpI, tmpX, N);
	
	// obstacles
	for(size_t k = 0; k < N*M; ++k){
		grid[k] = false;
	}
	while(fscanf(gridFile, "%d %d", &tmpX, &tmpY) != -1){
		if(tmpX >= (N-1) || tmpX < 0 || tmpY >= (M-1) || tmpY < 0){
			fprintf(stderr, "invalid tile position present in file \n");
			return -1;
		}
		tmpI = M - 1 - tmpY;
		grid[ind(tmpI, tmpX, N)] = true;
	}
	
	// checks we have an outer wall of tiles
	// first and last column
	for(size_t i = 0; i < M; ++i){
		if( grid[ind(i,0,N)] == false || grid[ind(i,N-1,N)] == true){
			fprintf(stderr, "missing border tile at line %d", i);
			return -1;
		}
	}
	// first and last line
	for(size_t j = 1; j < N-1; ++j){
		if(grid[ind(0,j,N)] == false || grid[ind(M-1,j,N)] == false){
			fprintf(stderr, "missing border tile at line %d", i);
			return -1;
		}
	}
	
	// checks if there is a tile on the stating/ending position
	if(grid[Offsets->begin] == true || grid[Offsets->end] == true){
		fprintf(stderr, "there is a tile over the begin/end position");
		return -1;
	}
	sharedStruct->best = -1;
	sharedStruct->stop = 0;
	fclose(gridFile);
	return 0;
}
