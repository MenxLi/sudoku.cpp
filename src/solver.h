#pragma once
#include "board.h"
#include "indexer.hpp"

struct IterationCounter
{
    unsigned int current;
    unsigned int limit;
};

class Solver
{
public:
    // https://stackoverflow.com/a/53705993/6775765
    inline static Indexer<GRID_SIZE> indexer;

    Solver(Board& board);
    void set_board(Board& board);
    virtual ~Solver() = default;
    virtual bool step() = 0;
    bool solve(unsigned int max_iterations = 1e6, bool verbose = false);
    Board& board();
    IterationCounter& iteration_counter();
protected:
    IterationCounter m_iteration_counter;
    Board* m_board_ptr;
};