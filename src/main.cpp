#include "solver_v1.h"

void solve_for(int puzzle)
{
    Board board;
    board.load_from_file("./puzzles/" + std::to_string(puzzle) + ".txt");

    SolverV1 solver(board);
    bool solved = solver.solve();

    std::cout << "Puzzle " << puzzle;
    if (solved)
    {
        std::cout << " Solved!" << std::endl;
    }
    else
    {
        std::cout << " Not solved." << std::endl;
    }

    board.save_to_file("./output/" + std::to_string(puzzle) + ".txt");
}

int main(){
    for (int i = 1; i <= 10; i++)
    {
        solve_for(i);
    }
}