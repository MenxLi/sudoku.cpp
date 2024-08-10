#pragma once
#include "config.h"
#include "board.h"
#include "indexer.hpp"

struct IterationCounter
{
    // use long to avoid overflow
    unsigned long current;
    unsigned long limit;
};

class Solver
{
public:
    // https://stackoverflow.com/a/53705993/6775765
    inline static Indexer<GRID_SIZE> indexer;

    Solver(Board& board);
    virtual ~Solver() = default;
    virtual bool step() = 0;
    bool solve(bool verbose = false);
    Board& board();
    IterationCounter& iteration_counter();
protected:
    IterationCounter m_iteration_counter;
    Board* m_board_ptr;
};