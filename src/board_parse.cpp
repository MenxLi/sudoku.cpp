#include "board.h"

Board Board::from_string(
    const std::string& str_data, 
    std::string_view sp,
    std::string_view nl, 
    std::string_view empty
){
    auto board = Board();

    // the default case, no need to split
    if (sp == " " && nl == "\n") {  
        board.load_data(str_data);
        return board;
    }

    if (sp != "" && nl =="")
        throw std::runtime_error("Invalid separator, nl cannot be empty if sp is not empty"); 
    if (BOARD_SIZE > 16 && sp == "") 
        throw std::runtime_error("Invalid separator (empty) for large boards"); 

    // maybe replace nl with sp (move to mark str_data not use anymore)
    std::string sdata;
    nl != sp ? 
        sdata = util::replace_string(std::move(str_data), std::string(nl), std::string(sp)):
        sdata = std::move(str_data);
    
    // handle the case where sp is empty
    if (sp == "") {
        std::vector<val_t> board_data(CELL_COUNT);
        if (sdata.size() != CELL_COUNT) {
            throw std::runtime_error("Invalid data size, expected " + std::to_string(CELL_COUNT) + " values, got " + std::to_string(str_data.size()));
        }
        for (unsigned int i = 0; i < sdata.size(); i++) {
            char c = sdata[i];
            if (std::string(1, c) == empty) { board_data[i] = 0; } 
            else if (c >= '0' && c <= '9') { board_data[i] = static_cast<val_t>(c - '0'); } 
            else if (c >= 'a' && c <= 'f') { board_data[i] = static_cast<val_t>(c - 'a' + 10); } 
            else if (c >= 'A' && c <= 'F') { board_data[i] = static_cast<val_t>(c - 'A' + 10); } 
            else {
                throw std::runtime_error("Invalid character in input: " + std::string(1, c));
            }
        }
        board.load_data(std::move(board_data));
        return board;
    }

    // sp is not empty, split the string
    std::vector<std::string> vals = util::split_string(sdata, std::string(sp));
    if (vals.size() != CELL_COUNT) {
        throw std::runtime_error("Invalid data size, expected " + std::to_string(CELL_COUNT) + " values, got " + std::to_string(vals.size()));
    }

    std::vector<val_t> board_data(CELL_COUNT);
    for (unsigned int i = 0; i < CELL_COUNT; i++) {
        std::string c = vals[i];
        c == empty?
            board_data[i] = 0 : 
            board_data[i] = static_cast<val_t>(std::stoi(c));
    }
    board.load_data(board_data);
    return board;
}

std::string Board::to_string(
    std::string_view sp, 
    std::string_view nl, 
    std::string_view empty
) const
{
    std::string result;
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            val_t value = m_board[i][j].retrive();
            if (value == 0) {
                result += std::string(empty);
            } else {
                result += std::to_string(value);
            }
            if (j < BOARD_SIZE - 1)
            {
                result += sp;
            }
        }
        result += nl;
    }
    return result;
}

std::string Board::to_bitstring(std::string_view sp, std::string_view nl, std::string_view ld) const
{
    std::string result;
    for (unsigned int i = 0; i < BOARD_SIZE; i++)
    {
        for (unsigned int j = 0; j < BOARD_SIZE; j++)
        {
            result += ld; 
            result += m_board[i][j].bitmask().to_string();
            if (j < BOARD_SIZE - 1)
            {
                result += sp;
            }
        }
        result += nl;
    }
    return result;
}
