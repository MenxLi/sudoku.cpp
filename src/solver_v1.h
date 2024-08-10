#pragma once

#include "board.h"
#include "config.h"
#include "solver.h"
#include <memory>

class SolverV1 : public Solver
{
public:
    SolverV1(Board& board);
    bool step();
    bool step_by_candidate();
    bool step_by_crossover();
    bool step_by_guess();

private:
    // // cadidate refers to the possible values for a cell, 
    // // based on the values of other cells in the same row, column, and grid
    // val_t m_candidates[BOARD_SIZE][BOARD_SIZE][CANDIDATE_SIZE];
    CandidateBoard m_candidates;

    // calculated candidates for each cell based on current board
    void update_candidate_for(int row, int col);
    // number of candidates for each cell, will call reset_candidates()
    void update_candidates();
    // handles implicit value determination
    void refine_candidates();
    void reset_candidates();
    bool update_value_for(int row, int col);
    bool update_values();

    unsigned int m_cross_map[BOARD_SIZE][BOARD_SIZE];
    unsigned int m_cross_row[BOARD_SIZE];       // maybe move to the function...
    unsigned int m_cross_col[BOARD_SIZE];
    bool update_by_cross(val_t value);
    void clear_cross_map();

    // this is for trail and error approach
    std::tuple<std::unique_ptr<SolverV1>, std::unique_ptr<Board>> fork();
    bool update_by_guess();
};
