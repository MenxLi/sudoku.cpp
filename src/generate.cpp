#include <chrono>
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
#include <ostream>
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
        std::stack<StackItem> stack;

        // fill the first one
        stack.push({get_randomized_filled_indices(board), 0, 0});

        // for (unsigned int i = 0; i < init_indices.size(); i++){ std::cout << init_indices[i] << " " << std::flush; } std::cout << std::endl;

        long depth_remain = max_depth;
        while (stack.size() > 0){
            if (stop_flag.load()){
                return std::make_tuple(false, depth_remain);
            }

            StackItem& top_item = stack.top();
            if (top_item.next_idx >= top_item.indices.size()){
                // all indices are tried, revert the base index
                board.set(top_item.base_pos, original_board.get(top_item.base_pos));
                stack.pop();

                n_clues_to_remove++;
                depth_remain--; if (depth_remain < n_clues_to_remove){ return std::make_tuple(false, depth_remain); }
                continue;
            }

            // remove the next index and check if the board is still uniquely solvable
            unsigned int pos = top_item.indices[top_item.next_idx];
            board.set(pos, 0);
            depth_remain--; if (depth_remain < n_clues_to_remove){ return std::make_tuple(false, depth_remain); }

            if (!uniquely_solvable(board, solution)){
                board.set(pos, original_board.get(pos));
                top_item.next_idx++;
                continue;
            }
            n_clues_to_remove--;

            if (n_clues_to_remove == 0){ return std::make_tuple(true, depth_remain); }

            auto next_indices = get_randomized_filled_indices(board);
            stack.push({next_indices, pos, 0});
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

    std::tuple<bool, Board> generate_board(
        unsigned int n_clues_remain, 
        unsigned int max_retries, 
        bool parallel_exec, 
        bool verbose
        ){
        Board board;
        if (n_clues_remain > CELL_COUNT){
            return std::make_tuple(false, board);
        }

        unsigned int n_clues_to_remove = CELL_COUNT - n_clues_remain;
        std::atomic_bool stop_flag(false);
        auto fn_thread = [n_clues_to_remove, &stop_flag, verbose](
            std::promise<std::tuple<bool, Board>> promise
        ){
            Board board = Board();
            fill_board(board, FillStrategy::NAIVE);

            auto solution = Board(board);

            if (stop_flag.load()){ promise.set_value(std::make_tuple(false, board)); return; }

            // speed up...
            unsigned int n_to_remove_ = n_clues_to_remove;
            const int confident_remove_bound = CELL_COUNT / 3;
            if (n_to_remove_ > confident_remove_bound){
                gen_helper::remove_clues_no_check(board, confident_remove_bound);
                n_to_remove_ -= confident_remove_bound;
            }

            bool generated = remove_clues_by_solve(stop_flag, board, solution, n_to_remove_);
            if (generated){
                promise.set_value(std::make_tuple(true, board));
            }
            else{
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    if (verbose) std::cout << '.' << std::flush;
                }
                promise.set_value(std::make_tuple(false, board));
            }
        };
        
        if (!parallel_exec){
            if (verbose) std::cout << "Generating board (" << BOARD_SIZE << "x" << BOARD_SIZE <<
            ") with " << n_clues_remain << " clues remaining." << std::flush;
            for (unsigned int i = 0; i < max_retries; i++){
                auto promise = std::promise<std::tuple<bool, Board>>();
                auto future = promise.get_future();
                fn_thread(std::move(promise));
                auto [success, b] = future.get();
                if (success){
                    std::cout << std::endl;
                    return std::make_tuple(true, b);
                }
            }
            return std::make_tuple(false, board);
        }

        // parallel execution
        const unsigned int MAX_THREADS = 8;
        unsigned int n_concurrent = std::max(std::min( std::thread::hardware_concurrency()-1, (unsigned int) MAX_THREADS), (unsigned int) 1);
        std::array<std::future<std::tuple<bool, Board>>, MAX_THREADS> futures;
        ASSERT(n_concurrent <= MAX_THREADS, "n_concurrent should be less than or equal to 8");
        ASSERT(max_retries >= n_concurrent, "max_retries should be greater than n_threads");

        if (verbose) std::cout << "Generating board (" << BOARD_SIZE << "x" << BOARD_SIZE <<
        ") with " << n_clues_remain << " clues remaining" << " (" << n_concurrent << " concurrent)." << std::flush;

        std::vector<std::thread> threads;
        unsigned int submitted_counter = 0;

        // submit the first batch
        for (unsigned int i = 0; i < n_concurrent; i++){
            auto promise = std::promise<std::tuple<bool, Board>>();
            futures[i] = promise.get_future();
            threads.emplace_back(fn_thread, std::move(promise));
            submitted_counter++;
        }

        std::tuple<bool, Board> result{false, board};
        while(submitted_counter < max_retries && !std::get<0>(result)){
            #ifdef PYBIND11_BUILD
            if (PyErr_CheckSignals() != 0){
                throw py::error_already_set();
            }
            #endif

            for (unsigned int i = 0; i < n_concurrent; i++){
                if (futures[i].valid() && futures[i].wait_for(std::chrono::microseconds(1)) == std::future_status::ready){
                    auto [success, b] = futures[i].get();
                    // std::cout << "Checking futures " << i << std::endl;
                    if (success){
                        stop_flag.store(true);
                        result = std::make_tuple(true, b);
                        break;
                    }
                    // replace the finished future with a new one
                    if (submitted_counter < max_retries) {
                        // std::cout << "Submitting new thread " << submitted_counter << std::endl;
                        auto promise = std::promise<std::tuple<bool, Board>>();
                        futures[i] = promise.get_future();
                        threads.emplace_back(fn_thread, std::move(promise));
                        submitted_counter++;
                    }
                }
            }
        }

        // wait for all threads to finish, clean up
        for (auto& t: threads){
            t.join();
        }
        if (verbose) std::cout << std::endl;

        return result;
    }

}
