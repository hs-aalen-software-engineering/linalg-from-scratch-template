// matrix_debug.hpp — inspection helpers for Matrix instances.
//
// Mirrors python/matrix/debug.py. Provided so you can experiment with
// the Matrix class *before* the chapter-6 TODO methods are implemented:
// dump_raw and memory_bytes only touch the public accessors (rows(),
// cols(), data()), so they don't depend on any TODO body.
//
// Use them in tests/test_experiments.cpp and any scratch tests of your
// own to see what's actually stored in memory.
#pragma once

#include "matrix.hpp"

#include <cstddef>
#include <string>

// Return a debug string showing the raw flat storage of a Matrix.
//
// Output looks like:
//
//     Matrix(rows=2, cols=3, data=[1, 2, 3, 4, 5, 6])
//
// Useful for seeing what's actually in memory (the row-major layout)
// without needing the chapter-6 element-access TODO (operator()) to be
// implemented yet.
std::string dump_raw(const Matrix& m);

// Estimate the memory footprint of a Matrix in bytes.
//
// Sums two contributions:
//
// * `sizeof(Matrix)` — the object's own storage (the rows_, cols_
//   integers and the std::vector's control structure: pointer, size,
//   capacity).
// * `m.data().capacity() * sizeof(double)` — the heap-allocated
//   buffer of doubles.
//
// Compare with python/matrix/debug.py's memory_bytes: a Python float
// is itself a heap-allocated object with a refcount, type pointer,
// and value (~28 bytes per element). In C++, `double` is *inline* in
// the std::vector buffer — 8 bytes per element, no wrapper. That's
// one of the four reasons your C++ matmul will beat your Python
// matmul by orders of magnitude even before any -O3 optimisation.
std::size_t memory_bytes(const Matrix& m);
