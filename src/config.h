#pragma once

#include <cstdint>
#include <cassert>

#ifndef GRID_SIZE
#define GRID_SIZE 3
#endif

#ifndef MAX_ITER
#define MAX_ITER 1e7
#endif

const unsigned int BOARD_SIZE = GRID_SIZE * GRID_SIZE;
const unsigned int CANDIDATE_SIZE = BOARD_SIZE;
const unsigned int UNIT_SIZE = BOARD_SIZE;
const unsigned int CELL_COUNT = BOARD_SIZE * BOARD_SIZE;

#ifdef STRICT
#define ASSERT(cond, faild_reason) \
if (!(cond)) { throw std::runtime_error("Assertion failed: " __FILE__ ":" + std::to_string(__LINE__) + " " #cond " " faild_reason); }
#else
#define ASSERT(cond, faild_reason)
#endif

typedef uint8_t val_t;

enum class UnitType{
    ROW,
    COL,
    GRID,
};

enum class OpState
{
    SUCCESS,
    FAIL,
    SKIP, 
    VIOLATION
};
