#include "solver_v1.h"
#include <chrono>

void solve_for(int puzzle)
{
    Board board;
    board.load_from_file("./puzzles/" + std::to_string(puzzle) + ".txt");

    SolverV1 solver(board);

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    bool solved = solver.solve();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time elapsed: " 
        << std::chrono::duration_cast<std::chrono::microseconds>( end - begin).count()
        << " [µs] ";
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