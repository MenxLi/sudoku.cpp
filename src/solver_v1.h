#pragma once

#include "board.h"
#include "config.h"
#include "solver.h"
#include <memory>

#define CANDIDATE_SIZE BOARD_SIZE

class SolverV1 : public Solver
{
public:
    SolverV1(Board& board);
    bool step();
    bool step_by_candidate();
    bool step_by_crossover();
    bool step_by_guess();

private:
    // cadidate refers to the possible values for a cell, 
    // based on the values of other cells in the same row, column, and grid
    val_t m_candidates[BOARD_SIZE][BOARD_SIZE][CANDIDATE_SIZE];
    void update_candidate_for(int row, int col);
    void update_candidates();
    void clear_candidates();
    bool update_value_for(int row, int col);
    bool update_values();

    unsigned char m_cross_map[BOARD_SIZE][BOARD_SIZE];
    unsigned char m_cross_map_row[BOARD_SIZE][BOARD_SIZE];
    unsigned char m_cross_map_col[BOARD_SIZE][BOARD_SIZE];
    bool update_by_cross(val_t value);
    void clear_cross_map();

    // this is for trail and error approach
    std::tuple<std::unique_ptr<SolverV1>, std::unique_ptr<Board>> fork();
    bool update_by_guess();
};
