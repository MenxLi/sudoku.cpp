#include "board.h"
#include "config.h"
#include "util.h"
#include <sstream>
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

Board::Board() {};
Board::~Board() {};
Board::Board(const Board& other) { load_data(other); };

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
    ASSERT_COORD_BOUNDS(row, col);
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
    ASSERT_CANDIDATE_BOUNDS(row, col, value)
    m_board[row][col] = value;
};

void Board::set(const Coord& coord, val_t value)
{
    set(coord.row, coord.col, value);
};

bool Board::is_filled() const
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

bool Board::is_valid()
{
    auto check_validity = [](val_t** arr)->bool{
        std::vector<bool> found(BOARD_SIZE, false);
        for (int i = 0; i < BOARD_SIZE; i++){
            if (*arr[i] == 0){ // not filled
                return false;
            }
            if (found[*arr[i] - 1]){ // duplicate
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

bool Board::is_solved(){ return is_valid(); };

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

val_t* Board::data(){
    return &m_board[0][0];
}
void Board::load_data(const std::vector<std::vector<val_t>> data){
    ASSERT(data.size() == BOARD_SIZE, "invalid data row size");
    for (int i = 0; i < BOARD_SIZE; i++){
        ASSERT(data.size() == BOARD_SIZE, "invalid data column size");
        for (int j = 0; j < BOARD_SIZE; j++){
            m_board[i][j] = data[i][j];
        }
    }
}

void Board::load_data(const std::string& str_data){
    std::istringstream iss(str_data);
    load_data(iss);
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

void Board::load_data(const Board& board)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            m_board[i][j] = board.get(i, j);
        }
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

CandidateBoard::CandidateBoard(){
    reset();
}
CandidateBoard::CandidateBoard(CandidateBoard& other){
    for (int i = 0; i < BOARD_SIZE; i++){
        for (int j = 0; j < BOARD_SIZE; j++){
            for (int k = 0; k < CANDIDATE_SIZE; k++){
                m_candidates[i][j][k] = other.m_candidates[i][j][k];
            }
        }
    }
}

void CandidateBoard::reset(){
    indexer.init();
    for (int i = 0; i < BOARD_SIZE; i++){
        for (int j = 0; j < BOARD_SIZE; j++){
            for (int k = 0; k < CANDIDATE_SIZE; k++){
                m_candidates[i][j][k] = 1;
            }
        }
    }
}

#define ASSERT_CANDIDATE_COUNT_THROW(count) \
    if (count == 0){ throw std::runtime_error("no candidate found for this cell, invalid board or candidate not initialized"); }

unsigned int CandidateBoard::count(int row, int col) const{
    ASSERT_COORD_BOUNDS(row, col)
    unsigned int count = 0;
    for (int i = 0; i < CANDIDATE_SIZE; i++){
        if (m_candidates[row][col][i]){
            count++;
        }
    }
    ASSERT_CANDIDATE_COUNT_THROW(count)
    return count;
}

bool_ CandidateBoard::remain_x(unsigned int offset, unsigned int count, val_t* buffer) const{
    unsigned int row = indexer.offset_coord_lookup[offset][0];
    unsigned int col = indexer.offset_coord_lookup[offset][1];
    return remain_x(row, col, count, buffer);
};

bool_ CandidateBoard::remain_x(int row, int col, unsigned int count, val_t* buffer) const{
    ASSERT_COORD_BOUNDS(row, col);
    int counter = 0;
    for (int i = 0; i < CANDIDATE_SIZE; i++){
        if (m_candidates[row][col][i]){
            if (counter >= count){
                // in-case of buffer overflow
                return false;
            }
            *(buffer + counter) = i + 1;
            counter++;
        }
    }
    ASSERT_CANDIDATE_COUNT_THROW(counter)
    return counter == count;
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