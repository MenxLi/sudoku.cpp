#pragma once
#include "fill.h"
#include <tuple>

namespace gen
{
    bool remove_clues_by_solve(Board& board, int n_clues_to_remove);

    std::tuple<bool, Board> generate_board(
        unsigned int n_clues_remain, 
        unsigned int max_retries = 2048, 
        bool parallel_exec = true,
        bool verbose = false
    );
} // namespace generate
