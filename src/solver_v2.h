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
    unsigned int m_row_value_state[BOARD_SIZE][CANDIDATE_SIZE] = {{0}};
    unsigned int m_col_value_state[BOARD_SIZE][CANDIDATE_SIZE] = {{0}};
    unsigned int m_grid_value_state[GRID_SIZE][GRID_SIZE][CANDIDATE_SIZE] = {{{0}}};

    OpState update_by_naked_single(int row, int col);
    OpState update_by_hidden_single(val_t value, UnitType unit_type);

    // handles implicit value determination
    // i.e. if a sub-row/col in a grid has multiple candidates for a value,
    // but can uniquely determine the value based on the row/col 
    // (e.g. 57, 75, 375 appears in one row/col of a grid, determins 7 and 5 must be in the same row/col)
    // then we can remove the other candidates from the same total-row/col
    OpState refine_candidates_by_naked_double(UnitType unit_type);
    OpState refine_candidates_by_hidden_double(UnitType unit_type);     // hidden double is a superset of naked double
    uint8_t m_visited_double_combinations[CELL_COUNT][CELL_COUNT] = {{0}};

    // this is for trail and error approach
    SolverV2 fork();
};
