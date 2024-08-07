
#include "solver.h"
#include "board.h"

Solver::Solver(Board& board)
{
    m_board_ptr = &board;
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            m_cells[row][col] = std::make_unique<Cell>(board, Coord{row, col});
        }
    }
};

bool Solver::solve(bool verbose){
    for (unsigned int i = 0; i < MAX_ITERATION; i++)
    {
        bool step_result = step();

        if(verbose)
        {
            std::cout << "Iteration " << i << std::endl;
            std::cout << board() << std::endl;
        }

        if (!step_result)
        {
            // check if the board is solved
            for (int row = 0; row < BOARD_SIZE; row++)
            {
                for (int col = 0; col < BOARD_SIZE; col++)
                {
                    if (cell(row, col).value() == 0)
                    {
                        return false;
                    }
                }
            }
        }

    }
    return board().is_solved();
};

Cell& Solver::cell(int row, int col)
{
    return *m_cells[row][col];
};

Cell& Solver::cell(const Coord& coord)
{
    return *m_cells[coord.row][coord.col];
};

Board& Solver::board()
{
    return *m_board_ptr;
};