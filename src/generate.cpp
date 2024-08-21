#include <chrono>
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
#include <tuple>
#include <algorithm>
#include <future>
#include <atomic>
#include <stack>

namespace gen{
    static Indexer<GRID_SIZE> indexer;

    static std::vector<val_t> get_candidates(Board& board, int row, int col){
        bool candidates_idx_allowd[CANDIDATE_SIZE];
        for (unsigned int i = 0; i < CANDIDATE_SIZE; i++){
            candidates_idx_allowd[i] = true;
        }
        for (auto offset: indexer.neighbor_index[row][col]){
            val_t n_value = board.get(offset);
            if (n_value != 0){
                unsigned int v_idx = n_value - 1;
                candidates_idx_allowd[v_idx] = false;
            }
        }
        util::SizedArray<val_t, CANDIDATE_SIZE> result;
        for (unsigned int i = 0; i < CANDIDATE_SIZE; i++){
            if (candidates_idx_allowd[i]){
                result.push(i + 1);
            }
        }
        return std::vector<val_t>(result.data(), result.data() + result.size());
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
            Board forked_board = Board(board);
            if (fill_cell_recursive(forked_board, offset + 1)){
                board.load_data(forked_board);
                return true;
            }
        }
        return false;
    }

    static void fill_cell_iterative(Board& board){
        // board should be all empty

        unsigned int offset = 0;
        
        struct StackItem{
            unsigned int offset;
            std::vector<val_t> candidates;
            unsigned int next_candidate_idx;
        };

        std::stack<StackItem> stack;

        // fill the first cell
        unsigned int row = indexer.offset_coord_lookup[offset][0];
        unsigned int col = indexer.offset_coord_lookup[offset][1];

        auto candidates = get_candidates(board, row, col);
        util::shuffle_array(candidates.data(), candidates.size());
        stack.push({offset, candidates, 0});

        while(stack.size() > 0){
            StackItem& top_item = stack.top();
            if (top_item.next_candidate_idx >= top_item.candidates.size()){
                // all candidates are tried, revert the current cell
                board.set(top_item.offset, 0);
                stack.pop();
                if (stack.size() == 0){
                    break;
                }
                stack.top().next_candidate_idx++;
                continue;
            }

            // fill the next cell
            val_t c = top_item.candidates[top_item.next_candidate_idx];
            board.set(top_item.offset, c);

            // check if the board is solved
            if (top_item.offset == CELL_COUNT - 1){
                ASSERT(board.is_solved(), "Invalid board, error while filling the board");
                return;
            }

            ASSERT(top_item.offset < CELL_COUNT - 1, "Invalid offset");

            // push the next cell to the stack
            offset = top_item.offset + 1;
            row = indexer.offset_coord_lookup[offset][0];
            col = indexer.offset_coord_lookup[offset][1];
            auto candidates = get_candidates(board, row, col);
            // std::cout << "Candidates: "; for (auto c: candidates){ std::cout << c << " "; } std::cout << std::endl;

            util::shuffle_array(candidates.data(), candidates.size());
            stack.push({offset, candidates, 0});

            // std::cout << "Current stack size: " << stack.size() << std::endl;
        }

    }

    void fill_valid_board(Board &board){
        indexer.init();
        board.clear(0);
        fill_cell_iterative(board);
    }

    static bool uniquely_solvable(const Board& board, const Board& solution){
        auto solve_board = [](
            const Board &board, 
            const Board &solution, 
            unsigned int solve_pattern
            ){
            SolverV2 solver(board);     // the solver will copy the board
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
                // if (i!=0) std::cout << "Reject non-unique solution on repeats: " << i << std::endl;
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
        std::atomic_bool& stop_flag,
        Board& board, 
        const Board& solution, 
        unsigned int n_clues_to_remove, 
        long max_depth = CELL_COUNT*BOARD_SIZE
    ){
        if (stop_flag.load()){
            return std::make_tuple(false, max_depth);
        }
        if (n_clues_to_remove == 0){
            // std::cout << "Found a board with max depth left: " << max_depth << std::endl;
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
    static std::tuple<bool, long> remove_n_clues_iteratively(
        std::atomic_bool& stop_flag,
        Board& board, 
        const Board& solution, 
        unsigned int n_clues_to_remove, 
        long max_depth = CELL_COUNT*BOARD_SIZE
    ){
        // avoid stack overflow from recursion

        struct StackItem{
            std::vector<unsigned int> indices;
            unsigned int base_pos;  // the position that should be reverted if all indices are tried
            unsigned int next_idx;  // the next index of the indices to try
        };
        class Stack{
        public:
            void push(const StackItem&& item){
                m_stack.push_back(new StackItem(item));
            }
            void pop(){
                delete m_stack.back();
                m_stack.pop_back();
            }
            StackItem& top(){
                return *m_stack.back();
            }
            size_t size(){
                return m_stack.size();
            }

        private:
            std::vector<StackItem*> m_stack;
        };

        Board original_board = Board(board);
        Stack stack;

        // fill the first one
        stack.push({get_randomized_filled_indices(board), 0, 0});

        // print
        // for (unsigned int i = 0; i < init_indices.size(); i++){ std::cout << init_indices[i] << " " << std::flush; } std::cout << std::endl;

        long depth_remain = max_depth;
        while (stack.size() > 0){
            // std::cout << "Depth: " << depth_remain << " Clues: " << n_clues_to_remove << std::endl;
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

                // std::cout << "Rev: " << n_clues_to_remove << std::endl;
                continue;
            }

            // remove the next index and check if the board is still uniquely solvable
            unsigned int pos = top_item.indices[top_item.next_idx];
            board.set(pos, 0);
            depth_remain--; if (depth_remain < n_clues_to_remove){ return std::make_tuple(false, depth_remain); }

            // std::cout << "N: " << n_clues_to_remove << " Depth: " << depth_remain << std::endl;
            if (!uniquely_solvable(board, solution)){
                board.set(pos, original_board.get(pos));
                top_item.next_idx++;
                continue;
            }
            n_clues_to_remove--;

            // the board is still uniquely solvable, check if we have removed enough clues
            if (n_clues_to_remove == 0){ return std::make_tuple(true, depth_remain); }

            // if we still need to remove more clues, push the current state to the stack
            auto next_indices = get_randomized_filled_indices(board);
            stack.push({next_indices, pos, 0});
        }
        return std::make_tuple(false, depth_remain);
    }

    bool remove_clues_by_solve(std::atomic_bool& stop_flag, Board& board, const Board& solution, int n_clues_to_remove){
        auto result = remove_n_clues_iteratively(stop_flag, board, solution, n_clues_to_remove);
        // auto result = remove_n_clues_recursively(stop_flag, board, solution, n_clues_to_remove);
        return std::get<0>(result);
    }

    std::tuple<bool, Board> generate_board(
        unsigned int n_clues_remain, 
        unsigned int max_retries, 
        bool parallel_exec
        ){
        Board board;
        if (n_clues_remain >= CELL_COUNT){
            return std::make_tuple(false, board);
        }

        // if (BOARD_SIZE == 9 && n_clues_remain < 17){
        //     return std::make_tuple(false, board);
        // }

        unsigned int n_clues_to_remove = CELL_COUNT - n_clues_remain;
        std::atomic_bool stop_flag(false);
        auto fn_thread = [n_clues_to_remove, &stop_flag](
            std::promise<std::tuple<bool, Board>> promise
        ){
            Board board = Board();
            unsigned int n_to_remove_ = n_clues_to_remove;

            if (stop_flag.load()){
                promise.set_value(std::make_tuple(false, board));
                return;
            }

            fill_valid_board(board);
            auto solution = Board(board);

            // speed up...
            const int confident_remove_bound = CELL_COUNT / 3;
            if (n_to_remove_ > confident_remove_bound){
                remove_clues_no_check(board, confident_remove_bound);
                n_to_remove_ -= confident_remove_bound;
            }

            bool generated = remove_clues_by_solve(stop_flag, board, solution, n_to_remove_);
            if (generated){
                promise.set_value(std::make_tuple(true, board));
            }
            else{
                std::cout << '.';
                std::cout.flush();
                promise.set_value(std::make_tuple(false, board));
            }
        };
        
        if (!parallel_exec){
            std::cout << "Generating board (" << BOARD_SIZE << "x" << BOARD_SIZE <<
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

        std::cout << "Generating board (" << BOARD_SIZE << "x" << BOARD_SIZE <<
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
                if (futures[i].valid() && futures[i].wait_for(std::chrono::microseconds(0)) == std::future_status::ready){
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
        std::cout << std::endl;

        return result;
    }

}
