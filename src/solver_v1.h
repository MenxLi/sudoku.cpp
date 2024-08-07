#pragma once

#include "config.h"
#include "solver.h"

#define CANDIDATE_SIZE BOARD_SIZE

class SolverV1 : public Solver
{
public:
    SolverV1(Board& board);
    bool step();

private:
    val_t m_candidates[BOARD_SIZE][BOARD_SIZE][CANDIDATE_SIZE];
    void update_candidate_for(int row, int col);
    void update_candidates();
    void clear_candidates();
    bool update_value_for(int row, int col);
    bool update_values();
};