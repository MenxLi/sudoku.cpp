#include "board.h"
#include <random>

namespace gen
{

/**
Random number generator with uniform distribution. 
Inclusive range [S, E]
 */
template <const unsigned int S, unsigned int E>
class UniformIntDist{
    std::random_device dev;
    std::mt19937 rng;
    std::uniform_int_distribution<std::mt19937::result_type> dist;
public:
    UniformIntDist( unsigned int seed = std::random_device{}() ): 
        rng(seed), 
        dist(S, E) 
    {}
    inline int sample(){ return dist(rng); }
};

/*
a meta board is a board that contains the simplist form of a filled board
generated with fixed strategy.
*/
Board get_meta_board();

/* Apply random equivalence transformation to the board */
void apply_random_transform(Board& board, unsigned int n_repeats);

/*
Get a list of valid candidates for a cell in the board, 
based on the current state of it's neighbors
*/
std::vector<val_t> get_candidates(Board& board, int row, int col);


/*
Fill the board with valid values, from transforming a meta board, 
this is a faster way to fill the board, but with less randomness
*/
void fill_board_naive(Board& board);

/* 
Fill the board with valid values, using backtracking 
Should make sure the bord is empty before calling this function
*/
void fill_board_backtrack(Board& board);

enum class FillStrategy
{
    NAIVE, 
    BACKTRACK
};

void fill_board(Board& board, FillStrategy strategy);

}
