#pragma once

#include "board.h"
#include "config.h"
#include "solver.h"
#include <memory>

enum class UnitType{
    GRID,
    ROW,
    COL
};


class SolverV2 : public Solver
{
public:
    SolverV2(const Board& board);
    SolverV2(SolverV2& other);

    bool step();
    bool step_by_only_candidate();
    bool step_by_implicit_only_candidate(UnitType unit_type);
    bool step_by_guess();

    // set the value of a cell, and propagate the value to change the candidates
    void fill_propagate(unsigned int row, unsigned int col, val_t value);

private:
    CandidateBoard m_candidates;

    // global filled count for each value
    unsigned int m_filled_count[CANDIDATE_SIZE] = {0};

    // unit filled count for each value
    unsigned int m_row_value_state[BOARD_SIZE][CANDIDATE_SIZE] = {0};
    unsigned int m_col_value_state[BOARD_SIZE][CANDIDATE_SIZE] = {0};
    unsigned int m_grid_value_state[GRID_SIZE][GRID_SIZE][CANDIDATE_SIZE] = {0};

    void init_candidates_and_count();
    bool update_if_only_candidate(int row, int col);
    bool update_for_implicit_only_candidates(val_t value, UnitType unit_type);

    // handles implicit value determination
    void refine_candidates();

    // this is for trail and error approach
    // std::tuple<std::unique_ptr<SolverV1>, std::unique_ptr<Board>> fork();
    SolverV2 fork();
};
