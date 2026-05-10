// Matrix — implementation.
//
// All method bodies live in this single translation unit. Eigen and the
// other implementation-only headers (<random>, <stdexcept>, <string>) are
// included here, so the public matrix.hpp stays minimal — including it
// from a TU that doesn't need the implementation costs almost nothing.

#include "matrix.hpp"

#include <Eigen/Dense>

#include <cmath>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

// File-scope using-declaration: lets us write `size_t` instead of
// `std::size_t` for the rest of the file. Safe to do here because this
// is a .cpp, not a header — the using doesn't leak to anyone who
// `#include`s matrix.hpp. We do NOT do this in the header.
using std::size_t;

// `Matrix::Data` (the storage-buffer alias declared in matrix.hpp) is
// already accessible inside member function bodies without
// qualification — class scope is in effect, so `Data` resolves
// correctly. We use it freely below.

// ======================================================================
// Construction (provided)
// ======================================================================

Matrix::Matrix(size_t rows, size_t cols)
    : rows_(rows), cols_(cols), data_(rows * cols, 0.0) {
  if (rows == 0 || cols == 0) {
    throw std::invalid_argument("Matrix: shape must be positive");
  }
}

Matrix::Matrix(size_t rows, size_t cols, Data data)
    : rows_(rows), cols_(cols), data_(std::move(data)) {
  if (rows == 0 || cols == 0) {
    throw std::invalid_argument("Matrix: shape must be positive");
  }
  if (data_.size() != rows * cols) {
    throw std::invalid_argument("Matrix: data length does not match shape");
  }
}

// ======================================================================
// Helper constructors (provided)
// ======================================================================

Matrix Matrix::zeros(size_t rows, size_t cols) {
  return Matrix(rows, cols);
}

Matrix Matrix::eye(size_t n) {
  Matrix m(n, n);
  for (size_t i = 0; i < n; ++i) {
    m.data_[i * n + i] = 1.0;
  }
  return m;
}

Matrix Matrix::from_rows(const std::vector<Data>& rows) {
  if (rows.empty()) {
    throw std::invalid_argument("from_rows: at least one row required");
  }
  const size_t nrows = rows.size();
  const size_t ncols = rows.front().size();
  Data flat;
  flat.reserve(nrows * ncols);
  for (const auto& r : rows) {
    if (r.size() != ncols) {
      throw std::invalid_argument("from_rows: rows must have equal length");
    }
    flat.insert(flat.end(), r.begin(), r.end());
  }
  return Matrix(nrows, ncols, std::move(flat));
}

// Stack a vector of (rows, 1) column matrices into one wide matrix.
// Each element of `columns` must be a (rows, 1) Matrix of the same height.
// The result is a (rows, columns.size()) Matrix whose j-th column is
// `columns[j]`. This is the inverse of get_column.
//
// Used by matmul_columnwise to assemble per-column matvec results into a
// single output without an explicit nested copy loop at the call site.
Matrix Matrix::from_columns(const std::vector<Matrix>& columns) {
  if (columns.empty()) {
    throw std::invalid_argument("from_columns: at least one column required");
  }
  const size_t rows = columns.front().rows_;
  const size_t n = columns.size();
  for (size_t j = 0; j < n; ++j) {
    if (columns[j].rows_ != rows || columns[j].cols_ != 1) {
      throw std::invalid_argument(
          "from_columns: column " + std::to_string(j) + " has shape (" +
          std::to_string(columns[j].rows_) + ", " +
          std::to_string(columns[j].cols_) + "), expected (" +
          std::to_string(rows) + ", 1)");
    }
  }
  Matrix out(rows, n);
  for (size_t j = 0; j < n; ++j) {
    for (size_t i = 0; i < rows; ++i) {
      out.data_[i * n + j] = columns[j].data_[i];
    }
  }
  return out;
}

Matrix Matrix::random(size_t rows, size_t cols, unsigned seed) {
  std::mt19937 rng(seed);
  std::uniform_real_distribution<double> dist(-1.0, 1.0);
  Data flat(rows * cols);
  for (auto& x : flat) {
    x = dist(rng);
  }
  return Matrix(rows, cols, std::move(flat));
}

