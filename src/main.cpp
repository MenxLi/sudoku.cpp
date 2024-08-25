#include "parser.hpp"
#include "solver_v2.h"
#include "generate.h"
#include <chrono>

bool solve_for(std::string input_file, std::string output_file)
{
    Board board;
    board.load_from_file(input_file);

    SolverV2 solver(board);
    bool solved = false;

    try{
        auto start = std::chrono::high_resolution_clock::now();
        solved = solver.solve();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time elapsed: " 
            << std::chrono::duration_cast<std::chrono::microseconds>( end - start).count()
            << " [µs] ";
        std::cout << "Puzzle: " << input_file << " ";
    } catch (std::exception& e){
        std::cerr << "Error while solving " << input_file << ", " << e.what() << std::endl;
        solved = false;
    }

    if (solved)
    {
        std::cout << "Solved! ";
    }
    else
    {
        std::cout << "Not solved. ";
    }

    if (!output_file.empty()){
        solver.board().save_to_file(output_file);
        std::cout << "Output saved to: " << output_file << std::endl;
    }
    else{
        std::cout << "Output: " << std::endl;
        std::cout << solver.board() << std::endl;
    }
    return solved;
}

bool generate_for(unsigned int clue_count, std::string output_file){
    auto [success, board] = gen::generate_board(clue_count, 2048, true);
    if (!success){
        std::cerr << "Failed to generate a board with " << clue_count << " clues" << std::endl;
        return false;
    }

    if (!output_file.empty()){
        board.save_to_file(output_file);
        std::cout << "Output saved to: " << output_file << std::endl;
    }
    else{
        std::cout << "Output: " << std::endl;
        std::cout << board << std::endl;
    }
    return true;
}

int main(int argc, char* argv[]){
    // parse arguments to get input and output file names
    if (argc < 2)
    {
        std::cerr << "Usage-1: " << argv[0] << " solve -i <input_file> [-o <output_file>]" << std::endl;
        std::cerr << "Usage-2: " << argv[0] << " generate -c <clue_count> [-o <output_file>]" << std::endl;
        exit(1);
    }

    // parse arguments
    std::string mode = argv[1];
    auto parser = parser::CommandlineParser(argc, argv);

    std::string input_file = parser.get_arg("-i");
    std::string output_file = parser.get_arg("-o");
    std::string clue_count_str = parser.get_arg("-c");

    if (mode == "solve") {
        if (input_file.empty())
        {
            std::cerr << "Usage: " << argv[0] << "solve -i <input_file> [-o <output_file>]" << std::endl;
            exit(1);
        }
        return solve_for(input_file, output_file) ? 0 : 1;
    } else if (mode == "generate") {
        if (clue_count_str.empty())
        {
            std::cerr << "Usage: " << argv[0] << "generate -c <clue_count> [-o <output_file>]" << std::endl;
            exit(1);
        }
        return generate_for(std::stoi(clue_count_str), output_file) ? 0 : 1;
    } else {
        std::cerr << "Unknown mode: " << mode << std::endl;
        exit(1);
    }
}
