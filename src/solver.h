#pragma once
#include "cell.h"
#include "board.h"

class Solver
{
public:
    static const unsigned int MAX_ITERATION = 2048;
    Solver(Board& board);
    virtual bool step() = 0;
    bool solve(bool verbose = false);
    Cell& cell(int row, int col);
    Cell& cell(const Coord& coord);
    Board& board();
protected:
    std::unique_ptr<Cell> m_cells[BOARD_SIZE][BOARD_SIZE];
    Board* m_board_ptr;
};