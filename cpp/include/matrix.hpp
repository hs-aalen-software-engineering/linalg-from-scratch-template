// Matrix — declarations.
//
// Implementation lives in src/matrix.cpp. The split between header and
// source is the standard C++ pattern: declare here, define there. This
// keeps each translation unit lean — including matrix.hpp doesn't drag
// in Eigen, <random>, <stdexcept>, etc. — and lets the implementation
// change without recompiling every consumer.
//
// Mirrors the Python Matrix class in matrix.py: same shape semantics,
// same row-major storage layout, same vector-space-primitives → matvec
// → matmul build-up. Vectors are matrices of shape (n, 1) or (1, n);
// there is no separate Vector class.
#pragma once

#include <cstddef>
#include <vector>

class Matrix {
public:
  // ------------------------------------------------------------------
  // Type aliases
  // ------------------------------------------------------------------
  // The flat row-major storage buffer. Public so that callers (tests,
  // the benchmark, matrix_debug.cpp) can write `Matrix::Data` instead
  // of `std::vector<double>`. This is the same pattern the standard
  // library uses for class-specific types: std::vector::value_type,
  // std::map::key_type, etc. The compiler treats `Matrix::Data` and
  // `std::vector<double>` as the same type — the alias is purely a
  // readability convention.
  using Data = std::vector<double>;

  // ------------------------------------------------------------------
  // Construction (provided)
  // ------------------------------------------------------------------
  Matrix(std::size_t rows, std::size_t cols);
  Matrix(std::size_t rows, std::size_t cols, Data data);

  // ------------------------------------------------------------------
  // Helper constructors (provided)
  // ------------------------------------------------------------------
  static Matrix zeros(std::size_t rows, std::size_t cols);
  static Matrix eye(std::size_t n);
  static Matrix from_rows(const std::vector<Data>& rows);
  static Matrix from_columns(const std::vector<Matrix>& columns);
  static Matrix random(std::size_t rows, std::size_t cols, unsigned seed);

  // ------------------------------------------------------------------
  // Shape and raw access (provided, inline)
  // ------------------------------------------------------------------
  std::size_t rows() const noexcept { return rows_; }
  std::size_t cols() const noexcept { return cols_; }
  const Data& data() const noexcept { return data_; }
  Data& data() noexcept { return data_; }

  // ==================================================================
  // Chapter 6 — basic operations (TODO; bodies in src/matrix.cpp)
  // ==================================================================
  double operator()(std::size_t i, std::size_t j) const;       // 6.1
  double& operator()(std::size_t i, std::size_t j);            // 6.1
  bool approx_equal(const Matrix& other, double tol = 1e-9) const;  // 6.2
  Matrix transpose() const;                                    // 6.3
  Matrix slice(std::size_t r0, std::size_t r1,
               std::size_t c0, std::size_t c1) const;          // 6.4

  // Equality is a one-line delegate (inline).
  bool operator==(const Matrix& other) const { return approx_equal(other); }
  bool operator!=(const Matrix& other) const { return !(*this == other); }

  // ------------------------------------------------------------------
  // Helpers (provided) — composed on top of slice
  // ------------------------------------------------------------------
  Matrix get_row(std::size_t i) const;
  Matrix get_column(std::size_t j) const;

  // ==================================================================
  // Chapter 7 — vector-space primitives, matvec, three matmul views
  //             (TODO; bodies in src/matrix.cpp)
  // ==================================================================
  Matrix operator+(const Matrix& other) const;                 // 7.1
  Matrix operator*(double scalar) const;                       // 7.2
  Matrix matvec(const Matrix& vec) const;                      // 7.3
  Matrix matmul_columnwise(const Matrix& other) const;         // 7.4
  Matrix matmul_entrywise(const Matrix& other) const;          // 7.5
  Matrix matmul_outerproduct(const Matrix& other) const;       // 7.6

  // Eigen reference (provided) — used in chapter 8 for the speedup chart.
  // Body is in src/matrix.cpp; that's also the only TU that needs to
  // include <Eigen/Dense>, so consumers of matrix.hpp don't pay for it.
  Matrix matmul_eigen(const Matrix& other) const;

private:
  static void check_matmul_compatible(const Matrix& a, const Matrix& b);

  std::size_t rows_;
  std::size_t cols_;
  Data data_;
};

// Free function for `2.0 * matrix` (mirrors Python's __rmul__). One-line
// delegate, kept inline so callers don't need a separate TU lookup.
inline Matrix operator*(double scalar, const Matrix& m) { return m * scalar; }
