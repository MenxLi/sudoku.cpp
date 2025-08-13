/*
This Board class is responsible for storing the state of the game board, and for
providing methods to read / dump the board state.
*/

#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <bitset>
#include "indexer.h"
#include "config.h"

#define ASSERT_COORD_BOUNDS(coord_row, coord_col) \
    ASSERT(static_cast<unsigned int>(coord_row) < BOARD_SIZE, "row out of bounds: " + std::to_string(coord_row)); \
    ASSERT(static_cast<unsigned int>(coord_col) < BOARD_SIZE, "column out of bounds: " + std::to_string(coord_col));
#define ASSERT_CANDIDATE_BOUNDS(row, col, value) \
    ASSERT(static_cast<unsigned int>(row) < BOARD_SIZE, "row out of bounds: " + std::to_string(row)); \
    ASSERT(static_cast<unsigned int>(col) < BOARD_SIZE, "column out of bounds: " + std::to_string(col)); \
    ASSERT(static_cast<unsigned int>(value) <= CANDIDATE_SIZE, "value out of bounds: " + std::to_string(value)); // value is 1-based, but we allow 0 to indicate empty


struct Coord
{
    int row;
    int col;
};

// one-hot encoding of the candidates
// N is the number of candidates, e.g. 9 for Sudoku
class Cell{

public:
    static const unsigned int N = CANDIDATE_SIZE; // number of candidates

    typedef std::bitset<N> bit_t;

    constexpr explicit Cell(): m_bitmask(){
        m_bitmask.set(); // default to all candidates available (empty cell)
    }
    explicit Cell(const bit_t& init_bitmask): m_bitmask(init_bitmask) {}
    explicit Cell(unsigned int init_value): m_bitmask() { assign(init_value); }

    /*
    Retrieves the value of the cell.
    If exactly one bit is set, it returns the index of that bit + 1 (1-based index).
    If no bits are set, it returns 0.
    (TODO: may optimize to lookup?)
     */
    inline val_t retrive()
    {
        if (!is_solved()) { return 0; }
        return retrive_nocheck();
    }
    val_t retrive_nocheck() const
    {
        for (val_t i = 0; i < N; ++i) {
            if (m_bitmask.test(i)) {
                return i + 1;
            }
        }
        throw std::runtime_error("Cell is not solved, no candidate found");
    }

    // Assigns a value to the cell.
    // If the value is 0, it resets the cell (all bits to 0).
    // If the value is between 1 and N, it sets the corresponding bit.
    inline void assign(val_t val) { 
        if (val == 0) {
            m_bitmask.set(); // reset the cell
        } else {
            m_bitmask.reset(); 
            m_bitmask.set(val - 1); // set the bit for the value
            return;
        }
    }

    bit_t bitmask() const { return m_bitmask; }

    inline bool operator[](unsigned int idx) const
        { return test(idx); }
    
    inline void set(unsigned int idx)
        { m_bitmask.set(idx, true); }
    
    inline void set()
        { m_bitmask.set(); }

    inline void reset(unsigned int idx)
        { m_bitmask.set(idx, false); }
    
    inline void reset()
        { m_bitmask.reset(); }

    inline bool test(unsigned int idx) const
        { return m_bitmask.test(idx); }
    
    inline bool is_empty() const
        { return m_bitmask == 0; }
    
    inline bool is_full() const
        { return m_bitmask == bit_t().set(); }
    
    inline bool is_solved() const
        { return count() == 1; }
    
    inline size_t count() const { 
        return m_bitmask.count();
    }
    
    inline Cell operator&(const Cell& other) const
        { return Cell(m_bitmask & other.m_bitmask); }
    
    inline Cell operator|(const Cell& other) const
        { return Cell(m_bitmask | other.m_bitmask); }
    
    inline Cell operator~()  const
        {
            Cell result{this->m_bitmask};
            result.m_bitmask.flip();
            return result;
        }
    
    inline Cell& operator&=(const Cell& other) 
        { m_bitmask &= other.m_bitmask; return *this; }

private:
    bit_t m_bitmask;
};

class Board
{
public:
    inline static Indexer indexer;
    Board() = default;
    Board(const Board& other) = default;
    Board& operator=(const Board& other) = default;
    ~Board() = default;

    inline void clear() { 
        for (auto& row : m_board) 
        { 
            for (auto& cell : row) 
                cell.set();
        } 
    };

    inline Cell& get(unsigned int idx);
    inline Cell& get(int row, int col);
    inline Cell& get(const Coord& coord);

    void set(unsigned int offset, val_t value);
    void set(int row, int col, val_t value);
    void set(const Coord& coord, val_t value);
    void set(unsigned int offset, Cell c) {
        ASSERT(offset < BOARD_SIZE * BOARD_SIZE, "index out of bounds: " + std::to_string(offset));
        *(data() + offset) = c;
    };
    void set(int row, int col, Cell c){
        ASSERT_COORD_BOUNDS(row, col);
        m_board[row][col] = c;
    }
    void set(const Coord& coord, Cell c){
        set(coord.row, coord.col, c);
    }

    // check if the board is valid, 
    // the board should be all filled with valid values
    bool is_valid(bool check_filled = false) noexcept;
    bool is_filled() noexcept;    // check if the board is filled, i.e. no empty cells
    bool is_solved() noexcept {return is_valid(true);};

    void load_data(const std::vector<std::vector<val_t>> data);
    void load_data(const std::vector<val_t> data);
    void load_data(std::istream& is);
    void load_data(const Board& board);
    void load_data(const std::string& str_data);
    void load_from_file(const std::string& filename);
    void save_to_file(const std::string& filename);
    std::string to_string();

    inline Cell* data();                      // return a pointer to the raw data
    inline Cell operator[](Coord coord){ return get(coord); }; // allow board[{row, col}] to get the value
    bool operator==(const Board& other) const;

    friend std::ostream& operator<<(std::ostream& os, Board& board);

private:
    Cell m_board[BOARD_SIZE][BOARD_SIZE];
    std::string to_string_raw();
};

inline Cell* Board::data(){
    return &m_board[0][0];
}

inline Cell& Board::get(unsigned int idx)
{
    ASSERT(idx < BOARD_SIZE * BOARD_SIZE, "index out of bounds: " + std::to_string(idx));
    return *(data() + idx);
};

inline Cell& Board::get(int row, int col)
{
    ASSERT_COORD_BOUNDS(row, col);
    return m_board[row][col];
};


inline Cell& Board::get(const Coord& coord)
{
    return get(coord.row, coord.col);
};

class BoardEquivalenceTransform
{
public:
    // no column transformation, 
    // it's the same as: TRANSPOSE + SWAP_ROW / SWAP_BAND
    static void swap_row(Board& board, unsigned int band, unsigned int band_row1, unsigned int band_row2);
    static void swap_band(Board& board, unsigned int band1, unsigned int band2);
    static void swap_value(Board& board, val_t value1, val_t value2);
    static void transpose(Board& board);
private:
    static void swap_row(Board& board, unsigned int row1, unsigned int row2);
};
