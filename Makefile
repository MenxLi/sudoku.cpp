
CXX := g++
STD_FLAGS := -std=c++17 -Wall -Wextra -Werror -pedantic
LIB_DIR := bin/lib
BIN_DIR := bin

_dst:
	mkdir -p $(BIN_DIR) && mkdir -p $(LIB_DIR) && mkdir -p output

obj: _dst
	$(CXX) $(STD_FLAGS) -o $(LIB_DIR)//util.o -c src/util.cpp
	$(CXX) $(STD_FLAGS) -o $(LIB_DIR)/board.o -c src/board.cpp

test: obj
	$(CXX) $(STD_FLAGS) -o $(BIN_DIR)//util_test \
		$(LIB_DIR)/util.o \
		src/util_test.cpp
	$(CXX) $(STD_FLAGS) -o $(BIN_DIR)/board_test \
		$(LIB_DIR)/board.o $(LIB_DIR)/util.o \
		src/board_test.cpp

all: test