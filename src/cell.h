#pragma once

#include "config.h"
#include "board.h"
#include <memory>
#include <iostream>

class Cell
{
public:
    Cell(Board& board, Coord coord);
    val_t& value();
    const Coord& coord();
    const Coord& grid_coord();

    // because unique_ptr does not support [] operator, 
    // we need to use get() method to obtain the raw pointer for easy indexing
    val_t** row();
    val_t** col();
    val_t** grid();

    friend std::ostream& operator<<(std::ostream& os, const val_t val);

private:
    Board* m_board_ptr;
    Coord m_coord;
    Coord m_grid_coord;
    val_t* m_value_ptr;
    std::unique_ptr<val_t*> m_row_ptr;
    std::unique_ptr<val_t*> m_col_ptr;
    std::unique_ptr<val_t*> m_grid_ptr;
};
