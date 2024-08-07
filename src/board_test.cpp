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

    int col=1;
    auto col_ptr = board.get_col(col);
    for (int i=0; i<BOARD_SIZE; i++){
        // this is correct
        *col_ptr.get()[i] = 2;
    }

    auto col_ptr2 = board.get_col(col + 1);
    for (int i=0; i<BOARD_SIZE; i++){
        // this is wrong!
        // val_t* x = *col_ptr2;
        // x[i] = 3;

        // this is also wrong!
        // (*col_ptr2.get())[i] = 3;

        // this is correct
        val_t** y = col_ptr2.get();
        *y[i] = 3;      // equivalent to *(y[i]) = 3;
    }


    auto grid_ptr = board.get_grid(1, 1);
    for (int i=0; i<BOARD_SIZE; i++){
        // this is correct, equivalent to *(grid_ptr.get()[i]) = 4;
        *(grid_ptr.get())[i] = 4;
    }

    auto grid_ptr2 = board.get_grid(2, 2);
    for (int i=0; i<BOARD_SIZE; i++){
        // this is wrong!
        (*grid_ptr2.get())[i] = 5;
    }

    board.save_to_file("./output/2.txt");
    std::cout << board << std::endl;

    return 0;
};
