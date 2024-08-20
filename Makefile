
SIZE?=9
DEBUG?=0

CXX := g++
STD_FLAGS := -std=c++17 -Wall -Wextra -Werror -pedantic

ifeq ($(DEBUG),0)
OPTIMIZATION_FLAGS := -O3 -funroll-loops -finline-functions
else
OPTIMIZATION_FLAGS := -O0 -g
endif

CONFIG_FLAGS := -DSIZE=$(SIZE) -DSTRICT
COMMON_FLAGS := $(STD_FLAGS) $(OPTIMIZATION_FLAGS) $(CONFIG_FLAGS)
LIB_DIR := bin/lib
BIN_DIR := bin

objs := $(LIB_DIR)/util.o $(LIB_DIR)/board.o $(LIB_DIR)/solver.o $(LIB_DIR)/solver_v1.o $(LIB_DIR)/solver_v2.o $(LIB_DIR)/generate.o

ifeq ($(OS),Windows_NT)
	UNAME_S := Windows
else
	UNAME_S := $(shell uname -s)
endif

ifeq ($(UNAME_S),Linux)
	COMMON_FLAGS += -pthread
endif


all: test target

_dst:
	mkdir -p $(BIN_DIR) && mkdir -p $(LIB_DIR)

util.o: _dst
	$(CXX) $(COMMON_FLAGS) -o $(LIB_DIR)/util.o -c src/util.cpp
board.o: _dst
	$(CXX) $(COMMON_FLAGS) -o $(LIB_DIR)/board.o -c src/board.cpp
solver.o: _dst
	$(CXX) $(COMMON_FLAGS) -o $(LIB_DIR)/solver.o -c src/solver.cpp
solver_v1.o: _dst
	$(CXX) $(COMMON_FLAGS) -o $(LIB_DIR)/solver_v1.o -c src/solver_v1.cpp
solver_v2.o: _dst
	$(CXX) $(COMMON_FLAGS) -o $(LIB_DIR)/solver_v2.o -c src/solver_v2.cpp
generate.o: _dst
	$(CXX) $(COMMON_FLAGS) -o $(LIB_DIR)/generate.o -c src/generate.cpp

obj: util.o board.o solver.o solver_v1.o solver_v2.o generate.o

test: obj
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/util_test \
		$(LIB_DIR)/util.o \
		src/util_test.cpp
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/board_test \
		$(LIB_DIR)/board.o $(LIB_DIR)/util.o \
		src/board_test.cpp
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/solver_test \
		$(objs) src/solver_test.cpp
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/generate_test \
		$(objs) src/generate_test.cpp

target: obj
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/sudoku \
		$(objs) src/main.cpp
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/benchmark \
		$(objs) src/benchmark.cpp
