#include "board.h"
#include "config.h"
#include "solver_base.hpp"
#include "solver.h"
#include <memory>

// #define DEBUG_PRINT_X(x) std::cout << x << std::endl;
#define DEBUG_PRINT_X(x)

namespace xwing{

template <unsigned int N>
std::array<unsigned int, N-2> get_ex_iota(unsigned ex0, unsigned ex1) {
    std::array<unsigned int, N-2> v;
    unsigned int idx = 0;
    for (unsigned int i = 0; i < N; ++i){
        if (i == ex0 || i == ex1) continue;
        v[idx++] = i;
    }
    return v;
}

OpStateTentative run_xwing_on(
    Board& board, 
    unsigned int r1, unsigned int r2, unsigned int c1, unsigned int c2
) {
    auto& indexer = Board::indexer;
    auto possible_values = 
        board.get(r1, c1) &
        board.get(r1, c2) &
        board.get(r2, c1) &
        board.get(r2, c2);
    
    if (possible_values.is_empty()){
        return OpStateTentative::FAIL;
    }

    auto r_range = get_ex_iota<indexer.N>(r1, r2);
    auto c_range = get_ex_iota<indexer.N>(c1, c2);

    // check other cells in the rows and columns
    Cell col_to_elim = possible_values;
    for (auto c: c_range){
        auto& cell_r1 = board.get(indexer.row_index[r1][c]);
        auto& cell_r2 = board.get(indexer.row_index[r2][c]);
        col_to_elim &= ~(cell_r1 | cell_r2);
    }
    Cell row_to_elim = possible_values;
    for (auto r: r_range){
        auto& cell_c1 = board.get(indexer.col_index[c1][r]);
        auto& cell_c2 = board.get(indexer.col_index[c2][r]);
        row_to_elim &= ~(cell_c1 | cell_c2);
    }

    if (col_to_elim.is_empty() && row_to_elim.is_empty()){ return OpStateTentative::FAIL; }
    // if (col_to_elim == row_to_elim){ return OpState::FAIL; }

    auto cell = [&board](unsigned int r, unsigned int c) -> Cell&{ return board.get(Board::indexer.row_index[r][c]); };

    // eliminate the candidates in the rows and columns
    if (!col_to_elim.is_empty()){
        DEBUG_PRINT_X("X-Wing found at rows (" << r1 << ", " << r2 << ") and cols (" << c1 << ", " << c2 << "), eliminating " << col_to_elim.bitmask())
        DEBUG_PRINT_X(board.to_bitstring(",", "\n", "'"))

        for (auto i: r_range){
            cell(i, c1) &= ~col_to_elim;
            if (cell(i, c1).is_empty()){ return OpStateTentative::VIOLATION; }

            cell(i, c2) &= ~col_to_elim;
            if (cell(i, c2).is_empty()){ return OpStateTentative::VIOLATION; }
        }
    }

    if (!row_to_elim.is_empty()){
        DEBUG_PRINT_X("X-Wing found at rows (" << r1 << ", " << r2 << ") and cols (" << c1 << ", " << c2 << "), eliminating " << row_to_elim.bitmask())
        DEBUG_PRINT_X(board.to_bitstring(",", "\n", "'"))

        for (auto i: c_range){
            cell(r1, i) &= ~row_to_elim;
            if (cell(r1, i).is_empty()){ return OpStateTentative::VIOLATION; }

            cell(r2, i) &= ~row_to_elim;
            if (cell(r2, i).is_empty()){ return OpStateTentative::VIOLATION; }
        }
    }

    DEBUG_PRINT_X("X-Wing elimination completed, board state:\n" << board.to_bitstring(",", "\n", "'"))
    return OpStateTentative::MAYBE_SUCCESS;
}

}

OpStateTentative Solver::step_by_xwing() noexcept {

    OpStateTentative ret = OpStateTentative::FAIL;
    for (auto [r1, r2]: indexer.subunit_combinations_2){
        for (auto [c1, c2]: indexer.subunit_combinations_2){

            if (this->m_fstate->is_cell_solved(r1, c1) || 
                this->m_fstate->is_cell_solved(r1, c2) ||
                this->m_fstate->is_cell_solved(r2, c1) ||
                this->m_fstate->is_cell_solved(r2, c2)){
                continue;
            }

            auto r_ = xwing::run_xwing_on(this->board(), r1, r2, c1, c2);
            if (r_ == OpStateTentative::VIOLATION){ return OpStateTentative::VIOLATION; }
            if (r_ == OpStateTentative::MAYBE_SUCCESS){ ret = OpStateTentative::MAYBE_SUCCESS; }
        }
    }
    return ret;
}