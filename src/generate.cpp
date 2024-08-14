#include "generate.h"
#include "board.h"
#include "config.h"
#include "indexer.hpp"
#include "util.h"
#include "solver_v2.h"
#include <random>
#include <memory>
#include <vector>
#include <algorithm>

namespace generate{
    static Indexer<GRID_SIZE> indexer;

    static util::SizedArray<val_t, CANDIDATE_SIZE> get_candidates(Board& board, int row, int col){
        uint8_t candidates[CANDIDATE_SIZE];
        for (unsigned int i = 0; i < CANDIDATE_SIZE; i++){
            candidates[i] = 1;
        }
        for (auto offset: indexer.neighbor_index[row][col]){
            val_t n_value = board.get(offset);
            if (n_value != 0){
                unsigned int v_idx = n_value - 1;
                candidates[v_idx] = 0;
            }
        }
        util::SizedArray<val_t, CANDIDATE_SIZE> result;
        for (unsigned int i = 0; i < CANDIDATE_SIZE; i++){
            if (candidates[i] == 1){
                result.push(i + 1);
            }
        }
        return result;
    };

    static bool fill_cell_recursive(Board& board, unsigned int offset){
        unsigned int row = indexer.offset_coord_lookup[offset][0];
        unsigned int col = indexer.offset_coord_lookup[offset][1];

        auto candidates = get_candidates(board, row, col);

        if (candidates.size() == 0){ return false; }

        util::shuffle_array(candidates.data(), candidates.size());
        for (unsigned int candidate_idx = 0; candidate_idx < candidates.size(); candidate_idx++){
            val_t c = candidates[candidate_idx];

            board.set(row, col, c);

            if (offset == BOARD_SIZE * BOARD_SIZE - 1)
            {
                // filled the last cell, return true
                ASSERT(board.is_solved(), "Invalid board");
                return true;
            }

            // fill the next cell
            std::unique_ptr<Board> forked_board(new Board(board));
            if (fill_cell_recursive(*forked_board, offset + 1)){
                board = *forked_board;
                return true;
            }
        }
        return false;
    }

    void fill_valid_board(Board &board){
        indexer.init();
        board.clear(0);
        fill_cell_recursive(board, 0);
    }


    static bool uniquely_solvable(const Board& board, const Board& solution){
        const int N_REPEATS = 3;

        auto solve_board = [](const Board &board, const Board &solution){
            SolverV2 solver(board);     // the solver will copy the board
            bool solved = solver.solve();
            if (!solved)
            {
                return false;
            }

            Board& answer = solver.board();
            return answer == solution;
        };

        for (int i = 0; i < N_REPEATS; i++){
            if (!solve_board(board, solution)){
                // std::cout << "Reject on trial " << i << std::endl;
                return false;
            }
        }
        return true;
    }

    // static std::vector<unsigned int> get_randomized_filled_indices(Board b){
    //     std::vector<unsigned int> indices;
    //     for (unsigned int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++){
    //         if (b.get(i) != 0){
    //             indices.push_back(i);
    //         }
    //     }
    //     // shuffle the indices
    //     auto device = std::random_device();
    //     std::mt19937 generator(device());
    //     std::shuffle(indices.begin(), indices.end(), generator);
    //     return indices;
    // };

    static  util::SizedArray<unsigned int, CELL_COUNT> get_randomized_filled_indices(Board b){
        util::SizedArray<unsigned int, CELL_COUNT> indices;
        for (unsigned int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++){
            if (b.get(i) != 0){
                indices.push(i);
            }
        }
        util::shuffle_array(indices.data(), indices.size());
        return indices;
    };

    // backtracking to remove n_clues_to_remove clues
    // this is a depth-first search... may not be the best way to do this...
    static bool remove_n_clues_recursively(Board& board, const Board& solution, int n_clues_to_remove){
        if (n_clues_to_remove == 0){
            return true;
        }

        auto indices = get_randomized_filled_indices(board);

        for (unsigned int i = 0; i < indices.size(); i++){
            unsigned int idx = indices[i];
            auto forked_board = Board(board);
            forked_board.set(idx, 0);
            if (!uniquely_solvable(forked_board, solution)) continue;

            if (remove_n_clues_recursively(forked_board, solution, n_clues_to_remove - 1)){
                board = forked_board;
                return true;
            }
        }
        return false;
    }

    bool remove_clues_by_solve(Board& board, int n_clues_to_remove){
        ASSERT(board.is_solved(), "The board should be completely filled");

        Board solution = Board(board);
        return remove_n_clues_recursively(board, board, n_clues_to_remove);


    }

}