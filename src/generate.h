#pragma once
#include "board.h"

namespace generate
{
    void fill_valid_board(Board& board);
    bool remove_clues_by_solve(Board& board, int n_clues_to_remove);
} // namespace generate
