#include "board.h"
#include "config.h"
#include "parser.hpp"
#include "solver_v2.h"
#include "generate.h"
#include <chrono>

bool solve_for(Board board, std::string output_file, bool verbose)
{
    SolverV2 solver(board);
    bool solved = false;

    try{
        auto start = std::chrono::high_resolution_clock::now();
        solved = solver.solve();
        auto end = std::chrono::high_resolution_clock::now();
        if (verbose) std::cout << "Time elapsed: " 
            << std::chrono::duration_cast<std::chrono::microseconds>( end - start).count()
            << " [µs] ";
    } catch (std::exception& e){
        std::cerr << "Error: " << e.what() << std::endl;
        solved = false;
    }

    if (verbose){
        if (solved) { std::cout << "Solved! "; }
        else { std::cout << "Not solved. "; }
    }

    if (!output_file.empty()){
        solver.board().save_to_file(output_file);
        if (verbose) std::cout << "Output saved to: " << output_file << std::endl;
    }
    else{
        if (verbose) std::cout << "Output: " << std::endl;
        std::cout << solver.board() << std::endl;
    }
    return solved;
}

bool generate_for(unsigned int clue_count, std::string output_file, bool verbose){
    auto [success, board] = gen::generate_board(clue_count, 1e5, true, verbose);
    if (!success){
        std::cerr << "Failed to generate a board with " << clue_count << " clues" << std::endl;
        return false;
    }

    if (!output_file.empty()){
        board.save_to_file(output_file);
        if (verbose) std::cout << "Output saved to: " << output_file << std::endl;
    }
    else{
        if (verbose) std::cout << "Output: " << std::endl;
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
        "  -v, --verbose         Show verbose output\n"\
        "solve:\n"\
        "  [-i <input_file>]     Input file, will read from stdin if not provided\n"\
        "  [-o <output_file>]    Output file\n"\
        "generate:\n"\
        "  [-c <clue_count>]     Number of clues, will output full board if not provided\n"\
        "  [-o <output_file>]    Output file\n"\
        );
    parser.check_help_exit();

    std::string input_file = parser.parse_arg<std::string>("-i", "");
    std::string output_file = parser.parse_arg<std::string>("-o", "");
    int clue_count = parser.parse_arg<int>("-c", CELL_COUNT);
    bool verbose = parser.parse_flag("-v") || parser.parse_flag("--verbose");

    if (parser.has_subparser("solve")) {
        Board board;
        if (input_file.empty())
        {
            std::string input_str;
            std::string line;
            while (std::getline(std::cin, line))
            {
                input_str += line + "\n";
            }
            board.load_data(input_str);
        }
        else{
            board.load_from_file(input_file);
        }
        return solve_for(board, output_file, verbose) ? 0 : 1;
    } else if (parser.has_subparser("generate")) {
        return generate_for(clue_count, output_file, verbose) ? 0 : 1;
    } else {
        std::cout << "Invalid subparser, please use -h to check usage" << std::endl;
        exit(1);
    }
}
