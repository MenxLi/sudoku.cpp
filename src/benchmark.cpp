/*
Maybe here is no need to do such benchmarking, 
because the code may be optimized by the compiler, 
and the results may not be accurate...
*/
#include <chrono>
#include "solver_v1.h"
#include <fstream>
#include <ostream>

std::chrono::duration<double> solve_for(std::string file_content)
{
    Board board;
    board.load_data(file_content);
    // std::cout << std::endl << file_content << std::endl << std::endl;

    // std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    SolverV1 solver(board);
    solver.set_board(board);
    solver.solve();
    auto end = std::chrono::high_resolution_clock::now();

    return end - start;
}

int main()
{
    int n_repeats = 100;
    const int n_puzzles = 7;

    std::cout << "Benchmarking " << n_puzzles << " puzzles with " << n_repeats << " repeats..." << std::endl;
    for (int i = 0; i < n_puzzles; i++)
    {
        std::string filename = "puzzles/" + std::to_string(i+1) + ".txt";
        std::string file_content;
        std::ifstream file(filename, std::ios::in);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        file_content.assign((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
        std::chrono::duration<double> total_time = std::chrono::duration<double>::zero();
        std::cout << "Puzzle " << i+1 << ": " ;
        for (int j = 0; j < n_repeats; j++)
        {
            total_time += solve_for(file_content);
        }
        unsigned long time = std::chrono::duration_cast<std::chrono::microseconds>(total_time).count() / n_repeats;
        // format to 5 characters by adding leading blanks
        std::string time_str = std::to_string(time);
        if (time_str.size() < 5)
        {
            time_str.insert(time_str.begin(), 5 - time_str.size(), ' ');
        }
        std::cout << time_str << " [us]" << std::endl;
    }
}
