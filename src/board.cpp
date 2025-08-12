#include "board.h"
#include "config.h"
#include "util.h"
#include <sstream>
#include <fstream>
#include <memory>
#include <vector>
#include <cstring>

void Board::set(unsigned int offset, val_t value)
{
    ASSERT(offset < BOARD_SIZE * BOARD_SIZE, "offset out of bounds: " + std::to_string(offset));
    (data() + offset)->assign(value);
};

void Board::set(int row, int col, val_t value)
{
    ASSERT_CANDIDATE_BOUNDS(row, col, value)
    m_board[row][col].assign(value);
};

void Board::set(const Coord& coord, val_t value)
{
    set(coord.row, coord.col, value);
};

bool Board::is_filled() noexcept
{
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            if (!m_board[i][j].is_solved())
            {
                return false;
            }
        }
    }
    return true;
};

bool Board::is_valid(bool check_filled) noexcept
{
    auto check_validity = [this, check_filled](
        const unsigned int * offsets, unsigned int size
    )->bool{
        auto found = std::unique_ptr<bool[]>(new bool[CANDIDATE_SIZE]{false});

        for (unsigned int i = 0; i < size; i++){
            const unsigned int offset = offsets[i];
            auto& cell = this->get(offset);
            bool is_cell_solved = cell.is_solved();
            if (check_filled && !is_cell_solved){ // not filled
                // std::cout << "not filled" << std::endl;
                return false;
            }

            if (!is_cell_solved) continue;
            auto v = cell.retrive_nocheck();
            if (found[v - 1]){ // duplicate
                // std::cout << "duplicate" << std::endl;
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

void Board::save_to_file(const std::string& filename)
{
    std::ofstream file(filename, std::ios::trunc);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    file << to_string_raw();
    file.close();
}

std::string Board::to_string()
{
    return to_string_raw();
}

void Board::load_data(const std::vector<std::vector<val_t>> data){
    ASSERT(data.size() == BOARD_SIZE, "invalid data row size");
    for (unsigned int i = 0; i < BOARD_SIZE; i++){
        ASSERT(data.size() == BOARD_SIZE, "invalid data column size");
        for (unsigned int j = 0; j < BOARD_SIZE; j++){
            m_board[i][j].assign(data[i][j]);
        }
    }
}

void Board::load_data(const std::vector<val_t> data){
    ASSERT(data.size() == BOARD_SIZE * BOARD_SIZE, "invalid data size");
    for (unsigned int i = 0; i < BOARD_SIZE; i++){
        for (unsigned int j = 0; j < BOARD_SIZE; j++){
            m_board[i][j].assign(data[i * BOARD_SIZE + j]);
        }
    }
}

void Board::load_data(const std::string& str_data){
    std::vector<std::string> elements;

    auto split_lines = util::split_string(str_data, "\n");
    for (auto line: split_lines){
        auto line_sp = util::split_string(line, " ");
        for (auto s: line_sp){
            if (s != ""){ elements.push_back(s); }
        }
    }

    if (elements.size() != BOARD_SIZE * BOARD_SIZE){
        throw std::runtime_error("invalid data size, the board is supposed to be " + std::to_string(BOARD_SIZE) + "x" + std::to_string(BOARD_SIZE));
    }

    for (unsigned int i = 0; i < BOARD_SIZE; i++){
        for (unsigned int j = 0; j < BOARD_SIZE; j++){
            m_board[i][j].assign(static_cast<val_t>(std::stoi(elements[i * BOARD_SIZE + j])));
        }
    }

}

void Board::load_data(std::istream& is)
{
    std::string content;
    for (std::string line; std::getline(is, line);)
    {
        content += line + "\n";
    }
    load_data(content);
}

void Board::load_data(const Board& board)
{
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            m_board[i][j] = board.m_board[i][j];
        }
    }
}

std::string Board::to_string_raw()
{
    std::string result;
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            result += std::to_string(m_board[i][j].retrive());
            if (j < BOARD_SIZE - 1)
            {
                result += " ";
            }
        }
        result += "\n";
    }
    return result;
}

void BoardEquivalenceTransform::swap_row(Board& board, unsigned int row1, unsigned int row2)
{
    if (row1 == row2) return;
    ASSERT(row1 < BOARD_SIZE && row2 < BOARD_SIZE, "Invalid row index");
    for (unsigned int j = 0; j < BOARD_SIZE; j++)
    {
        auto temp = board.get(row1, j);
        board.set(row1, j, board.get(row2, j));
        board.set(row2, j, temp);
    }
}

