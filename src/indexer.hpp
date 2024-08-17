#pragma once
#include "config.h"
#include "util.h"
#include "mutex"

template <unsigned int NG>   // NG is the grid size
class Indexer
{
public:
    static const unsigned int N = NG * NG;      // board size
    static const unsigned int NV = N;           // number of values/candidates
    static_assert(N == BOARD_SIZE, "NG * NG must be equal to BOARD_SIZE");

    // neighbor not including self, in a 9x9 sudoku board, a cell has 20 neighbors
    static const unsigned int N_NEIGHBORS = 2 * (N - NG) + NG * NG - 1;

    // input coord to obtain grid coord, [i][j] -> [row, col] in [0, NG)
    unsigned int grid_lookup[N][N][2];                      // input position to obtain it's grid row and column
    unsigned int offset_coord_lookup[N*N][2];               // input offset to obtain it's coord [i][j]
    unsigned int coord_offset_lookup[N][N];                 // input coord to obtain it's offset in [0, N*N), same to row_index

    // Input coord to obtain pointer offsets of a relevent area -> offset in [0, N*N).
    // The last dimension is the pointer offsets for the relevent area. 
    // For example, given a cell at [<any row>][col], 
    // we can obtain the pointer offsets for all elements in it's column 
    // by querying: col_index[col][...]
    unsigned int row_index[N][N];                       // pointer position for each row, input 1D row index [i]
    unsigned int col_index[N][N];                       // pointer position for each column, input 1D column index [j]
    unsigned int grid_index[N][N][N];                   // pointer position for each grid, input 2D cell coord [i][j]
    unsigned int grid_coord_index[NG][NG][N];           // pointer position for each grid, input 2D cell coord [i][j]
    unsigned int neighbor_index[N][N][N_NEIGHBORS];     // pointer position for each neighbor, input 2D cell coord [i][j]

    inline static std::array<std::array<unsigned int, 2>, util::n_combinations<N, 2>>
    subunit_combinations_2{ util::combinations<unsigned int, N, 2>(util::range<N>()) };

    inline static std::array<std::array<unsigned int, 2>, util::n_combinations<NV, 2>>
    subvalue_combinations_2{ util::combinations<unsigned int, NV, 2>(util::range<NV>()) };

    void init();

private:
    bool m_initialized = false;
    std::mutex m_mutex;
};

template <unsigned int NG>
void Indexer<NG>::init(){
    // this is important, because I want to use this as a static member 
    // it dooms to share global state, so need to make sure it's thread safe!
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_initialized) return; 
    m_initialized = true;

    // initialize the row index
    [&]() constexpr {
        for (unsigned int i = 0; i < N; i++)
        {
            for (unsigned int j = 0; j < N; j++)
            {
                const unsigned int offset = i * N + j;

                row_index[i][j] = offset;

                // by the way, initialize the offset lookup
                offset_coord_lookup[offset][0] = i;
                offset_coord_lookup[offset][1] = j;
                coord_offset_lookup[i][j] = offset;
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
        // i, j are the indices of the grid
        for (unsigned int i = 0; i < NG; i++)
        {
            for (unsigned int j = 0; j < NG; j++)
            {

                for (unsigned int k = 0; k < NG; k++)
                {
                    for (unsigned int l = 0; l < NG; l++)
                    {
                        grid_coord_index[i][j][k * NG + l] = (i * NG + k) * N + j * NG + l;
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
