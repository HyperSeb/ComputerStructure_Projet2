#ifndef gridHandler_h
#define gridHandler_h

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// the value representing an obstacle in the grid
#define obstacle true

// the x,y position of a tile
struct Position {
    int x;
    int y;
};
typedef struct Position Position;

// checks if two positions are the same 
bool equalPos(Position p1, Position p2);

// a structure representing the grid
struct Grid {
    bool* storage;
    int width;
    int height;
    Position start;
    Position finish;
};
typedef struct Grid Grid;

// returns the element of the grid at the given position
bool getInGrid(Grid grid, Position position);

// sets the element of the grid at the given position
void setInGrid(Grid grid, Position position, bool value);

// fills the grid randomly, and set start and finish at random
void fillGridRandomly(Grid* grid);

/* fill the grid of the given size with content from a file named "name"
checks if there is no tile over the begin/end flag and if start and end 
are different, returns -1 if an error occured
*/
int fillGridWithFile(Grid* grid, char* name);

// display the grid on stdout
void displayGrid(Grid grid, Position creaturePosition, int* genome, int genomeLength, int geneIndex);


#endif /*gridHandler_h*/
