#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <cassert>

#ifndef GRID_SIZE
#define GRID_SIZE 3
#endif

#ifndef MAX_ITER
#define MAX_ITER 1e7
#endif

#define BOARD_SIZE (GRID_SIZE * GRID_SIZE)

#ifdef STRICT
#define ASSERT(cond, faild_reason) \
if (!(cond)) { throw std::runtime_error("Assertion failed: " __FILE__ ":" + std::to_string(__LINE__) + " " #cond " " faild_reason); }
#else
#define ASSERT(cond, faild_reason)
#endif

typedef uint8_t val_t;

enum class UnitType{
    GRID,
    ROW,
    COL
};

enum class OpState
{
    SUCCESS,
    FAIL,
    SKIP, 
    VIOLATION
};
