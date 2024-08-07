#include "cell.h"

Cell::Cell(Board& board, Coord coord)
{
    m_board_ptr = &board;
    m_coord = coord;

    m_value_ptr = &board.get_(m_coord);
    m_row_ptr.reset((board.get_row(m_coord.row)).release()); 
    m_col_ptr.reset((board.get_col(m_coord.col)).release()); 

    int grid_row = m_coord.row / GRID_SIZE;
    int grid_col = m_coord.col / GRID_SIZE;
    m_grid_ptr = board.get_grid(grid_row, grid_col);    // both way work
    m_grid_coord = Coord{grid_row, grid_col};
};

val_t& Cell::value()
{
    return *m_value_ptr;
};

const Coord& Cell::coord()
{
    return m_coord;
};

const Coord& Cell::grid_coord()
{
    return m_grid_coord;
};

val_t** Cell::row()
{
    return m_row_ptr.get();
};

val_t** Cell::col()
{
    return m_col_ptr.get();
};

val_t** Cell::grid()
{
    return m_grid_ptr.get();
};
