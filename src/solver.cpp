
#include "solver.h"
#include "board.h"
#include "config.h"

Solver::Solver(Board& board)
{
    indexer.init();
    m_iteration_counter.current = 0;
    m_iteration_counter.limit = MAX_ITER;
    set_board(board);
};

void Solver::set_board(Board &board)
{
    m_board_ptr = &board;
};

bool Solver::solve(unsigned long max_iterations, bool verbose){

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


Board& Solver::board()
{
    return *m_board_ptr;
};