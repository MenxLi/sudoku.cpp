#include "board.h"
#include "config.h"
#include "util.h"
#include <sstream>
#include <fstream>
#include <memory>
#include <vector>
#include <cstring>

void Board::clear(val_t val = 0)
{
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            m_board[i][j] = val;
        }
    }
};

Board::Board() {indexer.init();};
Board::~Board() {};
Board::Board(const Board& other):Board() { load_data(other); };

void Board::set(unsigned int offset, val_t value)
{
    ASSERT(offset < BOARD_SIZE * BOARD_SIZE, "offset out of bounds: " + std::to_string(offset));
    *(&m_board[0][0] + offset) = value;
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
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            if (m_board[i][j] == 0)
            {
                return false;
            }
        }
    }
    return true;
};

bool Board::is_valid(bool check_filled)
{
    auto check_validity = [this, check_filled](
        unsigned int * offsets, unsigned int size
    )->bool{
        auto found = std::unique_ptr<bool[]>(new bool[size]{false});
        for (unsigned int i = 0; i < BOARD_SIZE; i++){
            const unsigned int offset = offsets[i];
            val_t v = this->get(offset);
            if (check_filled && v == 0){ // not filled
                return false;
            }
            if (v < 0 || v > BOARD_SIZE){ // invalid value
                return false;
            }

            if (v==0) continue;
            if (found[v - 1]){ // duplicate
                return false;
            }
            found[v - 1] = true;
        }
        return true;
    };

    for (unsigned int i = 0; i < BOARD_SIZE; i++){
        if (!check_validity(
            indexer.row_index[i], BOARD_SIZE
        )) return false;
    }
    for (unsigned int i = 0; i < BOARD_SIZE; i++){
        if (!check_validity(
            indexer.col_index[i], BOARD_SIZE
        )) return false;
    }
    for (unsigned int i = 0; i < GRID_SIZE; i++){
        for (unsigned int j = 0; j < GRID_SIZE; j++){
            if (!check_validity(
                indexer.grid_index[i][j], GRID_SIZE*GRID_SIZE
            )) return false;
        }
    }
    return true;
};

bool Board::is_solved(){ return is_valid(true); };

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
    for (unsigned int i = 0; i < BOARD_SIZE; i++){
        ASSERT(data.size() == BOARD_SIZE, "invalid data column size");
        for (unsigned int j = 0; j < BOARD_SIZE; j++){
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

        for (unsigned int i = 0; i < BOARD_SIZE; i++){
            m_board[row][i] = static_cast<val_t>(std::stoi(split[i]));
        }
        row++;
    }
}

void Board::load_data(const Board& board)
{
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            m_board[i][j] = board.get(i, j);
        }
    }
}

std::string Board::to_string_raw() const
{
    std::string result;
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
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
CandidateBoard::CandidateBoard(const CandidateBoard& other){
    // std::memcpy(m_candidates, other.m_candidates, sizeof(m_candidates));
    for (unsigned int i = 0; i < BOARD_SIZE; i++){
        for (unsigned int j = 0; j < BOARD_SIZE; j++){
            for (unsigned int k = 0; k < CANDIDATE_SIZE; k++){
                m_candidates[i][j][k] = other.m_candidates[i][j][k];
            }
        }
    }
}
CandidateBoard& CandidateBoard::operator=(const CandidateBoard &other){
    if (this == &other){ return *this; }
    for (unsigned int i = 0; i < BOARD_SIZE; i++){
        for (unsigned int j = 0; j < BOARD_SIZE; j++){
            for (unsigned int k = 0; k < CANDIDATE_SIZE; k++){
                m_candidates[i][j][k] = other.m_candidates[i][j][k];
            }
        }
    }
    return *this;
}

void CandidateBoard::reset(){
    indexer.init();
    for (unsigned int i = 0; i < BOARD_SIZE; i++){
        for (unsigned int j = 0; j < BOARD_SIZE; j++){
            for (unsigned int k = 0; k < CANDIDATE_SIZE; k++){
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
    for (unsigned int i = 0; i < CANDIDATE_SIZE; i++){
        count += m_candidates[row][col][i];
    }
    ASSERT_CANDIDATE_COUNT_THROW(count)
    return count;
}

bool CandidateBoard::remain_0(int row, int col) const{
    ASSERT_COORD_BOUNDS(row, col)
    const bool_ aim[CANDIDATE_SIZE] = {0};
    return std::memcmp(m_candidates[row][col], aim, CANDIDATE_SIZE * sizeof(bool_)) == 0;
}

bool CandidateBoard::remain_0(unsigned int offset) const{
    unsigned int row = indexer.offset_coord_lookup[offset][0];
    unsigned int col = indexer.offset_coord_lookup[offset][1];
    return remain_0(row, col);
}

OpState CandidateBoard::remain_x(unsigned int offset, unsigned int count, val_t* buffer) const{
    unsigned int row = indexer.offset_coord_lookup[offset][0];
    unsigned int col = indexer.offset_coord_lookup[offset][1];
    return remain_x(row, col, count, buffer);
};

OpState CandidateBoard::remain_x(int row, int col, unsigned int count, val_t* buffer) const{
    ASSERT_COORD_BOUNDS(row, col);
    unsigned int counter = 0;
    for (unsigned int i = 0; i < CANDIDATE_SIZE; i++){
        if (m_candidates[row][col][i]){
            if (counter >= count){
                // in-case of buffer overflow
                return OpState::FAIL;
            }
            *(buffer + counter) = i + 1;
            counter++;
        }
    }
    if (counter == 0) return OpState::VIOLATION;
    return counter == count? OpState::SUCCESS: OpState::FAIL;
}

val_t Board::operator[](Coord coord)
{
    return get(coord);
}
bool Board::operator==(const Board& other) const
{
    return std::memcmp(m_board, other.m_board, sizeof(m_board)) == 0;
}
std::ostream& operator<<(std::ostream& os, const Board& board)
{
    os << board.to_string_raw();
    return os;
}