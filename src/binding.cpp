#include <pybind11/pybind11.h>
#include <pybind11/stl.h>       // for automatic conversion of std::vector
#include <memory>
#include <string>
#include <vector>
#include <chrono>

#include "config.h"
#include "pybind11/gil.h"
#include "solver.h"
#include "board.h"
#include "generate.h"

namespace py = pybind11;

// a simple wrapper class to manage the Board object in C++
// and use it in Python code
class SudokuBoard {
    std::unique_ptr<Board> m_board;
public:

    explicit SudokuBoard() : m_board(std::make_unique<Board>()) {}
    explicit SudokuBoard(std::unique_ptr<Board> board) : m_board(std::move(board)) {}
    explicit SudokuBoard(const Board& board) : m_board(std::make_unique<Board>(board)) {}
    explicit SudokuBoard(Board&& board) : m_board(std::make_unique<Board>(std::move(board))) {}
    Board& board() { return *m_board; }

    static SudokuBoard from_list1d(std::vector<val_t> data) {
        auto board = std::make_unique<Board>();
        board->load_data(std::move(data));
        return SudokuBoard(std::move(board));
    }
    static SudokuBoard from_list2d(std::vector<std::vector<val_t>> data) {
        auto board = std::make_unique<Board>();
        board->load_data(std::move(data));
        return SudokuBoard(std::move(board));
    }

    static SudokuBoard from_str(
        std::string str_data, 
        std::string sp= " ",                // seperator for values
        std::string nl = "\n",              // newline character
        std::string empty = "0"             // empty value representation
    ) {
        Board board = Board::from_string(
            str_data, 
            std::move(sp), 
            std::move(nl), 
            std::move(empty)
        );
        return SudokuBoard(std::move(board));
    }

    std::vector<val_t> to_list1d() const {
        std::vector<val_t> data(CELL_COUNT);
        for (unsigned int i = 0; i < CELL_COUNT; i++) {
            data[i] = m_board->get(i).retrive();
        }
        return data;
    }
    std::vector<std::vector<val_t>> to_list2d() const {
        std::vector<std::vector<val_t>> data(BOARD_SIZE, std::vector<val_t>(BOARD_SIZE));
        for (unsigned int i = 0; i < BOARD_SIZE; i++) {
            for (unsigned int j = 0; j < BOARD_SIZE; j++) {
                data[i][j] = m_board->get(i, j).retrive();
            }
        }
        return data;
    }

    std::string to_str(
        std::string sp = " ", 
        std::string nl = "\n", 
        std::string empty = "0"  // default empty value representation
    ) const {
        return m_board->to_string(
            std::move(sp), 
            std::move(nl), 
            std::move(empty)
        );
    }

    val_t get(int row, int col) const {
        return m_board->get(row, col).retrive();
    }

    val_t set(int row, int col, val_t value) {
        m_board->set(row, col, value);
        return value;
    }

    bool equals(const SudokuBoard& other) const {
        return *m_board == *other.m_board;
    }
};

py::dict solve(SudokuBoard& sudoku_board) {
    Board b(sudoku_board.board());

    auto start_time = std::chrono::high_resolution_clock::now();
    Solver solver(b);
    bool solved = solver.solve();
    auto end_time = std::chrono::high_resolution_clock::now();

    py::dict result;
    result["board"] = SudokuBoard(std::make_unique<Board>(solver.board()));
    result["success"] = solved;
    result["iterations"] = solver.iteration_counter().current;
    result["iteration_limit"] = solver.iteration_counter().limit;
    result["n_guesses"] = solver.iteration_counter().n_guesses;
    result["time_us"] = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    return result;
}

py::dict generate(
    unsigned int n_clues_remain, 
    unsigned int max_retries = 1024,
    bool verbose = false
){
    Board b;
    auto start_time = std::chrono::high_resolution_clock::now();

    // here only one thread is used, 
    // because python's GIL handling is too complex to properly checking for keyboard interrupts...
    auto [generated, board] = gen::generate_board(n_clues_remain, max_retries, 0, verbose);
    auto end_time = std::chrono::high_resolution_clock::now();

    py::dict result;
    result["success"] = generated;
    result["board"] = SudokuBoard(board);
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
    py::class_<SudokuBoard>(m, "Board")
        .def(py::init<std::unique_ptr<Board>>())
        .def_static("from_str", &SudokuBoard::from_str, 
                    py::arg("str_data"), 
                    py::arg("sp") = " ", 
                    py::arg("nl") = "\n", 
                    py::arg("empty") = "0"
                )
        .def_static("from_list1d", &SudokuBoard::from_list1d)
        .def_static("from_list2d", &SudokuBoard::from_list2d)
        .def("to_list1d", &SudokuBoard::to_list1d)
        .def("to_list2d", &SudokuBoard::to_list2d)
        .def("to_str", &SudokuBoard::to_str, 
                py::arg("sp") = " ", 
                py::arg("nl") = "\n", 
                py::arg("empty") = "0"
            )
        .def("get", &SudokuBoard::get, py::arg("row"), py::arg("col"))
        .def("set", &SudokuBoard::set, py::arg("row"), py::arg("col"), py::arg("value"))
        .def("__eq__", &SudokuBoard::equals, py::arg("other"));

    m.doc() = "Sudoku solver and generator using C++ backend";
    m.def("solve", &solve, "Solve a sudoku puzzle");
    m.def("generate", &generate, "Generate a sudoku puzzle", 
          py::arg("n_clues_remain"), 
          py::arg("max_retries") = 1024, 
          py::arg("verbose") = false
        );
    m.def("build_config", &build_config, "Build config");
}