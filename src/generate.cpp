#include <chrono>
#include <iostream>
#ifdef PYBIND11_BUILD
#include <pybind11/pybind11.h>
namespace py = pybind11;
#endif
#include "generate.h"
#include "board.h"
#include "config.h"
#include "indexer.h"
#include "util.h"
#include "solver.h"
#include <tuple>
#include <algorithm>
#include <future>
#include <atomic>
#include <stack>
#include <mutex>

static std::mutex mtx;

namespace gen_helper{

    /*
    Check if the board is uniquely solvable by solving it twice with different solve patterns
    */
    bool uniquely_solvable(const Board& board, const Board& solution){
        auto solve_board = [](
            const Board &board, 
            const Board &solution, 
            unsigned int solve_pattern
            ){
            Solver solver(board);     // the solver will copy the board
            switch (solve_pattern){
                case 0:
                    solver.config().heuristic_guess = true;
                    solver.config().reverse_guess = false;
                    break;
                case 1:
                    solver.config().heuristic_guess = true;
                    solver.config().reverse_guess = true;
                    break;
                default:
                    solver.config().heuristic_guess = false;
                    solver.config().use_double = true;
                    break;
            }

            bool solved = solver.solve();
            if (!solved)
            {
                return false;
            }

            Board& answer = solver.board();
            return answer == solution;
        };

        const unsigned int N_REPEATS = 2;

        for (unsigned int i = 0; i < N_REPEATS; i++){
            if (!solve_board(board, solution, i)){
                return false;
            }
        }
        return true;
    }

    /* Get a list of indices of filled cells in a board, shuffled randomly */
    std::vector<unsigned int> get_randomized_filled_indices(Board b){
        util::SizedArray<unsigned int, CELL_COUNT> indices;
        for (unsigned int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++){
            if (b.get(i) != 0){
                indices.push(i);
            }
        }
        util::shuffle_array(indices.data(), indices.size());
        return std::vector<unsigned int>(indices.data(), indices.data() + indices.size());
    };

    /* To remove clues from a board, without checking if the board is still uniquely solvable */
    void remove_clues_no_check(Board& board, int n_clues_to_remove){
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

    /* 
    [Deprecated]
    Remove n_clues_to_remove clues from the board, recursively
    will make sure the board is still uniquely solvable
    */
    std::tuple<bool, long> remove_n_clues_recursively(
        std::atomic_bool& stop_flag,
        Board& board, 
        const Board& solution, 
        unsigned int n_clues_to_remove, 
        long max_depth = CELL_COUNT*2
    ){
        if (stop_flag.load()){
            return std::make_tuple(false, max_depth);
        }
        if (n_clues_to_remove == 0){
            return std::make_tuple(true, max_depth);
        }
        if (max_depth < n_clues_to_remove){ // not enough depth to remove all clues
            return std::make_tuple(false, 0);
        }

        auto indices = get_randomized_filled_indices(board);

        long depth_remain = max_depth;
        for (unsigned int i = 0; i < indices.size(); i++){

            unsigned int idx = indices[i];
            // auto forked_board = Board(board);
            auto forked_board = std::unique_ptr<Board>(new Board(board));
            forked_board->set(idx, 0);
            depth_remain--; if (depth_remain < n_clues_to_remove){ return std::make_tuple(false, depth_remain); }

            if (!uniquely_solvable(*forked_board, solution)) continue;

            auto [success, _depth_remain] = remove_n_clues_recursively(
                stop_flag, *forked_board, solution, n_clues_to_remove - 1, depth_remain
            );
            depth_remain = _depth_remain;
            if (success){
                board.load_data(*forked_board);
                return std::make_tuple(true, depth_remain);
            }
        }
        return std::make_tuple(false, depth_remain);
    }

    /*
    Remove n_clues_to_remove clues from the board, iteratively to avoid stack overflow. 
    Will make sure the board is still uniquely solvable
    */
    std::tuple<bool, long> remove_n_clues_iteratively(
        std::atomic_bool& stop_flag,
        Board& board, 
        const Board& solution, 
        unsigned int n_clues_to_remove, 
        long max_depth = CELL_COUNT*2
    ){
        struct StackItem{
            std::vector<unsigned int> indices;
            unsigned int base_pos;  // the position that should be reverted if all indices are tried
            unsigned int next_idx;  // the next index of the indices to try
        };
        Board original_board = Board(board);

        if (!uniquely_solvable(board, solution)){
            return std::make_tuple(false, max_depth);
        }

        std::stack<StackItem> stack;

        // fill the first one
        stack.push({get_randomized_filled_indices(board), 0, 0});

        long depth_remain = max_depth;

        const float TRAIL_PERCENTILE = 0.5f;
        while (stack.size() > 0){

            if (stop_flag.load())
                return std::make_tuple(false, depth_remain);

            StackItem& top_item = stack.top();
            auto trail_size = static_cast<unsigned int>(top_item.indices.size() * TRAIL_PERCENTILE);
            if (top_item.next_idx >= trail_size){
                // all indices are tried, revert the base index
                board.set(top_item.base_pos, original_board.get(top_item.base_pos));
                stack.pop();

                n_clues_to_remove++;
                depth_remain--; 
                
                if (depth_remain < n_clues_to_remove)
                    return std::make_tuple(false, depth_remain); 

                continue;
            }

            // remove the next index and check if the board is still uniquely solvable
            unsigned int pos = top_item.indices[top_item.next_idx];
            board.set(pos, 0);
            depth_remain--; 

            if (!uniquely_solvable(board, solution)){
                // std::cout << "Depth remain [c]: " << depth_remain << ", clues to remove: " << n_clues_to_remove << std::endl;
                board.set(pos, original_board.get(pos));
                top_item.next_idx++;
                continue;
            }

            n_clues_to_remove--;
            if (depth_remain < n_clues_to_remove)
                return std::make_tuple(false, depth_remain); 

            if (n_clues_to_remove == 0)
                return std::make_tuple(true, depth_remain); 

            auto next_indices = get_randomized_filled_indices(board);
            stack.push({next_indices, pos, 0});

            // std::cout << "Depth remain: " << depth_remain << ", clues to remove: " << n_clues_to_remove << std::endl;
        }
        return std::make_tuple(false, depth_remain);
    }
}

namespace gen{

