#pragma once

#include "board.h"
#include "config.h"
#include "solver_base.h"
#include "util.h"
#include "parser.hpp"
#include <cstring>
#include <memory>

struct Solver_config{
    bool use_guess;
    bool deterministic_guess;
    bool heuristic_guess;
    bool use_double;
    bool reverse_guess;
    static std::unique_ptr<Solver_config> from_env() {
        return std::make_unique<Solver_config>(
            Solver_config{
                parser::parse_env("SOLVER_USE_GUESS", true),
                parser::parse_env("SOLVER_DETERMINISTIC_GUESS", false),
                parser::parse_env("SOLVER_HEURISTIC_GUESS", true),
                parser::parse_env("SOLVER_USE_DOUBLE", false),
                false
            }
        );
    }
};

struct FillState{
    unsigned int count[CANDIDATE_SIZE] = {0};
    bool row[BOARD_SIZE][CANDIDATE_SIZE] = {{0}};
    bool col[BOARD_SIZE][CANDIDATE_SIZE] = {{0}};
    bool grid[GRID_SIZE][GRID_SIZE][CANDIDATE_SIZE] = {{{0}}};
    unsigned int visited_double_combinations[CELL_COUNT][CELL_COUNT] = {{0}};
};

class Solver : public SolverBase
{
public:
    Solver(const Board& board);
    Solver(Solver& other);
    void init_states() noexcept;

    bool step();
    OpState step_by_naked_single();
    OpState step_by_hidden_single(UnitType unit_type);
    OpState step_by_guess();

    // set the value of a cell, and propagate the value to change the states
    OpState fill_propagate(unsigned int row, unsigned int col, val_t value) noexcept;

    inline Solver_config& config() { return *m_config; }
private:
    // Solver_config m_config;

    // place them in the heap to avoid stack overflow
    std::unique_ptr<Solver_config> m_config;
    std::unique_ptr<CandidateBoard> m_candidates;
    std::unique_ptr<FillState> m_fill_state;

    OpState update_by_naked_single(unsigned int row, unsigned int col);
    OpState update_by_hidden_single(val_t value, UnitType unit_type);

    // handles implicit value determination (subsets)
    // i.e. if a sub-row/col in a grid has multiple candidates for a value,
    // but can uniquely determine the value based on the row/col 
    // (e.g. 57, 75, 375 appears in one row/col of a grid, determins 7 and 5 must be in the same row/col)
    // then we can remove the other candidates from the same total-row/col
    OpState refine_candidates_by_naked_double(UnitType unit_type);
    OpState refine_candidates_by_hidden_double(UnitType unit_type);     // hidden double is a superset of naked double
};
