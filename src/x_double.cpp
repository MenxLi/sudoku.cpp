#include "board.h"
#include "config.h"
#include "solver_base.hpp"
#include "solver.h"
#include <array>

// #define DEBUG_PRINT_D(x) std::cout << x << std::endl;
#define DEBUG_PRINT_D(x)

OpStateTentative update_by_naked_double(
    Board& board, 
    const unsigned int unit_offsets[], 
    unsigned int v1, unsigned int v2
){
    Cell v_mask = Cell(v1) | Cell(v2);

    std::vector<unsigned int> found_offsets;
    std::vector<unsigned int> not_found_offsets;

    for (unsigned int i = 0; i < UNIT_SIZE; i++){
        auto offset = unit_offsets[i];
        if (board.get(offset) == v_mask){
            found_offsets.push_back(offset);
        }
        else {
            not_found_offsets.push_back(offset);
        }
        if (found_offsets.size() > 2) { return OpStateTentative::VIOLATION; }
    }
    if (found_offsets.size() < 2) { return OpStateTentative::FAIL; } 

    // we have a naked double, eliminate the candidates in the other cells
    for (auto nf: not_found_offsets){
        auto& cell = board.get(nf);
        cell &= ~v_mask; // eliminate the candidates
    }

    return OpStateTentative::MAYBE_SUCCESS;
};

OpStateTentative update_by_hidden_double(
    Board& board, 
    const unsigned int unit_offsets[], 
    unsigned int v1, unsigned int v2
){
    Cell v_mask = Cell(v1) | Cell(v2);

    std::vector<unsigned int> found_offsets;

    for (unsigned int i = 0; i < UNIT_SIZE; i++){
        auto offset = unit_offsets[i];
        if (!(board.get(offset) & v_mask).is_empty()){
            found_offsets.push_back(offset);
        }
        if (found_offsets.size() > 2) { return OpStateTentative::FAIL; }
    }
    if (found_offsets.size() < 2) { return OpStateTentative::VIOLATION; }

    DEBUG_PRINT_D("Solver::update_by_hidden_double() - v1: " + std::to_string(v1) + ", v2: " + std::to_string(v2) + ", v_mask: " + v_mask.bitmask().to_string());
    DEBUG_PRINT_D("Solver::update_by_hidden_double() - found double at offsets: " + 
        std::to_string(found_offsets[0]) + ", " + std::to_string(found_offsets[1]) 
        + " (" << board.indexer.offset_coord_lookup[found_offsets[0]][0] << ", " << std::to_string(board.indexer.offset_coord_lookup[found_offsets[0]][1]) + "), " 
        + "(" << std::to_string(board.indexer.offset_coord_lookup[found_offsets[1]][0]) + ", " + std::to_string(board.indexer.offset_coord_lookup[found_offsets[1]][1]) + ")");
    DEBUG_PRINT_D(board)
    DEBUG_PRINT_D(board.to_bitstring(",", "\n", "'"))

    // we have a hidden double, eliminate the candidates in those cells
    for (auto fo: found_offsets){
        auto& cell = board.get(fo);
        cell = v_mask;
    }

    DEBUG_PRINT_D("Solver::update_by_hidden_double() - elimination completed, updated: " + std::to_string(updated));
    DEBUG_PRINT_D(board.to_bitstring(",", "\n", "'"))

    return OpStateTentative::MAYBE_SUCCESS;
};

OpStateTentative Solver::step_by_double() noexcept {
    bool maybe_updated = false;

    for (auto [v1, v2]: indexer.subvalue_combinations_2){
        // if (m_fstate->is_value_useup(v1) || m_fstate->is_value_useup(v2)){
        //     continue; // skip if any value is already used up
        // }


        for (auto unit_type : {UnitType::ROW, UnitType::COL, UnitType::GRID}){
            auto unit_offsets = [&unit_type](unsigned int i)-> const unsigned int* {
                switch (unit_type){
                    case UnitType::ROW: return indexer.row_index[i];
                    case UnitType::COL: return indexer.col_index[i];
                    case UnitType::GRID: return indexer.grid_index[i / GRID_SIZE][i % GRID_SIZE];
                    default: throw std::runtime_error("Invalid unit type");
                }
            };

            unsigned int i_unit = 0;
            while (i_unit < UNIT_SIZE){
                if (unit_type == UnitType::GRID){
                    unsigned int g_r = i_unit / GRID_SIZE;
                    unsigned int g_c = i_unit % GRID_SIZE;
                    if (m_fstate->is_in_grid(g_r, g_c, v1) || m_fstate->is_in_grid(g_r, g_c, v2)){
                        i_unit++;
                        continue; // skip if any value is already used up
                    }
                }
                if (unit_type == UnitType::ROW){
                    if (m_fstate->is_in_row(i_unit, v1) || m_fstate->is_in_row(i_unit, v2)){
                        i_unit++;
                        continue; // skip if any value is already used up
                    }
                }
                if (unit_type == UnitType::COL){
                    if (m_fstate->is_in_col(i_unit, v1) || m_fstate->is_in_col(i_unit, v2)){
                        i_unit++;
                        continue; // skip if any value is already used up
                    }
                }

                auto unit_offsets_ptr = unit_offsets(i_unit);
                OpStateTentative state = update_by_naked_double(board(), unit_offsets_ptr, v1, v2);
                if (state == OpStateTentative::VIOLATION){
                    return state; // violation, return immediately
                }
                state == OpStateTentative::MAYBE_SUCCESS ? maybe_updated = true : maybe_updated = false;

                OpStateTentative state_hidden = update_by_hidden_double(board(), unit_offsets_ptr, v1, v2);
                if (state_hidden == OpStateTentative::VIOLATION){
                    return state_hidden; // violation, return immediately
                }
                state_hidden == OpStateTentative::MAYBE_SUCCESS ? maybe_updated = true : maybe_updated = false;

                i_unit++;

            } // end of unit type loop
        }
    }

    return maybe_updated ? OpStateTentative::MAYBE_SUCCESS : OpStateTentative::FAIL;
}