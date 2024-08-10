
#include "solver.h"
#include "board.h"
#include "config.h"

Solver::Solver(Board& board)
{
    indexer.init();
    m_board_ptr = &board;
    m_iteration_counter.current = 0;
    m_iteration_counter.limit = MAX_ITER;
};

bool Solver::solve(bool verbose){

    // std::cout << "starting with iteration: " << m_iteration_counter.current << std::endl;
    while (m_iteration_counter.current < m_iteration_counter.limit && !board().is_solved()){
    
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