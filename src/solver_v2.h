#pragma once

#include "board.h"
#include "config.h"
#include "indexer.hpp"
#include "solver.h"
#include <memory>


class SolverV2 : public Solver
{
public:
    SolverV2(const Board& board);
    SolverV2(SolverV2& other);
    void init_states();

    bool step();
    OpState step_by_naked_single();
    OpState step_by_hidden_single(UnitType unit_type);
    OpState step_by_guess();

    // set the value of a cell, and propagate the value to change the states
    OpState fill_propagate(unsigned int row, unsigned int col, val_t value);

private:
    CandidateBoard m_candidates;

    // global filled count for each value
    unsigned int m_filled_count[CANDIDATE_SIZE] = {0};

    // unit filled count for each value
    unsigned int m_row_value_state[BOARD_SIZE][CANDIDATE_SIZE] = {0};
    unsigned int m_col_value_state[BOARD_SIZE][CANDIDATE_SIZE] = {0};
    unsigned int m_grid_value_state[GRID_SIZE][GRID_SIZE][CANDIDATE_SIZE] = {0};

    OpState update_by_naked_single(int row, int col);
    OpState update_by_hidden_single(val_t value, UnitType unit_type);

    // handles implicit value determination
    bool refine_candidates(UnitType unit_type);

    // this is for trail and error approach
    SolverV2 fork();
};
