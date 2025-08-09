#pragma once
#include "config.h"
#include "board.h"
#include "indexer.h"
#include <memory>

#ifdef PYBIND11_BUILD
#include <pybind11/pybind11.h>
namespace py = pybind11;
#endif


struct IterationCounter
{
    // use long to avoid overflow
    unsigned long current;
    unsigned long limit;
    unsigned long n_guesses;

    IterationCounter(): current(0), limit(MAX_ITER), n_guesses(0) {};
};

class SolverBase
{
public:
    // https://stackoverflow.com/a/53705993/6775765
    inline static Indexer indexer;

    explicit SolverBase(const Board& board);
    virtual ~SolverBase() = default;

    virtual bool step() = 0;
    bool solve(bool verbose = false);
    inline Board& board() { return *m_board; }
    inline IterationCounter& iteration_counter(){ return *m_iteration_counter; }
protected:
    explicit SolverBase(const SolverBase& other);
    std::unique_ptr<IterationCounter> m_iteration_counter;
    std::unique_ptr<Board> m_board;
};


// impl -----------------------------------------------

inline SolverBase::SolverBase(const Board& board): 
    m_iteration_counter(new IterationCounter()), 
    m_board(new Board(board)) 
{};

inline SolverBase::SolverBase(const SolverBase& other): 
    m_iteration_counter{std::make_unique<IterationCounter>(*other.m_iteration_counter)},
    m_board{std::make_unique<Board>(*other.m_board)}
{};

inline bool SolverBase::solve(bool verbose){

    // std::cout << "starting with iteration: " << m_iteration_counter.current << std::endl;
    while (m_iteration_counter->current < m_iteration_counter->limit && !board().is_filled()){
    
        if(verbose)
        {
            std::cout << "Iteration " << m_iteration_counter->current << std::endl;
            std::cout << board() << std::endl;
        }

        bool step_result = step();

        #ifdef PYBIND11_BUILD
        if (PyErr_CheckSignals() != 0){
            throw py::error_already_set();
        }
        #endif

        if (!step_result) break;

        m_iteration_counter->current++;
    }

    return board().is_solved();
};