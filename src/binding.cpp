#include <pybind11/pybind11.h>
#include <pybind11/stl.h>       // for automatic conversion of std::vector
#include <vector>
#include <chrono>

#include "config.h"
#include "solver_v2.h"
#include "board.h"
#include "generate.h"

namespace py = pybind11;

std::vector<std::vector<val_t>> board_to_vector(Board& b){
    std::vector<std::vector<val_t>> data;
    val_t* raw_data = b.data();
    for (unsigned int i=0; i<BOARD_SIZE; i++){
        std::vector<val_t> row;
        for (unsigned int j=0; j<BOARD_SIZE; j++){
            row.push_back( raw_data[i*BOARD_SIZE + j]);
        }
        data.push_back(row);
    }
    return data;
}

py::dict solve(
    std::vector<std::vector<val_t>> input
){
    Board b;
    b.load_data(input);

    auto start_time = std::chrono::high_resolution_clock::now();
    SolverV2 solver(b);
    bool solved = solver.solve();
    auto end_time = std::chrono::high_resolution_clock::now();

    auto data = board_to_vector(solver.board());

    py::dict result;
    result["solved"] = solved;
    result["iterations"] = solver.iteration_counter().current;
    result["iteration_limit"] = solver.iteration_counter().limit;
    result["n_guesses"] = solver.iteration_counter().n_guesses;
    result["data"] = data;
    result["time_us"] = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    return result;
}

py::dict generate(
    unsigned int n_clues_remain, 
    unsigned int max_retries, 
    bool parallel_exec, 
    bool verbose
){
    Board b;
    auto start_time = std::chrono::high_resolution_clock::now();
    auto [generated, board] = gen::generate_board(n_clues_remain, max_retries, parallel_exec, verbose);
    auto end_time = std::chrono::high_resolution_clock::now();

    if (!generated){
        throw std::runtime_error("Failed to generate a board with " + std::to_string(n_clues_remain) + " clues remaining");
    }

    std::vector<std::vector<val_t>> data = board_to_vector(board);

    py::dict result;
    result["data"] = data;
    result["time_us"] = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    return result;
}

py::dict build_config(){
    py::dict config;
    config["BOARD_SIZE"] = BOARD_SIZE;
    config["GRID_SIZE"] = GRID_SIZE;
    config["MAX_ITER"] = MAX_ITER;
    return config;
}

PYBIND11_MODULE(sudoku, m) {
    m.doc() = "Sudoku solver"; // optional module docstring
    m.def("solve", &solve, "Solve a sudoku puzzle");
    m.def("generate", &generate, "Generate a sudoku puzzle");
    m.def("build_config", &build_config, "Build config");
}