    bool remove_clues_by_solve(std::atomic_bool& stop_flag, Board& board, const Board& solution, int n_clues_to_remove){
        if (n_clues_to_remove == 0){ return board == solution; }
        auto result = gen_helper::remove_n_clues_iteratively(stop_flag, board, solution, n_clues_to_remove);
        return std::get<0>(result);
    }

    std::tuple<bool, Board> generate_board_worker(
        std::atomic_bool& stop_flag, 
        unsigned int n_clues_remain, 
        unsigned int max_retries, 
        std::function<void(unsigned int)> progress_callback
    ){
        if (n_clues_remain > CELL_COUNT){
            return std::make_tuple(false, Board{});
        }

        unsigned int n_clues_to_remove = CELL_COUNT - n_clues_remain;

        for (unsigned int i = 0; i < max_retries; i++){
            if (stop_flag.load()){
                return std::make_tuple(false, Board{});
            }
            Board board;
            fill_board(board, FillStrategy::NAIVE);
            auto solution = Board(board);

            // speed up...
            unsigned int n_to_remove_ = n_clues_to_remove;
            const int confident_remove_bound = CELL_COUNT / 3;
            if (n_to_remove_ > confident_remove_bound){
                gen_helper::remove_clues_no_check(board, confident_remove_bound);
                n_to_remove_ -= confident_remove_bound;
            }

            bool generated = remove_clues_by_solve(stop_flag, board, solution, n_to_remove_);

            if (progress_callback != nullptr){
                progress_callback(i);
            }
            if (generated){
                stop_flag.store(true);
                return std::make_tuple(true, board);
            }
        };

        return std::make_tuple(false, Board{});
    }

    std::tuple<bool, Board> generate_board(
        unsigned int n_clues_remain, 
        unsigned int max_retries, 
        int n_threads, 
        bool verbose
    ){
        std::atomic_bool stop_flag(false);
        if (verbose){
            std::cout << "Generating board with " << n_clues_remain << " clues remaining, max retries: " << max_retries << std::endl;
        }

        if (n_threads == 0){
            auto r = generate_board_worker(
                stop_flag, n_clues_remain, max_retries, 
                [verbose](unsigned int){ if (verbose){ std::cout << "." << std::flush; } }
            );
            if (verbose) std::cout << std::endl;
            return r;
        }

        if (n_threads < 0){
            n_threads = std::thread::hardware_concurrency() - 1; // leave one thread for the main thread
        }

        std::vector<std::future<std::tuple<bool, Board>>> futures;
        for (int i = 0; i < n_threads; i++){
            auto fut = std::async(std::launch::async, generate_board_worker, std::ref(stop_flag), n_clues_remain, max_retries, 
                [verbose](unsigned int){
                    if (verbose){
                        std::lock_guard<std::mutex> lock(mtx);
                        std::cout << "." << std::flush;
                    }
                }
            );
            futures.push_back(std::move(fut));
        }
        std::tuple<bool, Board> result = std::make_tuple(false, Board{});
        for (auto& future: futures){
            auto [success, board] = future.get();
            if (success){
                if (std::get<0>(result)){ continue; }
                result = std::make_tuple(true, board);
            }
        }
        if (verbose) std::cout << std::endl;
        return result;
    };
}
