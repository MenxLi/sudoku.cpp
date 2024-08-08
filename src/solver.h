#pragma once
#include "cell.h"
#include "board.h"

struct IterationCounter
{
    unsigned int current;
    unsigned int limit;
};

class Solver
{
public:
    Solver();
    Solver(Board& board);
    void set_board(Board& board);
    virtual ~Solver() = default;
    virtual bool step() = 0;
    bool solve(unsigned int max_iterations = 1024, bool verbose = false);
    Cell& cell(int row, int col);
    Cell& cell(const Coord& coord);
    Board& board();
    IterationCounter& iteration_counter();
protected:
    IterationCounter m_iteration_counter;
    std::unique_ptr<Cell> m_cells[BOARD_SIZE][BOARD_SIZE];
    Board* m_board_ptr;
};