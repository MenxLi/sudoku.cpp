#pragma once

#include "board.h"
#include "config.h"
#include "solver.h"
#include <memory>

class SolverV2 : public Solver
{
public:
    SolverV2(const Board& board);
    SolverV2(SolverV2& other);

    bool step();
    bool step_by_candidate();
    bool step_by_crossover();
    bool step_by_guess();

    // set the value of a cell, and propagate the value to change the candidates
    void fill_propagate(unsigned int row, unsigned int col, val_t value);

private:
    CandidateBoard m_candidates;
    unsigned int m_cross_map[CANDIDATE_SIZE][BOARD_SIZE][BOARD_SIZE];   // 0 means available, 1 means occupied
    unsigned int m_filled_count[CANDIDATE_SIZE] = {0};

    void init_candidates_and_count();
    void init_cross_map();

    // handles implicit value determination
    void refine_candidates();

    bool update_value_for(int row, int col);
    bool update_by_cross(int row, int col);
    bool update_by_cross(val_t value);

    // this is for trail and error approach
    // std::tuple<std::unique_ptr<SolverV1>, std::unique_ptr<Board>> fork();
    SolverV2 fork();
};
