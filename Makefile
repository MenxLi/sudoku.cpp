
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
ifeq ($(OS),Windows_NT)
	UNAME_S := Windows
	LIB_SUFFIX := .lib
	BIN_SUFFIX := .exe
else
	UNAME_S := $(shell uname -s)
	LIB_SUFFIX := .o
	BIN_SUFFIX :=
endif

ifeq ($(UNAME_S),Linux)
	COMMON_FLAGS += -pthread
endif

LIB_DIR := bin/lib-$(SIZE)
BIN_DIR := bin

LIB_STAM := indexer_impl_$(SIZE) util board solver_base solver generate
TARGET_STAM := sudoku benchmark

OBJS := $(patsubst %, $(LIB_DIR)/%$(LIB_SUFFIX), $(LIB_STAM))
TEST_TARGETS := $(patsubst src/%_test.cpp, $(BIN_DIR)/test_%, $(wildcard src/*_test.cpp))

.PHONY: target test clean init

target: init $(OBJS)
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/sudoku$(BIN_SUFFIX) $(OBJS) src/main.cpp
	$(CXX) $(COMMON_FLAGS) -o $(BIN_DIR)/benchmark$(BIN_SUFFIX) $(OBJS) src/benchmark.cpp

test: init $(TEST_TARGETS)

src/indexer_impl_$(SIZE).cpp: src/indexer_gen.py
	@python src/indexer_gen.py $(SIZE)

$(LIB_DIR)/%.o: src/%.cpp
	$(CXX) $(COMMON_FLAGS) -o $@ -c $<

$(BIN_DIR)/test_%: $(OBJS) src/%.cpp
	$(CXX) $(COMMON_FLAGS) -o $@ $(OBJS) src/$*_test.cpp

init:
	@echo "\033[2mBuilding for - Platform: $(UNAME_S); Size: $(SIZE); Debug: $(DEBUG)\033[0m"
	@mkdir -p $(BIN_DIR) && mkdir -p $(LIB_DIR)

clean:
	-rm -r $(BIN_DIR)
	-rm src/indexer_impl_*.cpp