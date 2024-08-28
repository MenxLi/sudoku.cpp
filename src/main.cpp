#include "config.h"
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
    auto parser = parser::CommandlineParser(argc, argv);

    parser.set_help_message(
        "Usage: " + parser.prog_name() + " solve|generate \n"
        "Options:\n"
        "  -h, --help            Show this help message and exit\n"\
        "solve:\n"\
        "  -i <input_file>       Input file\n"\
        "  [-o <output_file>]    Output file\n"\
        "generate:\n"\
        "  [-c <clue_count>]     Number of clues\n"\
        "  [-o <output_file>]    Output file\n"\
        );
    parser.check_help_exit();

    std::string input_file = parser.parse_arg<std::string>("-i", "");
    std::string output_file = parser.parse_arg<std::string>("-o", "");
    int clue_count = parser.parse_arg<int>("-c", CELL_COUNT);

    if (parser.has_subparser("solve")) {
        if (input_file.empty())
        {
            std::cerr << "Please provide an input file" << std::endl;
            exit(1);
        }
        return solve_for(input_file, output_file) ? 0 : 1;
    } else if (parser.has_subparser("generate")) {
        return generate_for(clue_count, output_file) ? 0 : 1;
    } else {
        std::cout << "Invalid subparser, please use -h to check usage" << std::endl;
        exit(1);
    }
}
