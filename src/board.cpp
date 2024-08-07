#include "board.h"
#include "config.h"
#include "util.h"
#include <fstream>
#include <memory>
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

bool Board::is_solved() const
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (m_board[i][j] == 0)
            {
                return false;
            }
        }
    }
    return true;
};

Board::Board() { clear(); };
Board::~Board() {};

val_t Board::get(int row, int col) const
{
    ASSERT(row >= 0 && row < BOARD_SIZE, "row out of bounds: " + std::to_string(row));
    ASSERT(col >= 0 && col < BOARD_SIZE, "column out of bounds: " + std::to_string(col));
    return m_board[row][col];
};

val_t Board::get(const Coord& coord) const
{ 
    return get(coord.row, coord.col); 
};

val_t& Board::get_(int row, int col)
{
    ASSERT(row >= 0 && row < BOARD_SIZE, "row out of bounds: " + std::to_string(row));
    ASSERT(col >= 0 && col < BOARD_SIZE, "column out of bounds: " + std::to_string(col));
    return m_board[row][col];
};

val_t& Board::get_(const Coord& coord)
{
    return get_(coord.row, coord.col);
};

std::unique_ptr<val_t*> Board::get_row(int row)
{
    ASSERT(row >= 0 && row < BOARD_SIZE, "row out of bounds: " + std::to_string(row));
    std::unique_ptr<val_t*> result(new val_t*[BOARD_SIZE]);
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        result.get()[i] = &m_board[row][i];
    }
    return result;
};

std::unique_ptr<val_t*> Board::get_col(int col)
{
    ASSERT(col >= 0 && col < BOARD_SIZE, "column out of bounds: " + std::to_string(col));
    std::unique_ptr<val_t*> result(new val_t*[BOARD_SIZE]);
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        result.get()[i] = &m_board[i][col];
    }
    return result;
};

std::unique_ptr<val_t*> Board::get_grid(int row, int col){
    ASSERT(row >= 0 && row < GRID_SIZE, "grid row out of bounds: " + std::to_string(row));
    ASSERT(col >= 0 && col < GRID_SIZE, "grid col out of bounds: " + std::to_string(col));
    std::unique_ptr<val_t*> result(new val_t*[BOARD_SIZE]);
    for (int i = 0; i < GRID_SIZE; i++){
        for (int j = 0; j < GRID_SIZE; j++){
            result.get()[i * GRID_SIZE + j] = &m_board[row * GRID_SIZE + i][col * GRID_SIZE + j];
        }
    }
    return result;
};

void Board::set(int row, int col, val_t value)
{
    ASSERT(row >= 0 && row < BOARD_SIZE, "row out of bounds: " + std::to_string(row));
    ASSERT(col >= 0 && col < BOARD_SIZE, "column out of bounds: " + std::to_string(col));
    ASSERT(col >= 0 && col < BOARD_SIZE, "value out of bounds: " + std::to_string(value));
    m_board[row][col] = value;
};

void Board::set(const Coord& coord, val_t value)
{
    set(coord.row, coord.col, value);
};


bool Board::is_valid()
{
    auto check_validity = [](val_t** arr)->bool{
        std::vector<bool> found(BOARD_SIZE, false);
        for (int i = 0; i < BOARD_SIZE; i++){
            if (*arr[i] == 0){ // not filled
                std::cout << "not filled" << std::endl;
                return false;
            }
            if (found[*arr[i] - 1]){ // duplicate
                std::cout << "duplicate" << std::endl;
                return false;
            }
            found[*arr[i] - 1] = true;
        }
        return true;
    };

    for (unsigned int i = 0; i < BOARD_SIZE; i++){
        if (!check_validity(get_row(i).get())) return false;
    }
    for (unsigned int i = 0; i < BOARD_SIZE; i++){
        if (!check_validity(get_col(i).get())) return false;
    }
    for (unsigned int i = 0; i < GRID_SIZE; i++){
        for (unsigned int j = 0; j < GRID_SIZE; j++){
            if (!check_validity(get_grid(i, j).get())) return false;
        }
    }
    return true;
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

val_t Board::operator[](Coord coord)
{
    return get(coord);
}

std::ostream& operator<<(std::ostream& os, const Board& board)
{
    os << board.to_string_raw();
    return os;
}