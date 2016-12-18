
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>

#include "gridHandler.h"
#include "creature.h"

// return a random gene
int randomGene() {
    return rand()%8;
}

static bool moreThanHalf(int half, int full) {
    if (full == 0) {
        return true;
    } else if (full > 0){
        return 2 * half >= full;
    } else {
        return 2 * half <= full;
    }
}

// assuming deltaX = -1, 0 or 1, else undefined behaviour may appear
static Position computeResultOfMove(Grid grid, Position from, int deltaX, int deltaY) {
    int verticalDirection = deltaY < 0 ? -1 : 1;
    
    if (deltaX == 0) {
        // vertical move, go forward until end or obstacle
        for (int dy = 0; dy * verticalDirection < deltaY * verticalDirection; dy += verticalDirection) {
            Position further = {from.x, from.y + dy + verticalDirection};
            
            if (getInGrid(grid, further) == obstacle) {
                Position current = {from.x, from.y + dy};
                return current;
            }
        }
        
        Position current = {from.x, from.y + deltaY};
        return current;
        
    } else if (deltaY == 0) {
        // horizontal move, the destination is either an obstacle or not
        Position further = {from.x + deltaX, from.y};
        if (getInGrid(grid, further) == obstacle) {
            return from;
        } else {
            return further;
        }
    } else {
        // diagonal move, go vertically forward,
        // if there is an obstacle in the column or in the column in the direction of deltaX, a special comportement is performed
        for (int dy = 0; dy * verticalDirection < deltaY * verticalDirection; dy += verticalDirection) {
            Position current = {from.x + (moreThanHalf(dy, deltaY) ? deltaX : 0), from.y + dy};
            
            Position further = {from.x, from.y + dy + verticalDirection},
            nextToFurther = {from.x + deltaX, from.y + dy + verticalDirection};
            
            if (getInGrid(grid, further) == obstacle && getInGrid(grid, nextToFurther) == obstacle) {
                // both are obstacle, don't go further
                return current;
                
            } else if (getInGrid(grid, further) == obstacle) {
                // the one in the column we start the move is an obstacle,
                // depending on how far we are in the move, we land on the obstacle, or continue
                if (moreThanHalf(dy + verticalDirection, deltaY)) {
                    continue;
                } else {
                    return current;
                }
                
            } else if (getInGrid(grid, nextToFurther) == obstacle) {
                // the one in the column we are going is an obstacle,
                // depending on how far we are in the move, we land on it, or fall vertically
                if (moreThanHalf(dy + verticalDirection, deltaY)) {
                    if (moreThanHalf(dy, deltaY)) {
                        return current;
                    } else {
                        return computeResultOfMove(grid, current, 0, deltaY - dy); // the rest of the move is vertical
                    }
                } else {
                    return computeResultOfMove(grid, current, 0, deltaY - dy); // the rest of the move is vertical
                }
                
            } else {
                // no obstacle, we continue
                continue;
            }
        }
        
        // no problematic obstacle were met, the move is completely done
        Position current = {from.x + deltaX, from.y + deltaY};
        return current;
    }
}

// return the final position of the creature
static Position performCreature(Grid grid, int* genome, int genomeLength, bool displayingSteps) {
    Position currentPosition = grid.start;

    for (int geneIndex = -1; geneIndex < genomeLength; geneIndex++) {
        int direction = geneIndex == -1 ? -1 : genome[geneIndex];
        int deltaX, deltaY;
        
        switch (direction) {
                
#define MOVE(dX, dY, intValue, character) \
            case intValue:\
                deltaX = dX;\
                deltaY = dY;\
                break;
#include "move.txt"
                
            default:
                deltaX = 0;
                deltaY = 0;
                break;
        }
        Position underPosition;
        do {
            Position nextPosition = computeResultOfMove(grid, currentPosition, deltaX, deltaY);
            
            deltaX = nextPosition.x - currentPosition.x; // the fall continue to be vertical if the partial move was
            deltaY--; // gravity
            
            currentPosition = nextPosition;
            
            if (displayingSteps) {
                printf("\n\n\n\n\n\n");
                displayGenome(genome, genomeLength, geneIndex);
                printf("\n");
                displayGrid(grid, currentPosition);
                sleep(1);
            }
            
            underPosition.x = currentPosition.x;
            underPosition.y = currentPosition.y - 1;

        } while (getInGrid(grid, underPosition) != obstacle);
    }
    
    if (displayingSteps) {
        printf("The journey is ended!\n");
    }
    return currentPosition;
}

