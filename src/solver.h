#pragma once

#include "solver_base.hpp"
#include "board.h"
#include "config.h"
#include "util.h"
#include "parser.hpp"
#include <cstring>
#include <memory>

// #define DEBUG_PRINT(x) std::cout << x << std::endl;
#define DEBUG_PRINT(x)

struct Solver_config{
    bool use_guess;
    bool deterministic_guess;
    bool heuristic_guess;
    bool use_double;
    bool use_xwing;
    bool reverse_guess;
    static Solver_config* new_from_env() {
        return new Solver_config{
            parser::parse_env("SOLVER_USE_GUESS", true),
            parser::parse_env("SOLVER_DETERMINISTIC_GUESS", false),
            parser::parse_env("SOLVER_HEURISTIC_GUESS", true),
            parser::parse_env("SOLVER_USE_DOUBLE", false),
            parser::parse_env("SOLVER_USE_XWING", false), 
            false
        };
    }
};

class FillState{
    inline static Indexer indexer;

    std::bitset<CELL_COUNT> solved;         // solved cells, default to all 0s

    Cell row[BOARD_SIZE];                   // use up is 0, bitmask default to all 1s
    Cell col[BOARD_SIZE];
    Cell grid[GRID_SIZE][GRID_SIZE];

public:
    /*
    This is used to update the filled state of the board,
    if constraints are violated,
    e.g. a value is filled more than once in a row, column, or grid
    it will return false [When this happens, the fill object will be in a broken state]
    */
    bool on_fill(unsigned int row, unsigned int col, UniCell value_cell);

    inline bool is_cell_solved(unsigned int row, unsigned int col) const {
        return this->solved.test(indexer.coord_offset_lookup[row][col]);
    }
    inline bool is_cell_solved(unsigned int offset) const {
        return this->solved.test(offset);
    }

    // inline bool is_value_useup(val_t value) const {
    //     unsigned int v_idx = static_cast<unsigned int>(value) - 1;
    //     return this->count[v_idx] == BOARD_SIZE;
    // }

    inline unsigned int get_value_count(val_t value) const {
        unsigned int v_idx = static_cast<unsigned int>(value) - 1;
        unsigned int total = 0;
        for (unsigned int i = 0; i < UNIT_SIZE; i++){
            if (!this->row[i].test(v_idx)) total++;
        }
        return total;
    }

    inline bool is_in_row(unsigned int row, val_t value) const {
        unsigned int v_idx = static_cast<unsigned int>(value) - 1;
        return !this->row[row].test(v_idx);
    }
    inline bool is_in_row(unsigned int row, Cell value_cell) const {
        return (this->row[row] & value_cell).is_empty();
    }

    inline bool is_in_col(unsigned int col, val_t value) const {
        unsigned int v_idx = static_cast<unsigned int>(value) - 1;
        return !this->col[col].test(v_idx);
    }
    inline bool is_in_col(unsigned int col, Cell value_cell) const {
        return (this->col[col] & value_cell).is_empty();
    }

    inline bool is_in_grid(unsigned int grid_row, unsigned int grid_col, val_t value) const {
        unsigned int v_idx = static_cast<unsigned int>(value) - 1;
        return !this->grid[grid_row][grid_col].test(v_idx);
    }
    inline bool is_in_grid(unsigned int grid_row, unsigned int grid_col, Cell value_cell) const {
        return (this->grid[grid_row][grid_col] & value_cell).is_empty();
    }
};

class Solver : public SolverBase
{
public:
    Solver(const Board& board);
    void init_states();

    bool step() override;

    /*
    This determines the value of a cell if
    there is only one candidate left in the cell
    */
    OpState step_by_naked_single() noexcept ;

    /*
    This determines the value of a cell if 
    it is the only cell in the row/col/grid that can have a certain value
    */
    OpState step_by_hidden_single(UnitType unit_type) noexcept;

    /* Using X-Wing to update the candidates */
    OpStateTentative step_by_xwing() noexcept;

    /* Using naked/hidden double to update the candidates */
    OpStateTentative step_by_double() noexcept;

    OpState step_by_guess() noexcept;

    // set the value of a cell, and propagate the value to change the states
    OpState fill_propagate(unsigned int row, unsigned int col, UniCell cell) noexcept;

    inline Solver_config& config() const { return *m_config; }

private:
    // copy constructor shares config, make it private
    explicit Solver(Solver& other) noexcept; 
    std::shared_ptr<Solver_config> m_config;

    // place them in the heap to avoid stack overflow
    std::unique_ptr<FillState> m_fstate;

    OpState update_by_naked_single(unsigned int row, unsigned int col);
    OpState update_by_hidden_single(val_t value, UnitType unit_type);

    std::pair<Coord, std::vector<val_t>> find_best_guess() noexcept;
};
