#pragma once

#include "config.h"
#include "board.h"
#include <memory>
#include <iostream>

class CellView;

class Cell
{
public:
    Cell(CellView& parent, Coord coord);
    val_t& value();
    const Coord& coord() const;
    const Coord& grid_coord() const;

    // because unique_ptr does not support [] operator, 
    // we need to use get() method to obtain the raw pointer for easy indexing
    val_t** row();
    val_t** col();
    val_t** grid();
    // a pointer to the array of pointers to the non-repeat neighbors
    // not including self, in a 9x9 sudoku board, a cell has 20 neighbors
    val_t** neighbor();     
    const unsigned int neighbor_count = 2*(BOARD_SIZE - GRID_SIZE) + GRID_SIZE*GRID_SIZE - 1;

    friend std::ostream& operator<<(std::ostream& os, const val_t val);

private:
    CellView* m_parent_ptr;
    Board* m_board_ptr;
    Coord m_coord;
    Coord m_grid_coord;
    val_t* m_value_ptr;
    std::unique_ptr<val_t*> m_neighbor_ptrs;
};


class CellView
{
friend class Cell;
public:
    CellView(Board& board);
    Cell& operator[](const Coord& coord) const;
    Cell& get(const Coord& coord) const;
    Cell& get(int row, int col) const;
    Board& board();
private:
    Board* m_board_ptr;
    std::unique_ptr<Cell> m_cells[BOARD_SIZE][BOARD_SIZE];
    std::unique_ptr<val_t*> m_row_ptrs[BOARD_SIZE];
    std::unique_ptr<val_t*> m_col_ptrs[BOARD_SIZE];
    std::unique_ptr<val_t*> m_grid_ptrs[GRID_SIZE][GRID_SIZE];
};