#include <pybind11/pybind11.h>
#include <pybind11/stl.h>       // for automatic conversion of std::vector
#include <vector>

#include "config.h"
#include "solver_v1.h"
#include "board.h"

namespace py = pybind11;

py::dict solve(
    std::vector<std::vector<val_t>> input
){
    Board b;
    b.load_data(input);
    SolverV1 solver(b);
    bool solved = solver.solve();

    std::vector<std::vector<val_t>> data;
    val_t* raw_data = b.data();
    for (int i=0; i<BOARD_SIZE; i++){
        std::vector<val_t> row;
        for (int j=0; j<BOARD_SIZE; j++){
            row.push_back( raw_data[i*BOARD_SIZE + j]);
        }
        data.push_back(row);
    }

    py::dict result;
    result["solved"] = solved;
    result["iterations"] = solver.iteration_counter().current;
    result["iteration_limit"] = solver.iteration_counter().limit;
    result["data"] = data;
    return result;
}

PYBIND11_MODULE(sudoku, m) {
    m.doc() = "Sudoku solver"; // optional module docstring
    m.def("solve", &solve, "Solve a sudoku puzzle");
}