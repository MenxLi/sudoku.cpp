#include <iostream>
#include "board.h"
#include "config.h"
#include <sstream>


// https://stackoverflow.com/questions/19562103/uint8-t-cant-be-printed-with-cout
std::ostream& operator<<(std::ostream& os, const val_t val)
{
    os << static_cast<int>(val);
    return os;
}

const std::string valid_board_str = \
"8 9 1 2 5 4 6 7 3\n"
"4 2 6 1 7 3 5 8 9\n"
"3 7 5 8 6 9 2 1 4\n"
"2 5 9 7 1 8 4 3 6\n"
"7 6 8 4 3 5 1 9 2\n"
"1 4 3 6 9 2 7 5 8\n"
"6 3 2 5 8 7 9 4 1\n"
"5 8 4 9 2 1 3 6 7\n"
"9 1 7 3 4 6 8 2 5\n"
;

int main()
{
    Board board;
    board.load_from_file("./puzzles/1.txt");
    std::cout << board << std::endl;

    board.clear(1);
    std::cout << board << std::endl;

    board.load_from_file("./puzzles/2.txt");
    std::cout << board << std::endl;

    std::cout << "--------" << std::endl;
    std::cout << board.get(3, 4) << std::endl;
    std::cout << board[{3, 4}] << std::endl;

    std::istringstream iss(valid_board_str);
    board = Board();
    board.load_data(iss);
    std::cout << "Board is valid: " << board.is_valid() << std::endl;
    std::cout << "Board is solved: " << board.is_filled() << std::endl;

    board.set(3, 4, 1);
    std::cout << board.get(3, 4) << std::endl;

    int row=1;
    for (unsigned int col=0; col<BOARD_SIZE; col++){
        board.set(row, col, 1);
    }

    board.save_to_file("./output/2.txt");
    std::cout << board << std::endl;

    return 0;
};
