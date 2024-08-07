#include "board.h"
#include "config.h"
#include "util.h"
#include <fstream>
#include <vector>

void Board::clear(val_t val = 0)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            m_board[i][j] = val;
        }
    }
};

Board::Board() { clear(); };
Board::~Board() {};

val_t Board::get(int col, int row) const
{
    ASSERT(col >= 0 && col < BOARD_SIZE, "column out of bounds: " + std::to_string(col));
    ASSERT(row >= 0 && row < BOARD_SIZE, "row out of bounds: " + std::to_string(row));
    return m_board[col][row];
};

void Board::set(int col, int row, val_t value)
{
    ASSERT(col >= 0 && col < BOARD_SIZE, "column out of bounds: " + std::to_string(col));
    ASSERT(row >= 0 && row < BOARD_SIZE, "row out of bounds: " + std::to_string(row));
    ASSERT(row >= 0 && row < BOARD_SIZE, "value out of bounds: " + std::to_string(value));
    m_board[col][row] = value;
};

void Board::load_from_file(const std::string& filename)
{
    std::ifstream file(filename, std::ios::in);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    load_data(file);
    file.close();
}

void Board::save_to_file(const std::string& filename) const
{
    std::ofstream file(filename, std::ios::trunc);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    file << to_string_raw();
    file.close();
}

std::string Board::to_string() const
{
    return to_string_raw();
}

void Board::load_data(std::istream& is)
{
    std::string line;
    unsigned int row = 0;
    while (std::getline(is, line)){
        auto split = util::split_string(line, " ");
        if (split.size() == 0){
            continue;
        }

        if (split[split.size() - 1] == ""){
            split.pop_back();
        }

        ASSERT(split.size() == BOARD_SIZE, "in row " + std::to_string(row));
        ASSERT(row < BOARD_SIZE, "too many rows");

        for (int i = 0; i < BOARD_SIZE; i++){
            m_board[row][i] = static_cast<val_t>(std::stoi(split[i]));
        }
        row++;
    }
}

std::string Board::to_string_raw() const
{
    std::string result;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            result += std::to_string(m_board[i][j]);
            if (j < BOARD_SIZE - 1)
            {
                result += " ";
            }
        }
        result += "\n";
    }
    return result;
}

std::ostream& operator<<(std::ostream& os, const Board& board)
{
    os << board.to_string_raw();
    return os;
}