/*
Maybe here is no need to do such benchmarking, 
because the code may be optimized by the compiler, 
and the results may not be accurate...
*/
#include <chrono>
#include "solver_v1.h"
#include <fstream>
#include <ostream>
#include <algorithm>
#include <random>

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
    const int n_puzzles = 5;
    std::vector<std::string> file_contents(n_puzzles);
    std::vector<std::chrono::duration<double>> times(n_puzzles);

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
        file_contents.push_back(file_content);
        times.push_back(std::chrono::duration<double>::zero());
    }

    for (int j = 0; j < n_repeats; j++)
    {
        std::shuffle(file_contents.begin(), file_contents.end(), std::mt19937(std::random_device()()));
        for (int i = 0; i < n_puzzles; i++){
            const std::string file_content = file_contents[i];
            auto time = solve_for(file_content);
            times[i] += time;
        }
    }

    std::cout << "Benchmark results (average time for " << n_repeats << " repeats):" << std::endl;
    for (int i = 0; i < n_puzzles; i++)
    {
        std::cout << "Puzzle " << i+1 << " took " << 
        std::chrono::duration_cast<std::chrono::microseconds>(times[i]).count() / n_repeats << " [µs]" << std::endl;
    }
}
