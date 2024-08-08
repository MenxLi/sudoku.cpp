
#include "solver.h"
#include "board.h"

Solver::Solver()
{
    m_iteration_counter.current = 0;
    m_iteration_counter.limit = 0;

    m_board_ptr = nullptr;
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            m_cells[row][col] = nullptr;
        }
    }
};

Solver::Solver(Board& board)
{
    m_iteration_counter.current = 0;
    m_iteration_counter.limit = 0;
    set_board(board);
};

void Solver::set_board(Board& board)
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

bool Solver::solve(unsigned int max_iterations, bool verbose){

    m_iteration_counter.limit = max_iterations;

    while (m_iteration_counter.current < max_iterations && !board().is_solved()){
    
        if(verbose)
        {
            std::cout << "Iteration " << m_iteration_counter.current << std::endl;
            std::cout << board() << std::endl;
        }

        bool step_result = step();

        if (!step_result) break;

        m_iteration_counter.current++;
    }

    return board().is_solved();
};

IterationCounter& Solver::iteration_counter()
{
    return m_iteration_counter;
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