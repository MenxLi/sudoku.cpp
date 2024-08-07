#pragma once

#include "config.h"
#include "solver.h"
#include <_types/_uint8_t.h>

#define CANDIDATE_SIZE BOARD_SIZE

class SolverV1 : public Solver
{
public:
    SolverV1(Board& board);
    bool step();
    bool step_by_candidate();
    bool step_by_crossover();

private:
    val_t m_candidates[BOARD_SIZE][BOARD_SIZE][CANDIDATE_SIZE];
    void update_candidate_for(int row, int col);
    void update_candidates();
    void clear_candidates();
    bool update_value_for(int row, int col);
    bool update_values();

    bool m_cross_map[BOARD_SIZE][BOARD_SIZE];
    bool update_by_cross(val_t value);
};