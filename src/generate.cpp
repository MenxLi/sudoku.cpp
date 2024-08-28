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

static Indexer indexer;
static std::mutex mtx;

namespace gen_helper{
    /*
    a meta board is a board that contains the simplist form of a filled board
    generated with fixed strategy.
    */
    Board get_meta_board(){
        auto get_iota_row = [](){
            std::array<val_t, BOARD_SIZE> row_data;
            std::iota(row_data.begin(), row_data.end(), 1);
            return row_data;
        };

        auto lshift_row = [](std::array<val_t, BOARD_SIZE>& arr, unsigned int n){
            std::rotate(arr.begin(), arr.begin() + n, arr.end());
        };

        auto meta_row = [&](unsigned int row){
            std::array<val_t, BOARD_SIZE> row_data = get_iota_row();
            unsigned int n_shift = row / GRID_SIZE + (row % GRID_SIZE) * GRID_SIZE;
            lshift_row(row_data, n_shift);
            return row_data;
        };

        Board board;
        for (unsigned int i = 0; i < BOARD_SIZE; i++){
            auto row_data = meta_row(i);
            for (unsigned int j = 0; j < BOARD_SIZE; j++){
                board.set(i, j, row_data[j]);
            }
        }
        ASSERT(board.is_solved(), "Invalid meta board");
        return board;
    }

    /*
    Apply random equivalence transformation to the board
    */
    void apply_random_transform(Board& board, unsigned int n_repeats){
        std::srand(std::time(nullptr));
        for (unsigned int i = 0; i < n_repeats; i++){
            unsigned int transform_type = std::rand() % 4;
            unsigned int idx1;
            unsigned int idx2;
            unsigned int g_idx1;
            unsigned int g_idx2;
            switch (transform_type){
                case 0:
                    idx1 = std::rand() % GRID_SIZE;
                    idx2 = std::rand() % GRID_SIZE;
                    g_idx1 = std::rand() % GRID_SIZE;
                    BoardEquivalenceTransform::swap_row(board, g_idx1, idx1, idx2);
                    break;
                case 1:
                    g_idx1 = std::rand() % GRID_SIZE;
                    g_idx2 = std::rand() % GRID_SIZE;
                    BoardEquivalenceTransform::swap_band(board, g_idx1, g_idx2);
                    break;
                case 2:
                    idx1 = std::rand() % CANDIDATE_SIZE;
                    idx2 = std::rand() % CANDIDATE_SIZE;
                    BoardEquivalenceTransform::swap_value(board, idx1 + 1, idx2 + 1);
                    break;
                case 3:
                    BoardEquivalenceTransform::transpose(board);
                    break;
                default:
                    break;
            }
        }
        ASSERT(board.is_valid(), "Invalid board after applying random transform");
    }

    /*
    Get a list of valid candidates for a cell in the board, 
    based on the current state of it's neighbors
    */
    std::vector<val_t> get_candidates(Board& board, int row, int col){
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

    /* 
    Fill the board with valid values, using backtracking 
    Should make sure the bord is empty before calling this function
    */
    void fill_cell_iterative(Board& board){
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

            util::shuffle_array(candidates.data(), candidates.size());
            stack.push({offset, candidates, 0});
        }
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

    void fill_valid_board(Board &board, FillStrategy strategy){
        if (strategy == FillStrategy::SEARCH){
            board.clear(0);
            gen_helper::fill_cell_iterative(board);
        }
        else{
            board.load_data(gen_helper::get_meta_board());
            gen_helper::apply_random_transform(board, 100*BOARD_SIZE);
        }
    }

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
            fill_valid_board(board, FillStrategy::TRANSFORM);
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
                    if (verbose) std::cout << '.'; std::cout.flush();
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
