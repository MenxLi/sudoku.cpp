#pragma once

#include <cstdint>
#include <stdexcept>

#ifndef BOARD_SIZE
#define BOARD_SIZE 9
#endif

#define ASSERT(cond, faild_reason) \
if (!(cond)) { throw std::runtime_error("Assertion failed: " #cond " " faild_reason); }

typedef uint8_t val_t;