#pragma once

#include <cstdint>
#include <stdexcept>

#ifndef GRID_SIZE
#define GRID_SIZE 3
#endif

#ifndef MAX_ITER
#define MAX_ITER 1e6
#endif

#define BOARD_SIZE (GRID_SIZE * GRID_SIZE)

#ifdef STRICT
#define ASSERT(cond, faild_reason) \
if (!(cond)) { throw std::runtime_error("Assertion failed: " #cond " " faild_reason); }
#else
#define ASSERT(cond, faild_reason)
#endif

typedef uint8_t val_t;