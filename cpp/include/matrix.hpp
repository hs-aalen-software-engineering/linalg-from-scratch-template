// Matrix — header-only, flat row-major storage.
//
// Mirrors the Python Matrix class in matrix.py: same shape semantics, same
// storage layout, same three matmul views. Vectors are matrices of shape
// (n, 1) or (1, n); there is no separate Vector class.
//
// We keep this header-only so the whole class is in one place and one
// compilation unit. For the size of matrices we use here that is fine.
//
// All functions are inlined inside the class body. The methods you must
// implement are marked with TODO and throw if called.
#pragma once

#include <Eigen/Dense>

#include <cmath>
#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

class Matrix {
public:
  // ------------------------------------------------------------------
  // Construction (provided)
  // ------------------------------------------------------------------
  Matrix(std::size_t rows, std::size_t cols)
      : rows_(rows), cols_(cols), data_(rows * cols, 0.0) {
    if (rows == 0 || cols == 0) {
      throw std::invalid_argument("Matrix: shape must be positive");
    }
  }

  Matrix(std::size_t rows, std::size_t cols, std::vector<double> data)
      : rows_(rows), cols_(cols), data_(std::move(data)) {
    if (rows == 0 || cols == 0) {
      throw std::invalid_argument("Matrix: shape must be positive");
    }
    if (data_.size() != rows * cols) {
      throw std::invalid_argument("Matrix: data length does not match shape");
    }
  }

  // ------------------------------------------------------------------
  // Helper constructors (provided)
  // ------------------------------------------------------------------
  static Matrix zeros(std::size_t rows, std::size_t cols) {
    return Matrix(rows, cols);
  }

  static Matrix eye(std::size_t n) {
    Matrix m(n, n);
    for (std::size_t i = 0; i < n; ++i) {
      m.data_[i * n + i] = 1.0;
    }
    return m;
  }

  static Matrix from_rows(const std::vector<std::vector<double>>& rows) {
    if (rows.empty()) {
      throw std::invalid_argument("from_rows: at least one row required");
    }
    const std::size_t nrows = rows.size();
    const std::size_t ncols = rows.front().size();
    std::vector<double> flat;
    flat.reserve(nrows * ncols);
    for (const auto& r : rows) {
      if (r.size() != ncols) {
        throw std::invalid_argument("from_rows: rows must have equal length");
      }
      flat.insert(flat.end(), r.begin(), r.end());
    }
    return Matrix(nrows, ncols, std::move(flat));
  }

  static Matrix random(std::size_t rows, std::size_t cols, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    std::vector<double> flat(rows * cols);
    for (auto& x : flat) {
      x = dist(rng);
    }
    return Matrix(rows, cols, std::move(flat));
  }

  // ------------------------------------------------------------------
  // Shape and raw access (provided)
  // ------------------------------------------------------------------
  std::size_t rows() const noexcept { return rows_; }
  std::size_t cols() const noexcept { return cols_; }
  const std::vector<double>& data() const noexcept { return data_; }
  std::vector<double>& data() noexcept { return data_; }

  // ------------------------------------------------------------------
  // Chapter 6.1 — element access (TODO)
  // operator() with two indices; bounds-check in debug, just compute the
  // flat index i*cols + j.
  // ------------------------------------------------------------------
  double operator()(std::size_t i, std::size_t j) const {
    // TODO 6.1: return data_[i * cols_ + j], with bounds-checking.
    (void)i;
    (void)j;
    throw std::logic_error("TODO 6.1 — implement operator() (const)");
  }

  double& operator()(std::size_t i, std::size_t j) {
    // TODO 6.1: return data_[i * cols_ + j] by reference, with bounds-checking.
    (void)i;
    (void)j;
    throw std::logic_error("TODO 6.1 — implement operator() (mutable)");
  }

