#ifndef gridHandler_h
#define gridHandler_h

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// the value representing an obstacle in the grid
#define obstacle true

struct Position {
    int x;
    int y;
};
typedef struct Position Position;

/* check if two positions are the same 
*/
bool equalPos(Position p1, Position p2);

struct Grid {
    bool* storage;
    int width;
    int height;
    Position start;
    Position finish;
};
typedef struct Grid Grid;

/* return the element of the grid at the given position
*/
bool getInGrid(Grid grid, Position position);

/* return the element of the grid at the given position
*/
void setInGrid(Grid grid, Position position, bool value);

/* fill the grid randomly, and set start and finish at random
*/
void fillGridRandomly(Grid* grid);

/* fill the grid of the given size with content from a file named "name"
checks if there is a border of tiles and if there is no tile over the
begin/end flag
return -1 if an error occured
*/
int fillgridWithFile(Grid* grid, char* name);

/* display the grid on stdout
*/
void displayGrid(Grid grid, Position creaturePosition, int* genome, int genomeLength, int geneIndex);


#endif /*gridHandler_h*/
