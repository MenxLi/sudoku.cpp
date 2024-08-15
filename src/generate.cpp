#ifdef PYBIND11_BUILD
#include <pybind11/pybind11.h>
namespace py = pybind11;
#endif
#include "generate.h"
#include "board.h"
#include "config.h"
#include "indexer.hpp"
#include "util.h"
#include "solver_v2.h"
#include <ostream>
#include <random>
#include <memory>
#include <tuple>
#include <vector>
#include <algorithm>
#include <thread>

namespace gen{
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
                if (i!=0) std::cout << "Reject non-unique solution on repeats: " << i << std::endl;
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

    static void remove_clues_no_check(Board& board, int n_clues_to_remove){
        auto indices = get_randomized_filled_indices(board);
        for (unsigned int i = 0; i < indices.size(); i++){
            unsigned int idx = indices[i];
            board.set(idx, 0);
            n_clues_to_remove--;
            if (n_clues_to_remove == 0){
                return;
            }
        }
    }


    // backtracking to remove n_clues_to_remove clues
    // this is a depth-first search... may not be the best way to do this...
    static std::tuple<bool, long> remove_n_clues_recursively(
        Board& board, 
        const Board& solution, 
        unsigned int n_clues_to_remove, 
        long max_depth = CELL_COUNT
    ){
        if (n_clues_to_remove == 0){
            // std::cout << "Found a board with max depth left: " << max_depth << std::endl;
            return std::make_tuple(true, max_depth);
        }
        if (max_depth <= 0 && n_clues_to_remove > 0){
            return std::make_tuple(false, 0);
        }

        auto indices = get_randomized_filled_indices(board);

        for (unsigned int i = 0; i < indices.size(); i++){
            unsigned int idx = indices[i];
            auto forked_board = Board(board);
            forked_board.set(idx, 0);
            if (!uniquely_solvable(forked_board, solution)) continue;

            auto [success, depth_remain] = remove_n_clues_recursively(
                forked_board, solution, n_clues_to_remove - 1, max_depth - 1
            );
            if (success){
                board = forked_board;
                return std::make_tuple(true, depth_remain);
            }
            max_depth = depth_remain;
        }
        return std::make_tuple(false, max_depth);
    }

    bool remove_clues_by_solve(Board& board, const Board& solution, int n_clues_to_remove){
        auto [success, depth_remain] = remove_n_clues_recursively(board, solution, n_clues_to_remove);
        return success;
    }

    std::tuple<bool, Board> generate_board(unsigned int n_clues_remain, unsigned int max_retries){
        Board board;
        if (n_clues_remain >= CELL_COUNT){
            return std::make_tuple(false, board);
        }

        // if (BOARD_SIZE == 9 && n_clues_remain < 17){
        //     return std::make_tuple(false, board);
        // }

        unsigned int n_clues_to_remove = CELL_COUNT - n_clues_remain;
        std::cout << "Generating board (" << BOARD_SIZE << "x" << BOARD_SIZE <<
        ") with " << n_clues_remain << " clues remaining." << std::flush;

        auto fn_thread = [&n_clues_to_remove](bool& flag_buffer, Board& board_buffer){
            std::unique_ptr<Board> board_ptr(new Board);
            Board& board = *board_ptr;
            unsigned int n_to_remove_ = n_clues_to_remove;

            fill_valid_board(board);
            auto solution = Board(board);

            // speed up...
            const int confident_remove_bound = CELL_COUNT / 2;
            if (n_to_remove_ > confident_remove_bound){
                remove_clues_no_check(board, confident_remove_bound);
                n_to_remove_ -= confident_remove_bound;
            }

            bool generated = remove_clues_by_solve(board, solution, n_to_remove_);
            if (generated){
                board_buffer = board;
                flag_buffer = true;
            }
            else{
                std::cout << '.';
                std::cout.flush();
                flag_buffer = false;
            }
        };

        unsigned int n_threads = std::min( std::thread::hardware_concurrency(), (unsigned int) 8);
        unsigned int n_batches = max_retries / n_threads;
        ASSERT(max_retries >= n_threads, "max_retries should be greater than n_threads");
        for (int i = 0; i < n_batches; i++){
            std::unique_ptr<std::thread[]> threads(new std::thread[n_threads]);
            std::unique_ptr<bool[]> result_flags(new bool[n_threads]{0});
            std::unique_ptr<Board[]> reault_boards(new Board[n_threads]);

            for (int j = 0; j < n_threads; j++){
                threads[j] = std::thread(fn_thread, std::ref(result_flags[j]), std::ref(reault_boards[j]));
            }

            for (int j = 0; j < n_threads; j++){
                threads[j].join();
            }

            #ifdef PYBIND11_BUILD
            if (PyErr_CheckSignals() != 0){
                throw py::error_already_set();
            }
            #endif

            for (int j = 0; j < n_threads; j++){
                if (result_flags[j]){
                    std::cout << std::endl;
                    board.load_data(reault_boards[j]);
                    return std::make_tuple(true, board);
                }
            }
        } // discard the leftover retries...

        std::cout << std::endl;
        return std::make_tuple(false, board);
    }

}