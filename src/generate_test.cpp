#include "generate.h"
#include <iostream>

int main(){
    Board board;
    generate::fill_valid_board(board);
    std::cout << "Filled board:" << std::endl;
    std::cout << board << std::endl;

    unsigned int n_clues_to_remove = 81-20;
    bool generated = generate::remove_clues_by_solve(board, n_clues_to_remove);
    if (!generated){
        std::cout << "Failed to generate a board with " << n_clues_to_remove << " clues removed" << std::endl;
        return 1;
    }
    std::cout << "Board with clues removed:" << std::endl;
    std::cout << board << std::endl;
}