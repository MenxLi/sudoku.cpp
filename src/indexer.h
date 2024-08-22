#pragma once
#include "config.h"
#include "util.h"

static unsigned int constexpr const_n_combinations_2(unsigned int N){
    return N * (N - 1) / 2;
}

const unsigned int NG =  GRID_SIZE;
class Indexer
{
public:
    static const unsigned int N = NG * NG;      // board size
    static const unsigned int NV = N;           // number of values/candidates
    static_assert(N == BOARD_SIZE, "NG * NG must be equal to BOARD_SIZE");

    // neighbor not including self, in a 9x9 sudoku board, a cell has 20 neighbors
    static const unsigned int N_NEIGHBORS = 2 * (N - NG) + NG * NG - 1;

    // input coord to obtain grid coord, [i][j] -> [row, col] in [0, NG)
    static const unsigned int grid_lookup[N][N][2];                      // input position to obtain it's grid row and column
    static const unsigned int offset_coord_lookup[N*N][2];               // input offset to obtain it's coord [i][j]
    static const unsigned int coord_offset_lookup[N][N];                 // input coord to obtain it's offset in [0, N*N), same to row_index

    // Input coord to obtain pointer offsets of a relevent area -> offset in [0, N*N).
    // The last dimension is the pointer offsets for the relevent area. 
    // For example, given a cell at [<any row>][col], 
    // we can obtain the pointer offsets for all elements in it's column 
    // by querying: col_index[col][...]
    static const unsigned int row_index[N][N];                       // pointer position (offset) for each row, input 1D row index [i]
    static const unsigned int col_index[N][N];                       // pointer position for each column, input 1D column index [j]
    static const unsigned int grid_index[NG][NG][N];                 // pointer position for each grid, input 2D grid coord [g_i][g_j]
    static const unsigned int grid_coord_index[N][N][N];             // pointer position for each grid, input 2D cell coord [i][j]
    static const unsigned int neighbor_index[N][N][N_NEIGHBORS];     // pointer position for each neighbor, input 2D cell coord [i][j]

    // static const std::array<std::array<unsigned int, 2>, util::n_combinations<N, 2>>
    static const unsigned int subunit_combinations_2[const_n_combinations_2(N)][2];  // all combinations of 2 elements in [0, N)

    // static const std::array<std::array<unsigned int, 2>, util::n_combinations<NV, 2>>
    static const unsigned int subvalue_combinations_2[const_n_combinations_2(NV)][2];  // all combinations of 2 elements in [0, NV)
};