void BoardEquivalenceTransform::swap_row(Board& board, unsigned int band, unsigned int band_row1, unsigned int band_row2)
{
    if (band_row1 == band_row2) return;
    ASSERT(band_row1 < GRID_SIZE && band_row2 < GRID_SIZE && band < GRID_SIZE, "Invalid band index");
    unsigned int row1 = band * GRID_SIZE + band_row1;
    unsigned int row2 = band * GRID_SIZE + band_row2;
    swap_row(board, row1, row2);
}

void BoardEquivalenceTransform::swap_band(Board& board, unsigned int band1, unsigned int band2)
{
    if (band1 == band2) return;
    ASSERT(band1 < GRID_SIZE && band2 < GRID_SIZE, "Invalid band index");
    for (unsigned int i = 0; i < GRID_SIZE; i++)
    {
        unsigned int row1 = band1 * GRID_SIZE + i;
        unsigned int row2 = band2 * GRID_SIZE + i;
        swap_row(board, row1, row2);
    }
}

void BoardEquivalenceTransform::swap_value(Board& board, val_t value1, val_t value2)
{
    if (value1 == value2) return;
    ASSERT(value1 <= CANDIDATE_SIZE && value2 <= CANDIDATE_SIZE, "Invalid value");
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            val_t value = board.get(i, j).retrive();
            if (value == value1)
            {
                board.set(i, j, value2);
            }
            else if (value == value2)
            {
                board.set(i, j, value1);
            }
        }
    }
}

void BoardEquivalenceTransform::transpose(Board& board)
{
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = i + 1; j < BOARD_SIZE; j++)
        {
            auto temp = board.get(i, j);
            board.set(i, j, board.get(j, i));
            board.set(j, i, temp);
        }
    }
}

#define ASSERT_CANDIDATE_COUNT_THROW(count) \
    if (count == 0){ throw std::runtime_error("no candidate found for this cell, invalid board or candidate not initialized"); }

// unsigned int CandidateBoard::count(int row, int col) const{
//     ASSERT_COORD_BOUNDS(row, col)
//     unsigned int count = 0;
//     for (unsigned int i = 0; i < CANDIDATE_SIZE; i++){
//         count += m_candidates[row][col][i];
//     }
//     ASSERT_CANDIDATE_COUNT_THROW(count)
//     return count;
// }

// bool CandidateBoard::remain_0(int row, int col) const{
//     ASSERT_COORD_BOUNDS(row, col)
//     const bool_ aim[CANDIDATE_SIZE] = {0};
//     return std::memcmp(m_candidates[row][col], aim, CANDIDATE_SIZE * sizeof(bool_)) == 0;
// }

// bool CandidateBoard::remain_0(unsigned int offset) const{
//     unsigned int row = indexer.offset_coord_lookup[offset][0];
//     unsigned int col = indexer.offset_coord_lookup[offset][1];
//     return remain_0(row, col);
// }

// OpState CandidateBoard::remain_x(unsigned int offset, unsigned int count, val_t* buffer) const{
//     unsigned int row = indexer.offset_coord_lookup[offset][0];
//     unsigned int col = indexer.offset_coord_lookup[offset][1];
//     return remain_x(row, col, count, buffer);
// };

// OpState CandidateBoard::remain_x(int row, int col, unsigned int count, val_t* buffer) const{
//     ASSERT_COORD_BOUNDS(row, col);
//     unsigned int counter = 0;
//     for (unsigned int i = 0; i < CANDIDATE_SIZE; i++){
//         if (m_candidates[row][col][i]){
//             if (counter >= count){
//                 // in-case of buffer overflow
//                 return OpState::FAIL;
//             }
//             *(buffer + counter) = i + 1;
//             counter++;
//         }
//     }
//     if (counter == 0) return OpState::VIOLATION;
//     return counter == count? OpState::SUCCESS: OpState::FAIL;
// }

bool Board::operator==(const Board& other) const
{
    return std::memcmp(m_board, other.m_board, sizeof(m_board)) == 0;
}
std::ostream& operator<<(std::ostream& os, Board& board)
{
    os << board.to_string_raw();
    return os;
}