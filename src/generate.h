#pragma once
#include "board.h"

namespace gen
{
    void fill_valid_board(Board& board);
    bool remove_clues_by_solve(Board& board, int n_clues_to_remove);
    std::tuple<bool, Board> generate_board(int n_clues_remain, unsigned int max_retries = 2048);
} // namespace generate
