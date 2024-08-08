
CXX := g++
STD_FLAGS := -std=c++17
OPTIMIZATION_FLAGS := -O3 -funroll-loops
CONFIG_FLAGS := -DGRID_SIZE=3 -DSTRICT
COMMON_FLAGS := $(STD_FLAGS) $(OPTIMIZATION_FLAGS) $(CONFIG_FLAGS)
LIB_DIR := bin/lib
BIN_DIR := bin

ifeq ($(OS),Windows_NT)
	UNAME_S := Windows
else
	UNAME_S := $(shell uname -s)
endif

all: test target python

_dst:
	mkdir -p $(BIN_DIR) && mkdir -p $(LIB_DIR)

obj: _dst
	$(CXX) $(COMMON_FLAGS) -o $(LIB_DIR)/util.o -c src/util.cpp
	$(CXX) $(COMMON_FLAGS) -o $(LIB_DIR)/board.o -c src/board.cpp
	$(CXX) $(COMMON_FLAGS) -o $(LIB_DIR)/cell.o -c src/cell.cpp
	$(CXX) $(COMMON_FLAGS) -o $(LIB_DIR)/solver.o -c src/solver.cpp
	$(CXX) $(COMMON_FLAGS) -o $(LIB_DIR)/solver_v1.o -c src/solver_v1.cpp

test: obj
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/util_test \
		$(LIB_DIR)/util.o \
		src/util_test.cpp
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/board_test \
		$(LIB_DIR)/board.o $(LIB_DIR)/util.o \
		src/board_test.cpp
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/cell_test \
		$(LIB_DIR)/board.o $(LIB_DIR)/util.o $(LIB_DIR)/cell.o \
		src/cell_test.cpp

target: obj
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/sudoku \
		$(LIB_DIR)/board.o $(LIB_DIR)/util.o $(LIB_DIR)/cell.o $(LIB_DIR)/solver.o $(LIB_DIR)/solver_v1.o \
		src/main.cpp
	$(CXX) $(STD_FLAGS) $(CONFIG_FLAGS) -Og -o $(BIN_DIR)/benchmark \
		$(LIB_DIR)/board.o $(LIB_DIR)/util.o $(LIB_DIR)/cell.o $(LIB_DIR)/solver.o $(LIB_DIR)/solver_v1.o \
		src/benchmark.cpp

python: obj
ifeq ($(UNAME_S),Darwin)
	$(CXX) $(COMMON_FLAGS) -undefined dynamic_lookup \
		-shared -fPIC $(shell python3 -m pybind11 --includes) -o $(LIB_DIR)/sudoku$(shell python3-config --extension-suffix) \
		$(LIB_DIR)/board.o $(LIB_DIR)/util.o $(LIB_DIR)/cell.o $(LIB_DIR)/solver.o $(LIB_DIR)/solver_v1.o \
		src/binding.cpp
else
	$(CXX) $(COMMON_FLAGS) \
		-shared -fPIC $(shell python3 -m pybind11 --includes) -o $(LIB_DIR)/sudoku$(shell python3-config --extension-suffix) \
		$(LIB_DIR)/board.o $(LIB_DIR)/util.o $(LIB_DIR)/cell.o $(LIB_DIR)/solver.o $(LIB_DIR)/solver_v1.o \
		src/binding.cpp
endif
