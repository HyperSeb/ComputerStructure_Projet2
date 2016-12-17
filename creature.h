
// return a random gene
int randomGene();

// show the best creature's movements on the terminal
void showBest(Grid grid, int* genome, int genomeLength);

// computes the score of the creature
double computeScore(Grid grid, int* genome, int genomeLength);

// display the genome on stdout, indicating gene at `geneIndex`
void displayGenome(int* genome, int genomeLength, int geneIndex);

// display the grid on stdout
void displayGrid(Grid grid, Position creaturePosition);
