#include <iostream>
#include "board.h"
#include "config.h"


// https://stackoverflow.com/questions/19562103/uint8-t-cant-be-printed-with-cout
std::ostream& operator<<(std::ostream& os, const val_t val)
{
    os << static_cast<int>(val);
    return os;
}

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

    board.set(3, 4, 1);
    std::cout << board.get(3, 4) << std::endl;

    int row=1;
    for (int col=0; col<BOARD_SIZE; col++){
        board.set(row, col, 1);
    }

    auto row_ptr = board.get_row(row + 1);
    for (int i=0; i<BOARD_SIZE; i++){
        *(row_ptr.get())[i] = 2;
    }

    int col=2;
    auto col_ptr = board.get_col(col);
    for (int i=0; i<BOARD_SIZE; i++){
        *col_ptr.get()[i] = 3;
    }

    auto grid_ptr = board.get_grid(1, 1);
    for (int i=0; i<BOARD_SIZE; i++){
        *(grid_ptr.get())[i] = 4;
    }

    board.save_to_file("./output/2.txt");

    return 0;
};
