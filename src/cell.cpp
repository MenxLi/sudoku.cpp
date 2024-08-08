#include "cell.h"
#include "board.h"
#include "config.h"
#include <memory>

Cell::Cell(CellView& parent, Coord coord)
{
    Board& board = *parent.m_board_ptr;
    m_parent_ptr = &parent;
    m_board_ptr = parent.m_board_ptr;
    m_coord = coord;

    m_value_ptr = &board.get_(m_coord);

    int grid_row = m_coord.row / GRID_SIZE;
    int grid_col = m_coord.col / GRID_SIZE;
    m_grid_coord = Coord{grid_row, grid_col};

    // calculate it's neighbors
    m_neighbor_ptrs.reset(new val_t*[neighbor_count]);
    unsigned int neighbor_index = 0;
    for (unsigned int col_idx = 0; col_idx < BOARD_SIZE; col_idx++)
    {
        if (col_idx != m_coord.col)
        {
            m_neighbor_ptrs.get()[neighbor_index] = &board.get_(m_coord.row, col_idx);
            neighbor_index++;
        }
    }
    ASSERT(neighbor_index == BOARD_SIZE - 1, "neighbor index out of bounds: " + std::to_string(neighbor_index));
    for (unsigned int row_idx = 0; row_idx < BOARD_SIZE; row_idx++)
    {
        if (row_idx != m_coord.row)
        {
            m_neighbor_ptrs.get()[neighbor_index] = &board.get_(row_idx, m_coord.col);
            neighbor_index++;
        }
    }
    ASSERT(neighbor_index == 2*BOARD_SIZE - 2, "neighbor index out of bounds: " + std::to_string(neighbor_index));
    for (unsigned int row_idx = m_grid_coord.row * GRID_SIZE; row_idx < (m_grid_coord.row + 1) * GRID_SIZE; row_idx++)
    {
        for (unsigned int col_idx = m_grid_coord.col * GRID_SIZE; col_idx < (m_grid_coord.col + 1) * GRID_SIZE; col_idx++)
        {
            if (row_idx != m_coord.row && col_idx != m_coord.col)
            {
                m_neighbor_ptrs.get()[neighbor_index] = &board.get_(row_idx, col_idx);
                neighbor_index++;
            }
        }
    }
    ASSERT(neighbor_index == neighbor_count, "neighbor index out of bounds: " + std::to_string(neighbor_index));
};

val_t& Cell::value()
{
    return *m_value_ptr;
};

const Coord& Cell::coord() const 
{
    return m_coord;
};

const Coord& Cell::grid_coord() const
{
    return m_grid_coord;
};

val_t** Cell::row()
{
    return m_parent_ptr->m_row_ptrs[coord().row].get();
};

val_t** Cell::col()
{
    return m_parent_ptr->m_col_ptrs[coord().col].get();
};

val_t** Cell::neighbor()
{
    return m_neighbor_ptrs.get();
};

val_t** Cell::grid()
{
    return m_parent_ptr->m_grid_ptrs[grid_coord().row][grid_coord().col].get();
};


CellView::CellView(Board& board)
{
    m_board_ptr = &board;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            m_cells[i][j] = std::unique_ptr<Cell>(new Cell(*this, Coord{i, j}));
        }
    }
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        m_row_ptrs[i] = board.get_row(i);
        m_col_ptrs[i] = board.get_col(i);
    }
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            m_grid_ptrs[i][j] = board.get_grid(i, j);
        }
    }
};

Cell& CellView::operator[](const Coord &coord) const
{
    return this->get(coord);
};

Cell& CellView::get(const Coord &coord) const
{
    return *m_cells[coord.row][coord.col];
};

Cell& CellView::get(int row, int col) const
{
    return *m_cells[row][col];
};

Board& CellView::board()
{
    return *m_board_ptr;
};