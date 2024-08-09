#pragma once
#include "config.h"

template <unsigned int NG>   // NG is the grid size
class Indexer
{
public:
    static const unsigned int N = NG * NG;
    static_assert(N == BOARD_SIZE, "NG * NG must be equal to BOARD_SIZE");

    // neighbor not including self, in a 9x9 sudoku board, a cell has 20 neighbors
    static const unsigned int N_NEIGHBORS = 2 * (N - NG) + NG * NG - 1;

    // input coord to obtain grid coord, [i][j] -> [row, col] in [0, NG)
    unsigned int grid_lookup[N][N][2];               // input position to obtain it's grid row and column

    // input coord to obtain pointer offsets of this area -> offset in [0, N*N)
    unsigned int row_index[N][N];                       // pointer position for each row
    unsigned int col_index[N][N];                       // pointer position for each column
    unsigned int grid_index[N][N][N];                   // pointer position for each grid
    unsigned int neighbor_index[N][N][N_NEIGHBORS];     // pointer position for each neighbor

    void init();

private:
    bool m_initialized = false;
};

template <unsigned int NG>
void Indexer<NG>::init(){
    if (m_initialized) return; 
    m_initialized = true;

    // initialize the row index
    [&]() constexpr {
        for (unsigned int i = 0; i < N; i++)
        {
            for (unsigned int j = 0; j < N; j++)
            {
                row_index[i][j] = i * N + j;
            }
        }
    }();

    // initialize the column index
    [&]() constexpr {
        for (unsigned int i = 0; i < N; i++)
        {
            for (unsigned int j = 0; j < N; j++)
            {
                col_index[i][j] = j * N + i;
            }
        }
    }();

    // initialize the grid row and column lookup, the order of this initialization is important
    [&]() constexpr {
        for (unsigned int i = 0; i < N; i++)
        {
            for (unsigned int j = 0; j < N; j++)
            {
                grid_lookup[i][j][0] = i / NG;
                grid_lookup[i][j][1] = j / NG;
            }
        }
    }();

    // initialize the grid index
    [&]() constexpr {
        // i, j are the indices of the cell
        for (unsigned int i = 0; i < N; i++)
        {
            for (unsigned int j = 0; j < N; j++)
            {
                // k, l are the indices within the grid
                unsigned int grid_row = grid_lookup[i][j][0];
                unsigned int grid_col = grid_lookup[i][j][1];
                for (unsigned int k = 0; k < NG; k++)
                {
                    for (unsigned int l = 0; l < NG; l++)
                    {
                        grid_index[i][j][k * NG + l] = (grid_row * NG + k) * N + grid_col * NG + l;
                    }
                }
            }
        }
    }();


    // initialize the neighbor index
    [&]() constexpr {
        for (unsigned int i = 0; i < N; i++)
        {
            for (unsigned int j = 0; j < N; j++)
            {
                unsigned int neighbor_index_count = 0;

                // row neighbors
                for (unsigned int col_idx = 0; col_idx < N; col_idx++)
                {
                    if (col_idx != j)
                    {
                        neighbor_index[i][j][neighbor_index_count] = i * N + col_idx;
                        neighbor_index_count++;
                    }
                }
                ASSERT(neighbor_index_count == N - 1, "row neighbor count mismatch");

                // column neighbors
                for (unsigned int row_idx = 0; row_idx < N; row_idx++)
                {
                    if (row_idx != i)
                    {
                        neighbor_index[i][j][neighbor_index_count] = row_idx * N + j;
                        neighbor_index_count++;
                    }
                }
                ASSERT(neighbor_index_count == 2 * (N - 1), "column neighbor count mismatch");

                // grid neighbors
                for (unsigned int grid_row_idx = 0; grid_row_idx < NG; grid_row_idx++)
                {
                    for (unsigned int grid_col_idx = 0; grid_col_idx < NG; grid_col_idx++)
                    {
                        unsigned int grid_row = grid_lookup[i][j][0];
                        unsigned int grid_col = grid_lookup[i][j][1];
                        unsigned int row_idx = grid_row * NG + grid_row_idx;    
                        unsigned int col_idx = grid_col * NG + grid_col_idx;
                        if (row_idx != i && col_idx != j)
                        {
                            neighbor_index[i][j][neighbor_index_count] = row_idx * N + col_idx;
                            neighbor_index_count++;
                        }
                    }
                }
                ASSERT(neighbor_index_count == N_NEIGHBORS, "grid neighbor count mismatch");
            }
        }
    }();
};
