/*
This Board class is responsible for storing the state of the game board, and for
providing methods to read / dump the board state.
*/

#pragma once
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include "indexer.hpp"
#include "config.h"

#define ASSERT_COORD_BOUNDS(coord_row, coord_col) \
    ASSERT(static_cast<unsigned int>(coord_row) >= 0 && static_cast<unsigned int>(coord_row) < BOARD_SIZE, "row out of bounds: " + std::to_string(coord_row)); \
    ASSERT(static_cast<unsigned int>(coord_col) >= 0 && static_cast<unsigned int>(coord_col) < BOARD_SIZE, "column out of bounds: " + std::to_string(coord_col));
#define ASSERT_CANDIDATE_BOUNDS(row, col, value) \
    ASSERT(static_cast<unsigned int>(row) >= 0 && static_cast<unsigned int>(row) < BOARD_SIZE, "row out of bounds: " + std::to_string(row)); \
    ASSERT(static_cast<unsigned int>(col) >= 0 && static_cast<unsigned int>(col) < BOARD_SIZE, "column out of bounds: " + std::to_string(col)); \
    ASSERT(static_cast<unsigned int>(value) >= 0 && static_cast<unsigned int>(value) <= CANDIDATE_SIZE, "value out of bounds: " + std::to_string(value)); // value is 1-based, but we allow 0 to indicate empty


struct Coord
{
    int row;
    int col;
};

class Board
{
public:
    inline static Indexer<GRID_SIZE> indexer;
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

    void set(unsigned int offset, val_t value);
    void set(int row, int col, val_t value);
    void set(const Coord& coord, val_t value);

    // check if the board is valid, 
    // the board should be all filled with valid values
    bool is_valid(bool check_filled = false);
    bool is_solved();           // equal to is_valid(true)
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
    bool operator==(const Board& other) const;

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
    CandidateBoard(const CandidateBoard& other);
    CandidateBoard& operator=(const CandidateBoard& other);
    inline bool_& get_(int row, int col, val_t value);
    inline bool_* get(int row, int col);
    inline bool_* get(int idx);

    void load(const CandidateBoard& board);

    void reset();
    unsigned int count(int row, int col) const;

    bool remain_0(int row, int col) const;
    bool remain_0(unsigned int offset) const;

    // return if the cell has only count candidates left
    // and store the candidates in the buffer
    OpState remain_x(int row, int col, unsigned int count, val_t* buffer) const;

    OpState remain_x(unsigned int offset, unsigned int count, val_t* buffer) const;

private:
    // one-hot encoding of the candidates
    bool_ m_candidates[BOARD_SIZE][BOARD_SIZE][CANDIDATE_SIZE];
};

bool_& CandidateBoard::get_(int row, int col, val_t value){
    ASSERT_CANDIDATE_BOUNDS(row, col, value)
    return m_candidates[row][col][value - 1];
}

bool_* CandidateBoard::get(int row, int col){
    return m_candidates[row][col];
}

bool_* CandidateBoard::get(int offset){
    // return m_candidates[idx / BOARD_SIZE][idx % BOARD_SIZE];
    return &m_candidates[0][0][0] + offset * CANDIDATE_SIZE;
}