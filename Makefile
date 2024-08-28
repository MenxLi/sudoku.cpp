
SIZE?=9
DEBUG?=0

CXX := g++
STD_FLAGS := -std=c++17 -Wall -Wextra

ifeq ($(DEBUG),0)
OPTIMIZATION_FLAGS := -O3 -funroll-loops -finline-functions
else
OPTIMIZATION_FLAGS := -O0 -g
endif

CONFIG_FLAGS := -DSIZE=$(SIZE) -DSTRICT
COMMON_FLAGS := $(STD_FLAGS) $(OPTIMIZATION_FLAGS) $(CONFIG_FLAGS)
LIB_DIR := bin/lib-$(SIZE)
BIN_DIR := bin

OBJS := $(LIB_DIR)/indexer_impl_$(SIZE).o $(LIB_DIR)/util.o $(LIB_DIR)/board.o \
	$(LIB_DIR)/solver_base.o $(LIB_DIR)/solver_v2.o $(LIB_DIR)/generate.o

TEST_TARGETS := $(patsubst src/%_test.cpp, $(BIN_DIR)/test_%, $(wildcard src/*_test.cpp))

ifeq ($(OS),Windows_NT)
	UNAME_S := Windows
else
	UNAME_S := $(shell uname -s)
endif

ifeq ($(UNAME_S),Linux)
	COMMON_FLAGS += -pthread
endif

.PHONY: target test clean dst

target: dst $(OBJS)
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/sudoku $(OBJS) src/main.cpp
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/benchmark $(OBJS) src/benchmark.cpp

test: dst $(TEST_TARGETS)

src/indexer_impl_$(SIZE).cpp: src/indexer_gen.py
	@python src/indexer_gen.py $(SIZE)

$(LIB_DIR)/%.o: src/%.cpp
	$(CXX) $(COMMON_FLAGS) -o $@ -c $<

$(BIN_DIR)/test_%: $(OBJS) src/%.cpp
	$(CXX) $(COMMON_FLAGS) -o $@ $(OBJS) src/$*_test.cpp

dst:
	@mkdir -p $(BIN_DIR) && mkdir -p $(LIB_DIR)

clean:
	-rm -r $(BIN_DIR)
	-rm src/indexer_impl_*.cpp