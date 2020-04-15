#pragma once

// standard library headers
#include <vector> //always
#include <stdint.h> //uint32_t etc 
#include <fstream>
#include <algorithm>

// external library headers
#include <fmt/format.h>
#include <fmt/core.h>

// parallel pragmas doesn't matter if not loaded
#if __has_include(<omp.h>)
#  include <omp.h>
#  define NO_OMP 0
#else
#  define NO_OMP 1
#endif
// goal place #pragma omp parallel for in every embarassingly parallel case.

// static/unchanging project headers
