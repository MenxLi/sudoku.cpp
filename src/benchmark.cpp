#include "config.h"
#include "solver.h"
#include "parser.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <ostream>
#include <string>
#include <array>


/*
most of the dataset are stored compactly in a single line, 
with each character representing a cell in the sudoku board.
this function will do:
- read the first CELL_COUNT characters
- replace a-z with 10-35 (for 16x16 and 25x25 sudoku)
- replace A-Z with 10-35 (case insensitive)
- replace . with 0
- format the line to a CELL_COUNT array
*/
std::array<val_t, CELL_COUNT> data_from_compact_line(std::string line){
    if (line.size() < CELL_COUNT){
        throw std::runtime_error("Invalid line size");
    }

    std::string valid_str = line.substr(0, CELL_COUNT);
    std::array<val_t, CELL_COUNT> board_data;

    for (unsigned int i=0; i<CELL_COUNT; i++){
        char& c = valid_str[i];
        if (c >= 'a' && c <= 'z'){
            board_data[i] = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'Z'){
            board_data[i] = c - 'A' + 10;
        } else if (c == '.'){
            board_data[i] = 0;
        } else {
            board_data[i] = std::stoi(std::string(1, c));
        }
    }

    return board_data;
}

struct CaseResult
{
    std::chrono::duration<double> time;
    bool solved;
    unsigned int n_guesses;
};

template <typename T>
CaseResult solve_for(T content)
{
    Board board;
    board.load_data(content);

    auto start = std::chrono::high_resolution_clock::now();
    Solver solver(board);
    bool ret = solver.solve();
    auto end = std::chrono::high_resolution_clock::now();

    CaseResult res;
    res.time = end - start;
    res.solved = ret;
    res.n_guesses = solver.iteration_counter().n_guesses;

    return res;
}

int run_default_test(){
    const unsigned int n_repeats = 100;
    const unsigned int n_puzzles = 9;
    std::cout << "Benchmarking " << n_puzzles << " puzzles with " << n_repeats << " repeats..." << std::endl;
    for (unsigned int i = 0; i < n_puzzles; i++)
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
        for (unsigned int j = 0; j < n_repeats; j++)
        {
            total_time += (solve_for(file_content)).time;
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
    return 0;
}

int run_test_on_file(
    const std::string& filename, 
    unsigned int offset = 0,
    unsigned int max_cases = 100000
){
    std::ifstream file(filename);
    if (!file.is_open()){
        return 1;
    }

    std::string line_content;
    std::vector<CaseResult> results;
    size_t line_counter = 0;
    while (std::getline(file, line_content)){
        if (line_counter < offset) continue; 
        if (line_counter - offset > max_cases) {
            std::cout << "[Warn] Reached max cases limit: " << max_cases << ", skip rest." << std::endl;
            break;
        }
        line_counter++;
        if (line_content.size() < CELL_COUNT) continue;
        if (line_content[0] == '#') continue; // skip comment lines
        auto data = data_from_compact_line(line_content);
        auto data_vector = std::vector<val_t>(data.begin(), data.end());
        auto res = solve_for(data_vector);
        results.push_back(res);
    }

    std::qsort(results.data(), results.size(), sizeof(CaseResult), [](const void* a, const void* b){
        return ((CaseResult*)a)->time.count() < ((CaseResult*)b)->time.count()? -1 : 1;
    });
    unsigned int n = results.size();
    unsigned int total_time_us = ([&results](){
        std::chrono::duration<double> total = std::chrono::duration<double>::zero();
        for (auto res : results){
            total += res.time;
        }
        return std::chrono::duration_cast<std::chrono::microseconds>(total).count();
    })();
    unsigned int max_time_us = std::chrono::duration_cast<std::chrono::microseconds>(results[n-1].time).count();
    unsigned int median_time_us = std::chrono::duration_cast<std::chrono::microseconds>(results[n/2].time).count();
    unsigned int one_quartile_time_us = std::chrono::duration_cast<std::chrono::microseconds>(results[n/4].time).count();
    unsigned int three_quartile_time_us = std::chrono::duration_cast<std::chrono::microseconds>(results[3*n/4].time).count();

    auto [success_rate, average_guesses] = ([&results, &n](){
        unsigned int n_solved = 0;
        unsigned int total_guesses = 0;
        for (auto res : results){
            if (res.solved){
                n_solved++;
            }
            total_guesses += res.n_guesses;
        }
        return std::make_pair((float)n_solved / n, (float)total_guesses / n);
    })();

    unsigned int median_guesses = results[n/2].n_guesses;

    unsigned int max_guesses = ([&results](){
        unsigned int max = 0;
        for (auto res : results){
            if (res.n_guesses > max){
                max = res.n_guesses;
            }
        }
        return max;
    })();

    std::cout << "Finished on " << n << " cases" << std::endl;
    std::cout << "Success rate: " << success_rate * 100 << "%" << std::endl;
    std::cout << "Mean time: " << total_time_us / n << " [us]" << std::endl;
    std::cout << "Median time: " << median_time_us << " [us]" << std::endl;
    std::cout << "Max time: " << max_time_us << " [us]" << std::endl;
    std::cout << "1st quartile time: " << one_quartile_time_us << " [us]" << std::endl;
    std::cout << "3rd quartile time: " << three_quartile_time_us << " [us]" << std::endl;
    std::cout << "Average guesses: " << average_guesses << std::endl;
    std::cout << "Median guesses: " << median_guesses << std::endl;
    std::cout << "Max guesses: " << max_guesses << std::endl;

    return 0;
};

int main(int argc, char* argv[])
{
    auto parser = parser::CommandlineParser(argc, argv);
    parser.set_help_message(
        "Usage: " + std::string(argv[0]) + "[filename] [options]\n"
        "Options:\n"
        "  -h, --help          Show this help message and exit\n"
        "  --offset <n>        Offset for the file (default: 0)\n"
        "  --limit <n>         Maximum number of cases to run (default: 100000)\n"
    );

    parser.check_help_exit();

    if (argc == 1){
        exit(run_default_test());
    }

    unsigned int offset = parser.parse_arg<unsigned int>("--offset", 0);
    unsigned int limit = parser.parse_arg<unsigned int>("--limit", 100000);

    std::string filename = argv[1];
    if (filename[0] == '-'){
        std::cerr << "Error: filename cannot start with '-'" << std::endl;
        exit(1);
    }

    exit(run_test_on_file(filename, offset, limit));
    std::cout << "Usage: " << argv[0] << " [filename]" << std::endl;

}
