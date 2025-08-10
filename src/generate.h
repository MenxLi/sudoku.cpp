#pragma once
#include "fill.h"
#include <atomic>
#include <tuple>
#include <functional>

namespace gen
{
    bool remove_clues_by_solve(Board& board, int n_clues_to_remove);

    std::tuple<bool, Board> generate_board_worker(
        std::atomic_bool& stop_flag, 
        unsigned int n_clues_remain, 
        unsigned int max_retries = 2048, 
        std::function<void(unsigned int)> progress_callback = nullptr
    );

    std::tuple<bool, Board> generate_board(
        unsigned int n_clues_remain, 
        unsigned int max_retries = 2048, 
        int n_threads = 1, 
        bool verbose = true
    );
}