// ======================================================================
// Chapter 6 — basic operations (TODO)
// ======================================================================

// ----------------------------------------------------------------------
// 6.1 element access
// ----------------------------------------------------------------------
double Matrix::operator()(size_t i, size_t j) const {
  // TODO 6.1: return data_[i * cols_ + j], with bounds-checking.
  (void)i;
  (void)j;
  throw std::logic_error("TODO 6.1 — implement operator() (const)");
}

double& Matrix::operator()(size_t i, size_t j) {
  // TODO 6.1: return data_[i * cols_ + j] by reference, with bounds-checking.
  (void)i;
  (void)j;
  throw std::logic_error("TODO 6.1 — implement operator() (mutable)");
}

// ----------------------------------------------------------------------
// 6.2 equality with tolerance
// ----------------------------------------------------------------------
bool Matrix::approx_equal(const Matrix& other, double tol) const {
  // TODO 6.2: return true iff the shapes match AND every entry is within tol.
  (void)other;
  (void)tol;
  throw std::logic_error("TODO 6.2 — implement approx_equal");
}

// ----------------------------------------------------------------------
// 6.3 transpose
// ----------------------------------------------------------------------
Matrix Matrix::transpose() const {
  // TODO 6.3: return a new Matrix of shape (cols_, rows_) where
  // result(j, i) == (*this)(i, j).
  throw std::logic_error("TODO 6.3 — implement transpose");
}

// ----------------------------------------------------------------------
// 6.4 slice — half-open intervals [r0, r1) x [c0, c1)
// ----------------------------------------------------------------------
Matrix Matrix::slice(size_t r0, size_t r1,
                     size_t c0, size_t c1) const {
  // TODO 6.4: validate ranges, build a new Matrix of shape (r1-r0, c1-c0)
  // by copying the sub-block.
  (void)r0;
  (void)r1;
  (void)c0;
  (void)c1;
  throw std::logic_error("TODO 6.4 — implement slice");
}

// ----------------------------------------------------------------------
// Helpers (provided) — composed on top of slice
//
// Per our shape contract, vectors ARE matrices: a row is a (1, cols)
// Matrix, a column is a (rows, 1) Matrix. These helpers are sugar over
// slice so the matmul variants in chapter 7 don't have to spell out the
// half-open form every time. They are not separate TODOs; once you
// implement slice (TODO 6.4), both work for free.
// ----------------------------------------------------------------------
Matrix Matrix::get_row(size_t i) const {
  return slice(i, i + 1, 0, cols_);
}

Matrix Matrix::get_column(size_t j) const {
  return slice(0, rows_, j, j + 1);
}

// ======================================================================
// Chapter 7 — vector-space primitives, matvec, three matmul views
// ======================================================================
// The chapter builds matmul up from primitives. The order matters; each
// TODO uses what came before. matmul_columnwise has no nested loops at
// its level — the algorithmic work moved into matvec, the assembly
// moved into from_columns. matmul_entrywise and matmul_outerproduct are
// the brute-force counterpoints (the textbook triple loop and the
// in-place rank-1 accumulation). Each maps to a different memory-access
// pattern in row-major storage — chapter 7 explains why their wall-clock
// times differ even though their flop counts are identical.

// ----------------------------------------------------------------------
// Vector-space operations on matrices
// ----------------------------------------------------------------------

// 7.1 — Matrix addition
//
// Linear-algebra context: matrices of a fixed shape form a *vector
// space*. Addition is the first of its two defining operations.
// Geometrically, if A and B are linear maps, A + B is the map that
// applies both to the same input and adds the outputs.
Matrix Matrix::operator+(const Matrix& other) const {
  // TODO 7.1: return a new Matrix with the same shape as *this/other,
  // whose entries are the elementwise sum. Throw std::invalid_argument
  // on shape mismatch.
  (void)other;
  throw std::logic_error("TODO 7.1 — implement operator+");
}

