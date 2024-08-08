/*
This Board class is responsible for storing the state of the game board, and for
providing methods to read / dump the board state.
*/

#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include "config.h"

struct Coord
{
    int row;
    int col;
};

class Board
{
public:
    Board();
    ~Board();

    void clear(val_t val);

    val_t get(int row, int col) const;
    val_t get(const Coord& coord) const;
    val_t& get_(int row, int col);
    val_t& get_(const Coord& coord);

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
    void load_data(std::vector<std::vector<val_t>> data);
    void load_data(std::istream& is);
    void load_data(Board& board);
    void load_data(std::string& str_data);
    void load_from_file(const std::string& filename);
    void save_to_file(const std::string& filename) const;
    std::string to_string() const;

    val_t operator[](Coord coord);

    friend std::ostream& operator<<(std::ostream& os, const Board& board);

private:
    val_t m_board[BOARD_SIZE][BOARD_SIZE];
    std::string to_string_raw() const;
};
