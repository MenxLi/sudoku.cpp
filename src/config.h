#pragma once

#include <cassert>
#include <cstddef>

#ifndef SIZE 
#define SIZE 9
#endif

#ifndef MAX_ITER
#define MAX_ITER 1e4
#endif

// https://stackoverflow.com/a/8625010/6775765
constexpr size_t isqrt_impl(size_t sq, size_t dlt, size_t value){
    return sq <= value ?
        isqrt_impl(sq+dlt, dlt+2, value) : (dlt >> 1) - 1;
}
constexpr size_t isqrt(size_t value){
    return isqrt_impl(1, 3, value);
}

const unsigned int BOARD_SIZE = SIZE;
constexpr unsigned int GRID_SIZE = isqrt(BOARD_SIZE);
static_assert(GRID_SIZE * GRID_SIZE == BOARD_SIZE, "SIZE must be a perfect square");

const unsigned int CANDIDATE_SIZE = BOARD_SIZE;
const unsigned int UNIT_SIZE = BOARD_SIZE;
const unsigned int CELL_COUNT = BOARD_SIZE * BOARD_SIZE;

#ifdef STRICT
#define ASSERT(cond, faild_reason) \
if (!(cond)) { throw std::runtime_error("Assertion failed: " __FILE__ ":" + std::to_string(__LINE__) + " " #cond " " faild_reason); }
#else
#define ASSERT(cond, faild_reason)
#endif

typedef unsigned short val_t;

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