// 7.2 — Scalar multiplication (scalar on the right)
//
// Linear-algebra context: the second vector-space operation. Together
// with addition, scalar multiplication makes (rows, cols)-matrices a
// real vector space. Geometrically, c * A is the linear map A whose
// outputs are uniformly scaled by c.
//
// Note: this is the elementwise scaling c * A_ij, *not* matrix-matrix
// multiplication. There is intentionally no `Matrix * Matrix` operator
// in this class — call the matmul_* methods explicitly so it's always
// clear which view of matmul you're using.
Matrix Matrix::operator*(double scalar) const {
  // TODO 7.2: return a new Matrix with the same shape, where each
  // entry is multiplied by `scalar`. (See the free `operator*` below
  // — we get `2.0 * A` for free by calling this.)
  (void)scalar;
  throw std::logic_error("TODO 7.2 — implement operator* (scalar)");
}

// ----------------------------------------------------------------------
// Matrix-vector product — the primitive matmul is built on
// ----------------------------------------------------------------------

// 7.3 — Matrix-vector product (column-space view)
//
// Linear-algebra context: this implementation makes the *column-space*
// view of matrix-vector multiplication explicit. The product A * v is
// a linear combination of the columns of A, with the entries of v as
// coefficients:
//
//     A * v = v(0) * A(:, 0)
//           + v(1) * A(:, 1)
//           + ...
//           + v(k-1) * A(:, k-1)
//
// Read it as: "the i-th entry of v says how much of the i-th column of
// A to take; the output is the sum of those scaled columns." This is
// why a matrix's column space *is* the set of all possible outputs of
// A * v as v varies.
Matrix Matrix::matvec(const Matrix& vec) const {
  if (vec.rows_ != cols_ || vec.cols_ != 1) {
    throw std::invalid_argument(
        "matvec shape mismatch: (" + std::to_string(rows_) + ", " +
        std::to_string(cols_) + ") * (" + std::to_string(vec.rows_) + ", " +
        std::to_string(vec.cols_) + ") (expected vec to be (" +
        std::to_string(cols_) + ", 1))");
  }
  // TODO 7.3: implement matvec as the linear combination of self's
  // columns described above. The two operations you need are matrix
  // addition (TODO 7.1) and scalar multiplication (TODO 7.2), applied
  // to columns of self (use get_column). There should be no nested
  // for loops in your implementation — that's the structural point of
  // building this on top of the vector-space primitives.
  throw std::logic_error("TODO 7.3 — implement matvec");
}

// ----------------------------------------------------------------------
// Three views of matrix-matrix multiplication
// ----------------------------------------------------------------------

// 7.4 — column-by-column matmul (built on matvec)
//
// Linear-algebra context: matrix-matrix multiplication is matrix-vector
// multiplication applied independently to each column of the right
// operand, with the results stacked as columns of the output:
//
//     C(:, j) = A * B(:, j)   for each column j of B.
//
// Read it as: "B is a stack of input vectors; multiplying by A applies
// the same linear map to each one." Same flop count as the other two
// views, but the *structure* of the code mirrors this story — the
// outer loop walks columns, the per-column work is a single matvec.
Matrix Matrix::matmul_columnwise(const Matrix& other) const {
  check_matmul_compatible(*this, other);
  // TODO 7.4: the j-th column of the output is matvec applied to the
  // j-th column of `other`. Build a vector of those output columns
  // and assemble them into the result with the provided
  // `Matrix::from_columns` factory. There should be no explicit
  // nested for loops at this level — the algorithmic work moved
  // into matvec, the assembly moved into from_columns.
  (void)other;
  throw std::logic_error("TODO 7.4 — implement matmul_columnwise");
}

