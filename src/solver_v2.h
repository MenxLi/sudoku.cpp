#pragma once

#include "board.h"
#include "config.h"
#include "solver.h"
#include "util.h"
#include <cstring>
#include <memory>

struct SolverV2_config{
    bool use_guess;
    bool deterministic_guess;
    bool heuristic_guess;
    bool use_double;
    bool reverse_guess;

    SolverV2_config& operator=(const SolverV2_config& other){
        load(other);
        return *this;
    }

    void load(const SolverV2_config& other){
        use_guess = other.use_guess;
        deterministic_guess = other.deterministic_guess;
        heuristic_guess = other.heuristic_guess;
        use_double = other.use_double;
        reverse_guess = other.reverse_guess;
    }
};

struct FillState{
    unsigned int count[CANDIDATE_SIZE] = {0};
    bool row[BOARD_SIZE][CANDIDATE_SIZE] = {{0}};
    bool col[BOARD_SIZE][CANDIDATE_SIZE] = {{0}};
    bool grid[GRID_SIZE][GRID_SIZE][CANDIDATE_SIZE] = {{{0}}};
    unsigned int visited_double_combinations[CELL_COUNT][CELL_COUNT] = {{0}};

    void load(const FillState& other){
        std::memcpy (count, other.count, sizeof(count));
        std::memcpy (row, other.row, sizeof(row));
        std::memcpy (col, other.col, sizeof(col));
        std::memcpy (grid, other.grid, sizeof(grid));
        std::memcpy (visited_double_combinations, other.visited_double_combinations, sizeof(visited_double_combinations));
    }
};

class SolverV2 : public Solver
{
public:
    SolverV2(const Board& board);
    SolverV2(SolverV2& other);
    void init_states();

    bool step();
    SolverV2_config& config();
    OpState step_by_naked_single();
    OpState step_by_hidden_single(UnitType unit_type);
    OpState step_by_guess();

    // set the value of a cell, and propagate the value to change the states
    OpState fill_propagate(unsigned int row, unsigned int col, val_t value);

private:
    // SolverV2_config m_config;

    // place them in the heap to avoid stack overflow
    std::unique_ptr<SolverV2_config> m_config;
    std::unique_ptr<CandidateBoard> m_candidates;
    std::unique_ptr<FillState> m_fill_state;

    OpState update_by_naked_single(unsigned int row, unsigned int col);
    OpState update_by_hidden_single(val_t value, UnitType unit_type);

    // handles implicit value determination
    // i.e. if a sub-row/col in a grid has multiple candidates for a value,
    // but can uniquely determine the value based on the row/col 
    // (e.g. 57, 75, 375 appears in one row/col of a grid, determins 7 and 5 must be in the same row/col)
    // then we can remove the other candidates from the same total-row/col
    OpState refine_candidates_by_naked_double(UnitType unit_type);
    OpState refine_candidates_by_hidden_double(UnitType unit_type);     // hidden double is a superset of naked double
};
