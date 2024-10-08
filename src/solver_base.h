#pragma once
#include "config.h"
#include "board.h"
#include "indexer.h"
#include <memory>

struct IterationCounter
{
    // use long to avoid overflow
    unsigned long current;
    unsigned long limit;
    unsigned long n_guesses;

    IterationCounter(): current(0), limit(MAX_ITER), n_guesses(0) {};

    void load(const IterationCounter& other)
    {
        current = other.current;
        limit = other.limit;
        n_guesses = other.n_guesses;
    }
};

class SolverBase
{
public:
    // https://stackoverflow.com/a/53705993/6775765
    inline static Indexer indexer;

    SolverBase(const Board& board);
    virtual ~SolverBase() = default;
    virtual bool step() = 0;
    bool solve(bool verbose = false);
    Board& board();
    IterationCounter& iteration_counter();
protected:
    std::unique_ptr<IterationCounter> m_iteration_counter;
    std::unique_ptr<Board> m_board;
};