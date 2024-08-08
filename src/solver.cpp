
#include "solver.h"
#include "board.h"

Solver::Solver()
{
    m_iteration_counter.current = 0;
    m_iteration_counter.limit = 0;
    m_view_ptr = nullptr;
};

Solver::Solver(Board& board)
{
    m_iteration_counter.current = 0;
    m_iteration_counter.limit = 0;
    set_board(board);
};

void Solver::set_board(Board& board)
{
    m_view_ptr.reset(new CellView(board));
};

bool Solver::solve(unsigned int max_iterations, bool verbose){

    m_iteration_counter.current = 0;
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
    return m_view_ptr->get(row, col);
};

Cell& Solver::cell(const Coord& coord)
{
    return m_view_ptr->get(coord);
};

Board& Solver::board()
{
    return m_view_ptr->board();
};