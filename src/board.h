/*
This Board class is responsible for storing the state of the game board, and for
providing methods to read / dump the board state.
*/

#pragma once
#include <iostream>
#include <string>
#include "config.h"

class Board
{
public:
    Board();
    ~Board();

    void clear(val_t val);

    val_t get(int col, int row) const;
    void set(int col, int row, val_t value);

    void load_from_file(const std::string& filename);
    void save_to_file(const std::string& filename) const;
    std::string to_string() const;

    friend std::ostream& operator<<(std::ostream& os, const Board& board);

private:
    val_t m_board[BOARD_SIZE][BOARD_SIZE];
    void load_data(std::istream& is);
    std::string to_string_raw() const;
};
