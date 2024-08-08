
CXX := g++
STD_FLAGS := -std=c++17 -Wall -Wextra -pedantic -O2 -DSTRICT
LIB_DIR := bin/lib
BIN_DIR := bin

all: test target

_dst:
	mkdir -p $(BIN_DIR) && mkdir -p $(LIB_DIR)

obj: _dst
	$(CXX) $(STD_FLAGS) -o $(LIB_DIR)/util.o -c src/util.cpp
	$(CXX) $(STD_FLAGS) -o $(LIB_DIR)/board.o -c src/board.cpp
	$(CXX) $(STD_FLAGS) -o $(LIB_DIR)/cell.o -c src/cell.cpp
	$(CXX) $(STD_FLAGS) -o $(LIB_DIR)/solver.o -c src/solver.cpp
	$(CXX) $(STD_FLAGS) -o $(LIB_DIR)/solver_v1.o -c src/solver_v1.cpp

test: obj
	$(CXX) $(STD_FLAGS) -o $(BIN_DIR)/util_test \
		$(LIB_DIR)/util.o \
		src/util_test.cpp
	$(CXX) $(STD_FLAGS) -o $(BIN_DIR)/board_test \
		$(LIB_DIR)/board.o $(LIB_DIR)/util.o \
		src/board_test.cpp
	$(CXX) $(STD_FLAGS) -o $(BIN_DIR)/cell_test \
		$(LIB_DIR)/board.o $(LIB_DIR)/util.o $(LIB_DIR)/cell.o \
		src/cell_test.cpp

target: obj
	$(CXX) $(STD_FLAGS) -o $(BIN_DIR)/sudoku \
		$(LIB_DIR)/board.o $(LIB_DIR)/util.o $(LIB_DIR)/cell.o $(LIB_DIR)/solver.o $(LIB_DIR)/solver_v1.o \
		src/main.cpp