// 7.5 — entry-wise textbook triple loop
//
//     C(i, j) = sum_k A(i, k) * B(k, j)
//
// Linear-algebra context: the most elementary view — each output
// entry is a dot product of one row of A with one column of B. Three
// explicit nested loops, no decomposition into primitives. We keep
// this method deliberately as the *brute-force baseline*: a comparison
// point for benchmarks (chapter 8) and a reminder of what the math
// literally says before any abstraction is applied.
//
// Memory note: the natural (i, j, k) loop order means the innermost
// loop touches `B(k, j)` — striding through B by `cols_` doubles per
// iteration, jumping over a cache line each time. Compare with
// matmul_outerproduct below, which walks contiguous memory.
Matrix Matrix::matmul_entrywise(const Matrix& other) const {
  check_matmul_compatible(*this, other);
  // TODO 7.5: implement the explicit triple loop directly per the
  // formula above. Unlike the columnwise variant, this method is
  // supposed to have nested loops — that's its whole point.
  (void)other;
  throw std::logic_error("TODO 7.5 — implement matmul_entrywise");
}

// 7.6 — sum of rank-1 outer products (in-place accumulation)
//
//     C = sum_k A(:, k) * B(k, :)
//
// where each term is a rank-1 (rows × cols) matrix obtained from
// column k of A and row k of B.
//
// Linear-algebra context: a different decomposition of the same total
// work. The sum has rank up to k. Outer-product accumulation is how
// you'd describe the operation if you were thinking of A and B as
// sums of rank-1 pieces — useful in low-rank approximation, attention
// mechanisms (Q K^T), and SVD.
//
// Memory note: walk k outer, then i, then j, and accumulate
// `A(i, k) * B(k, j)` into the running output entry at (i, j). The
// innermost j loop now walks contiguous memory in both `B(k, :)` and
// `C(i, :)` — same flop count as matmul_entrywise, but the cache
// behaviour is dramatically better. Expect a 5–10× speedup at -O3
// for medium n.
//
// Do *not* allocate every rank-k intermediate as its own Matrix and
// store them in a vector — that uses k times more memory than
// necessary. Accumulate in place into a single output.
Matrix Matrix::matmul_outerproduct(const Matrix& other) const {
  check_matmul_compatible(*this, other);
  // TODO 7.6: implement the sum-of-outer-products view per the
  // formula above. Each rank-1 term is `column_k(A) * row_k(B)`, an
  // (rows_, other.cols_) matrix whose entry at (i, j) is
  // A(i, k) * B(k, j). Accumulate these contributions in place into a
  // single output Matrix — do not build a vector of rank-1 matrices
  // and sum them at the end. (The "Memory note" above tells you which
  // loop order makes the accumulation cache-friendly.)
  (void)other;
  throw std::logic_error("TODO 7.6 — implement matmul_outerproduct");
}

// ----------------------------------------------------------------------
// Eigen reference (provided) — used in chapter 8 for the speedup chart
//
// Wraps `data_` in `Eigen::Map` (a zero-copy view) and lets Eigen do
// the multiplication. This is the C++ analogue of matrix.py's
// matmul_numpy: a reference implementation we benchmark against, not
// a TODO. Eigen will dispatch to BLAS where useful and use SIMD
// intrinsics in its own kernels — chapter 8 explains why it's faster
// than your hand-written variants.
// ----------------------------------------------------------------------
Matrix Matrix::matmul_eigen(const Matrix& other) const {
  check_matmul_compatible(*this, other);
  using EigenMat =
      Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
  Eigen::Map<const EigenMat> A(data_.data(), rows_, cols_);
  Eigen::Map<const EigenMat> B(other.data_.data(), other.rows_, other.cols_);
  Matrix C(rows_, other.cols_);
  Eigen::Map<EigenMat> Ceig(C.data_.data(), C.rows_, C.cols_);
  Ceig.noalias() = A * B;
  return C;
}

// ----------------------------------------------------------------------
// Private helpers
// ----------------------------------------------------------------------
void Matrix::check_matmul_compatible(const Matrix& a, const Matrix& b) {
  if (a.cols_ != b.rows_) {
    throw std::invalid_argument(
        "matmul shape mismatch: (" + std::to_string(a.rows_) + ", " +
        std::to_string(a.cols_) + ") x (" + std::to_string(b.rows_) + ", " +
        std::to_string(b.cols_) + ")");
  }
}