// show the best creature's movements on the terminal
void showBest(Grid grid, int* genome, int genomeLength) {
	performCreature(grid, genome, genomeLength, true);
}

// computes the score of the creature
double computeScore(Grid grid, int* genome, int genomeLength) {
	// c'est pas cette fonction qui gère le cas où on aurait un bestScore == 0
	// ne gere pas non plus le cas ou on changerait le meilleur
    Position finalPosition = performCreature(grid, genome, genomeLength, false);
    
    int dX = finalPosition.x - grid.finish.x;
    int dY = finalPosition.y - grid.finish.y;
    
    return sqrt(dX * dX + dY * dY);
}

// display the genome on stdout, indicating gene at `geneIndex`
void displayGenome(int* genome, int genomeLength, int geneIndex) {
    for (int i = 0; i < genomeLength; i++) {
        switch (genome[i]) {
#define MOVE(dX, dY, intValue, character) \
            case intValue:\
                printf(character);\
                break;
#include "move.txt"
            default:
                fprintf(stderr, "unexpected inexistant genome code\n");
        }
    }
    printf("\n");
    if (geneIndex == -1) {
        printf("start");
    } else {
        for (int i = 0; i < genomeLength; i++) {
            if (i == geneIndex) {
                printf("^");
            } else {
                printf(" ");
            }
        }
    }
    printf("\n");
}

// display the grid on stdout
void displayGrid(Grid grid, Position creaturePosition) {
	
    static const char* face = "☻"; // utf8: {0xE2, 0x98, 0xBB, '\0'};
    static const char* start = "S";
    static const char* finish = "F";
    static const char* obstacleCenter = "X";
    
    printf("+");
    for (int j = 0; j < grid.width; j++) {
        printf("---+");
    }
    printf("\n");
    for (Position p = {0, grid.height-1}; p.y >= 0; p.y--) {
        printf("|");
        for (p.x = 0; p.x < grid.width; p.x++) {
            // ___|
            printf(" ");
            if (equalPos(p, creaturePosition)) {
                printf("%s", face);
            } else if (equalPos(p, grid.start)) {
                printf("%s", start);
            } else if (equalPos(p, grid.finish)) {
                printf("%s", finish);
            } else if (getInGrid(grid, p) == obstacle) {
                printf("%s", obstacleCenter); // distinction between obstacles and free place with obstacles around
            } else {
                printf(" ");
            }
            printf(" ");
            
            Position nextP = {p.x + 1, p.y};
            
            if (nextP.x == grid.width) {
                printf("|");
            } else if (getInGrid(grid, p) == obstacle || getInGrid(grid, nextP) == obstacle) {
                printf("|");
            } else {
                printf(" ");
            }
        }
        printf("\n");
        
        printf("+");
        for (p.x = 0; p.x < grid.width; p.x++) {
            Position underP = {p.x, p.y - 1}, nextP = {p.x + 1, p.y}, underNextP = {p.x + 1, p.y - 1};
            // ---+
            if (underP.y == grid.height) {
                printf("---");
            } else if (getInGrid(grid, p) == obstacle || getInGrid(grid, underP) == obstacle) {
                printf("---");
            } else {
                printf("   ");
            }
            
            if (underP.y == grid.height || nextP.x == grid.width) {
                printf("+");
            } else if (getInGrid(grid, p) == obstacle ||
                       getInGrid(grid, underP) == obstacle ||
                       getInGrid(grid, nextP) == obstacle ||
                       getInGrid(grid, underNextP) == obstacle) {
                printf("+");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
}
