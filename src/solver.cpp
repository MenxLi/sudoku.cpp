
#include "solver.h"
#include "board.h"
#include "config.h"

#ifdef PYBIND11_BUILD
#include <pybind11/pybind11.h>
namespace py = pybind11;
#endif

Solver::Solver(const Board& board)
{
    indexer.init();
    m_board.load_data(board);
    m_iteration_counter.n_guesses = 0;
    m_iteration_counter.current = 0;
    m_iteration_counter.limit = MAX_ITER;
};

bool Solver::solve(bool verbose){

    // std::cout << "starting with iteration: " << m_iteration_counter.current << std::endl;
    while (m_iteration_counter.current < m_iteration_counter.limit && !board().is_filled()){
    
        if(verbose)
        {
            std::cout << "Iteration " << m_iteration_counter.current << std::endl;
            std::cout << board() << std::endl;
        }

        bool step_result = step();

        #ifdef PYBIND11_BUILD
        if (PyErr_CheckSignals() != 0){
            throw py::error_already_set();
        }
        #endif

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
    return m_board;
};