  // ------------------------------------------------------------------
  // Chapter 6.2 — equality with tolerance (TODO)
  // ------------------------------------------------------------------
  bool approx_equal(const Matrix& other, double tol = 1e-9) const {
    // TODO 6.2: return true iff the shapes match AND every entry is within tol.
    (void)other;
    (void)tol;
    throw std::logic_error("TODO 6.2 — implement approx_equal");
  }

  bool operator==(const Matrix& other) const { return approx_equal(other); }
  bool operator!=(const Matrix& other) const { return !(*this == other); }

  // ------------------------------------------------------------------
  // Chapter 6.3 — transpose (TODO)
  // ------------------------------------------------------------------
  Matrix transpose() const {
    // TODO 6.3: return a new Matrix of shape (cols_, rows_) where
    // result(j, i) == (*this)(i, j).
    throw std::logic_error("TODO 6.3 — implement transpose");
  }

  // ------------------------------------------------------------------
  // Chapter 6.4 — slice (TODO)
  // Half-open interval [r0, r1) x [c0, c1).
  // ------------------------------------------------------------------
  Matrix slice(std::size_t r0, std::size_t r1, std::size_t c0, std::size_t c1) const {
    // TODO 6.4: validate ranges, build a new Matrix of shape (r1-r0, c1-c0)
    // by copying the sub-block.
    (void)r0;
    (void)r1;
    (void)c0;
    (void)c1;
    throw std::logic_error("TODO 6.4 — implement slice");
  }

  // ------------------------------------------------------------------
  // Chapter 7 — three matmul implementations (TODO)
  // ------------------------------------------------------------------

  // 7.1: classic textbook triple loop in i-j-k order.
  //   C(i, j) = sum_k A(i, k) * B(k, j)
  // The inner k loop strides through B by `cols` per iteration, which
  // jumps across cache lines: this is the "wrong" order in C++.
  Matrix matmul_ijk(const Matrix& other) const {
    check_matmul_compatible(*this, other);
    // TODO 7.1: triple loop, i then j then k. Allocate C of shape
    // (rows_, other.cols_) and fill.
    (void)other;
    throw std::logic_error("TODO 7.1 — implement matmul_ijk");
  }

  // 7.2: same algorithm, loops reordered to i-k-j.
  //   for i: for k: for j: C(i, j) += A(i, k) * B(k, j)
  // The hot inner loop now strides through B AND C row by row — both
  // contiguous in row-major. Expect a 5-10x speedup vs ijk for medium n.
  Matrix matmul_ikj(const Matrix& other) const {
    check_matmul_compatible(*this, other);
    // TODO 7.2: same as ijk but with the i-k-j loop order.
    (void)other;
    throw std::logic_error("TODO 7.2 — implement matmul_ikj");
  }

  // 7.3: hand off to Eigen via Eigen::Map (zero-copy view of our data).
  Matrix matmul_eigen(const Matrix& other) const {
    check_matmul_compatible(*this, other);
    // TODO 7.3:
    //   using EigenMat = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
    //   Eigen::Map<const EigenMat> A(data_.data(), rows_, cols_);
    //   Eigen::Map<const EigenMat> B(other.data_.data(), other.rows_, other.cols_);
    //   Matrix C(rows_, other.cols_);
    //   Eigen::Map<EigenMat> Ceig(C.data_.data(), C.rows_, C.cols_);
    //   Ceig.noalias() = A * B;
    //   return C;
    (void)other;
    throw std::logic_error("TODO 7.3 — implement matmul_eigen");
  }

private:
  static void check_matmul_compatible(const Matrix& a, const Matrix& b) {
    if (a.cols_ != b.rows_) {
      throw std::invalid_argument(
          "matmul shape mismatch: (" + std::to_string(a.rows_) + ", " +
          std::to_string(a.cols_) + ") x (" + std::to_string(b.rows_) + ", " +
          std::to_string(b.cols_) + ")");
    }
  }

  std::size_t rows_;
  std::size_t cols_;
  std::vector<double> data_;
};
