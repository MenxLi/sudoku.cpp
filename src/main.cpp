#include "solver_v1.h"
#include <chrono>

bool solve_for(std::string input_file, std::string output_file)
{
    Board board;
    board.load_from_file(input_file);

    SolverV1 solver(board);
    bool solved = false;

    try{
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        solved = solver.solve();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Time elapsed: " 
            << std::chrono::duration_cast<std::chrono::microseconds>( end - begin).count()
            << " [µs] ";
        std::cout << "Puzzle: " << input_file << " ";
    } catch (std::exception& e){
        std::cerr << "Error while solving " << input_file << ", " << e.what() << std::endl;
        board.save_to_file(output_file);
        return solved;
    }

    if (solved)
    {
        std::cout << "Solved! ";
    }
    else
    {
        std::cout << "Not solved. ";
    }

    board.save_to_file(output_file);

    std::cout << "Output saved to: " << output_file << std::endl;
    return solved;
}

int main(int argc, char* argv[]){
    // parse arguments to get input and output file names
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " -i <input_file> -o <output_file>" << std::endl;
        exit(1);
    }

    // parse arguments
    std::string input_file;
    std::string output_file;

    for (int i = 1; i < argc; i++)
    {
        if (std::string(argv[i]) == "-i")
        {
            if (i + 1 < argc)
            {
                input_file = argv[i + 1];
            }
            else
            {
                std::cerr << "-i requires an argument" << std::endl;
                exit(1);
            }
        }
        else if (std::string(argv[i]) == "-o")
        {
            if (i + 1 < argc)
            {
                output_file = argv[i + 1];
            }
            else
            {
                std::cerr << "-o requires an argument" << std::endl;
                exit(1);
            }
        }
    }

    if (input_file.empty() || output_file.empty())
    {
        std::cerr << "Usage: " << argv[0] << " -i <input_file> -o <output_file>" << std::endl;
        exit(1);
    }

    return solve_for(input_file, output_file) ? 0 : 1;
}
