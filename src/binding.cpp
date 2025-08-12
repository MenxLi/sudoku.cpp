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
        std::string sp= " ",           // seperator for values
        std::string nl = "\n"           // newline character
    ) {

        auto board = std::make_unique<Board>();

        // the default case, no need to split
        if (sp == " " && nl == "\n") {  
            board->load_data(str_data);
            return SudokuBoard(std::move(board));
        }

        if (sp != "" && nl =="")
            throw std::runtime_error("Invalid separator, nl cannot be empty if sp is not empty"); 
        if (BOARD_SIZE > 16 && sp == "") 
            throw std::runtime_error("Invalid separator (empty) for large boards"); 

        // maybe replace nl with sp (move to mark str_data not use anymore)
        std::string sdata;
        nl != sp ? 
            sdata = util::replace_string(std::move(str_data), nl, sp): 
            sdata = std::move(str_data);
        
        // handle the case where sp is empty
        if (sp == "") {
            std::vector<val_t> board_data(CELL_COUNT);
            if (sdata.size() != CELL_COUNT) {
                throw std::runtime_error("Invalid data size, expected " + std::to_string(CELL_COUNT) + " values, got " + std::to_string(str_data.size()));
            }
            for (unsigned int i = 0; i < sdata.size(); i++) {
                char c = sdata[i];
                if (c == '.') { board_data[i] = 0; } 
                else if (c == ' ') { board_data[i] = 0; } 
                else if (c >= '0' && c <= '9') { board_data[i] = static_cast<val_t>(c - '0'); } 
                else if (c >= 'a' && c <= 'f') { board_data[i] = static_cast<val_t>(c - 'a' + 10); } 
                else if (c >= 'A' && c <= 'F') { board_data[i] = static_cast<val_t>(c - 'A' + 10); } 
                else {
                    throw std::runtime_error("Invalid character in input: " + std::string(1, c));
                }
            }
            return from_list1d(std::move(board_data));
        }

        // sp is not empty, split the string
        std::vector<std::string> vals = util::split_string(sdata, sp);
        if (vals.size() != CELL_COUNT) {
            throw std::runtime_error("Invalid data size, expected " + std::to_string(CELL_COUNT) + " values, got " + std::to_string(vals.size()));
        }

        std::vector<val_t> board_data(CELL_COUNT);
        for (unsigned int i = 0; i < CELL_COUNT; i++) {
            std::string c = vals[i];
            c == "."?
                board_data[i] = 0 : 
                board_data[i] = static_cast<val_t>(std::stoi(c));
        }
        return from_list1d(board_data);
    }

    std::vector<val_t> to_list1d() const {
        std::vector<val_t> data(CELL_COUNT, 0);
        for (unsigned int i = 0; i < CELL_COUNT; i++) {
            data[i] = m_board->get(i).retrive();
        }
        return data;
    }
    std::vector<std::vector<val_t>> to_list2d() const {
        std::vector<std::vector<val_t>> data(BOARD_SIZE, std::vector<val_t>(BOARD_SIZE, 0));
        for (unsigned int i = 0; i < BOARD_SIZE; i++) {
            for (unsigned int j = 0; j < BOARD_SIZE; j++) {
                data[i][j] = m_board->get(i, j).retrive();
            }
        }
        return data;
    }

    val_t get(int row, int col) const {
        return m_board->get(row, col).retrive();
    }

    val_t set(int row, int col, val_t value) {
        m_board->set(row, col, value);
        return value;
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
    result["solved"] = solved;
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

    if (!generated){
        throw std::runtime_error("Failed to generate a board with " + std::to_string(n_clues_remain) + " clues remaining");
    }

    py::dict result;
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
                    py::arg("nl") = "\n"
                )
        .def_static("from_list1d", &SudokuBoard::from_list1d)
        .def_static("from_list2d", &SudokuBoard::from_list2d)
        .def("to_list1d", &SudokuBoard::to_list1d)
        .def("to_list2d", &SudokuBoard::to_list2d)
        .def("get", &SudokuBoard::get, py::arg("row"), py::arg("col"))
        .def("set", &SudokuBoard::set, py::arg("row"), py::arg("col"), py::arg("value"));

    m.doc() = "Sudoku solver and generator using C++ backend";
    m.def("solve", &solve, "Solve a sudoku puzzle");
    m.def("generate", &generate, "Generate a sudoku puzzle", 
          py::arg("n_clues_remain"), 
          py::arg("max_retries") = 1024, 
          py::arg("verbose") = false
        );
    m.def("build_config", &build_config, "Build config");
}