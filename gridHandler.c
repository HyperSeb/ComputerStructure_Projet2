#include<stddef.h>
#include<stdbool.h>
#include "gridHandler.h"

bool equalPos(Position p1, Position p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

// return a pointer to an adress of the grid
static bool* ptrInGrid(Grid grid, Position position) {
	if (!(0 <= position.x && position.x < grid.width && 0 <= position.y && position.y < grid.height)) {
		return NULL;
	}
	return &(grid.storage[position.x * grid.width + position.y]);
}

bool getInGrid(Grid grid, Position position) {
	bool* ptr = ptrInGrid(grid, position);
	return ptr == NULL ? obstacle : *ptr;
}
 
void setInGrid(Grid grid, Position position, bool value) {
   bool* ptr = ptrInGrid(grid, position);
	if (ptr != NULL) {
		*ptr = value;
	} else {
		fprintf(stderr, "setInGrid out of the grid");
	}
}

void fillGridRandomly(Grid* grid) {
    for (Position p = {0, 0}; p.y < grid->height; p.y++) {
        for (p.x = 0; p.x < grid->width; p.x++) {
            setInGrid(*grid, p, (rand()%3) == 0 ? obstacle : !obstacle); // one position in 3 is an obstacle
        }
    }
    
    Position start, finish;
    start.x = rand() % grid->width;
    start.y = rand() % grid->height;
    
    setInGrid(*grid, start, !obstacle);
    grid->start = start;
    
    // we want the finish nor to be in the air nor to be on the start
    do {
        finish.x = rand() % grid->width;
        finish.y = rand() % grid->height;
        
        Position underFinish = {finish.x, finish.y - 1};
        while (getInGrid(*grid, underFinish) != obstacle) {
            underFinish.y -= 1;
            finish.y -= 1;
        }
    } while (equalPos(start, finish));
        
    setInGrid(*grid, finish, !obstacle);
    grid->finish = finish;
}

/*reads a position in the file, returns 0 if the position is fine,
-1 if the read failed and -2 if it is out of bounds */
static int readPosition(FILE* gridFile, Grid grid, Position* p) {
    if(fscanf(gridFile, "%d %d", &(p->x), &(p->y)) == -1) {
        return -1;
    } else if (p->x >= (grid.width) || p->x < 0 || p->y >= (grid.height) || p->y < 0) {
        fprintf(stderr, "position in the file is out of bound \n");
        return -2;
    } else {
        return 0;
    }
}

int fillGridWithFile(Grid* grid, char* name) {
    FILE* gridFile = fopen(name, "r");
    int result = -1;
    if (gridFile != NULL) {
    	Position tmpPosition;
        
        // fill with free tiles
        for (Position p = {0, 0}; p.y < grid->height; p.y++) {
            for (p.x = 0; p.x < grid->width; p.x++) {
                setInGrid(*grid, p, !obstacle);
            }
        }
    	//start tile
        if(readPosition(gridFile, *grid, &tmpPosition) == 0) {
            grid->start = tmpPosition;
            // finish tile
            if(readPosition(gridFile, *grid, &tmpPosition) == 0) {
                grid->finish = tmpPosition;
        	
				if(!equalPos(grid->finish, grid->start)){
                	int readStatus;
                
             	   while ((readStatus = readPosition(gridFile, *grid, &tmpPosition)) == 0) {
               	     setInGrid(*grid, tmpPosition, obstacle);
            	    }
            	    // it must be because read failed
            	    if (readStatus == -1) {
            	        // checks if there is a tile on the stating/ending position
						if(getInGrid(*grid, grid->start) != obstacle && getInGrid(*grid, grid->finish) != obstacle) {
           	          	   result = 0;
						} else {
							fprintf(stderr, "there is a tile over the begin/end position");
						}
                	} else {
                    	fprintf(stderr, "bad obstacle position \n");
                	}
				} else {
					fprintf(stderr, "same start and end position \n");
				}
            } else {
                fprintf(stderr, "bad finish position \n");
            }
        } else {
            fprintf(stderr, "bad start position \n");
        }
        fclose(gridFile);
    } else {
        fprintf(stderr, "Could not open file \n");
    }
    return result;
}
