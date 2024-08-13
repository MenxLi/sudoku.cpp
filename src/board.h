/*
This Board class is responsible for storing the state of the game board, and for
providing methods to read / dump the board state.
*/

#pragma once
#include <iostream>
#include <ostream>
#include <string>
#include <memory>
#include <vector>
#include "indexer.hpp"
#include "config.h"

#define CANDIDATE_SIZE BOARD_SIZE

#define ASSERT_COORD_BOUNDS(coord_row, coord_col) \
    ASSERT(coord_row >= 0 && coord_row < BOARD_SIZE, "row out of bounds: " + std::to_string(coord_row)); \
    ASSERT(coord_col >= 0 && coord_col < BOARD_SIZE, "column out of bounds: " + std::to_string(coord_col));
#define ASSERT_CANDIDATE_BOUNDS(row, col, value) \
    ASSERT(row >= 0 && row < BOARD_SIZE, "row out of bounds: " + std::to_string(row)); \
    ASSERT(col >= 0 && col < BOARD_SIZE, "column out of bounds: " + std::to_string(col)); \
    ASSERT(value >= 0 && value <= CANDIDATE_SIZE, "value out of bounds: " + std::to_string(value)); // value is 1-based, but we allow 0 to indicate empty


struct Coord
{
    int row;
    int col;
};

class Board
{
public:
    Board();
    Board(const Board& other);
    ~Board();

    void clear(val_t val);

    inline val_t get(unsigned int idx);
    inline val_t get(int row, int col) const;
    inline val_t get(const Coord& coord) const;
    inline val_t& get_(unsigned int idx);
    inline val_t& get_(int row, int col);
    inline val_t& get_(const Coord& coord);

    // NOTE:
    // the following methods return a unique_ptr to an array of pointers to the values
    // the pointers are not ordered contiguously in memory, 
    // when querying the values, you should use the unique_ptr.get() method to obtain the val_t**
    // and indexing with: unique_ptr.get()[i] to obtain the val_t* at index i
    // instead of dereferencing the unique_ptr directly, i.e. 
    // use: unique_ptr.get()[i] instead of (*unique_ptr)[i], 
    std::unique_ptr<val_t*> get_row(int row);
    std::unique_ptr<val_t*> get_col(int col);
    std::unique_ptr<val_t*> get_grid(int grid_row, int grid_col);
    std::unique_ptr<val_t*> get_grid(const Coord& coord);

    void set(int row, int col, val_t value);
    void set(const Coord& coord, val_t value);

    // check if the board is valid, 
    // the board should be all filled with valid values
    bool is_valid();
    bool is_solved();           // equal to is_valid()
    bool is_filled() const;     // check if the board is filled, i.e. no empty cells

    // val_t(*data())[BOARD_SIZE];
    val_t* data();                      // return a pointer to the raw data
    void load_data(const std::vector<std::vector<val_t>> data);
    void load_data(std::istream& is);
    void load_data(const Board& board);
    void load_data(const std::string& str_data);
    void load_from_file(const std::string& filename);
    void save_to_file(const std::string& filename) const;
    std::string to_string() const;

    val_t operator[](Coord coord);

    friend std::ostream& operator<<(std::ostream& os, const Board& board);

private:
    val_t m_board[BOARD_SIZE][BOARD_SIZE];
    std::string to_string_raw() const;
};

val_t Board::get(unsigned int idx)
{
    ASSERT(idx < BOARD_SIZE * BOARD_SIZE, "index out of bounds: " + std::to_string(idx));
    return *(data() + idx);
};

val_t& Board::get_(unsigned int idx)
{
    ASSERT(idx < BOARD_SIZE * BOARD_SIZE, "index out of bounds: " + std::to_string(idx));
    return *(data() + idx);
};

val_t Board::get(int row, int col) const
{
    ASSERT_COORD_BOUNDS(row, col);
    return m_board[row][col];
};

val_t Board::get(const Coord& coord) const
{ 
    return get(coord.row, coord.col); 
};

val_t& Board::get_(int row, int col)
{
    ASSERT_COORD_BOUNDS(row, col);
    return m_board[row][col];
};

val_t& Board::get_(const Coord& coord)
{
    return get_(coord.row, coord.col);
};


/*
cadidate refers to the possible values for a cell, 
based on the values of other cells in the same row, column, and grid, 
it serves as a draft for the actual value of the cell when solving the puzzle
*/
typedef uint8_t bool_;
class CandidateBoard
{
public:
    inline static Indexer<GRID_SIZE> indexer;
    CandidateBoard();
    CandidateBoard(CandidateBoard& other);
    inline bool_& get_(int row, int col, val_t value);
    inline bool_* get(int row, int col);
    inline bool_* get(int idx);

    void reset();
    unsigned int count(int row, int col) const;

    // return if the cell has only count candidates left
    // and store the candidates in the buffer
    bool_ remain_x(int row, int col, unsigned int count, val_t* buffer) const;

    bool_ remain_x(unsigned int idx, unsigned int count, val_t* buffer) const;

private:
    bool_ m_candidates[BOARD_SIZE][BOARD_SIZE][CANDIDATE_SIZE];
};

bool_& CandidateBoard::get_(int row, int col, val_t value){
    ASSERT_CANDIDATE_BOUNDS(row, col, value)
    return m_candidates[row][col][value - 1];
}

bool_* CandidateBoard::get(int row, int col){
    return m_candidates[row][col];
}

bool_* CandidateBoard::get(int idx){
    // return m_candidates[idx / BOARD_SIZE][idx % BOARD_SIZE];
    return &m_candidates[0][0][0] + idx * CANDIDATE_SIZE;
}