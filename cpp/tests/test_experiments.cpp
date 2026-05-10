// Experiments and starter tests for the C++ Matrix class.
//
// Mirrors python/tests/test_experiments.py. Every test below passes
// from day one because it exercises only the *provided* parts of the
// class: constructors, factory static methods (zeros, eye, from_rows,
// random), and the raw rows() / cols() / data() accessors. None of it
// depends on any TODO. Use these as a starting point: modify them,
// add your own, drop in std::cout debug prints, and re-run.
//
// Run only these tests:
//
//     ./build/test_matrix "[experiments]"     # Catch2 tag filter
//
// Or via ctest by name pattern:
//
//     ctest --test-dir build -R "experiment|constructor|factory"
//
// All tests in this file are tagged [experiments] so the bracketed
// form above is the cleanest. See docs/06 for the full tag table.
//
// Two helpers from matrix_debug.hpp are used below:
//
//     dump_raw(m)        -> a string showing rows, cols, and the raw
//                           flat data buffer. Use this any time you
//                           want to see the underlying storage.
//     memory_bytes(m)    -> a rough estimate of how many bytes the
//                           Matrix uses, including the data buffer.

#include <catch2/catch_test_macros.hpp>

#include "matrix.hpp"
#include "matrix_debug.hpp"

#include <cstddef>
#include <string>
#include <vector>

// ----------------------------------------------------------------------
// Construction — verify the provided machinery works
// ----------------------------------------------------------------------

TEST_CASE("primary constructor takes (rows, cols, data)", "[experiments]") {
  Matrix m(2, 3, {1, 2, 3, 4, 5, 6});
  REQUIRE(m.rows() == 2);
  REQUIRE(m.cols() == 3);
  // Storage is row-major: entry (i, j) lives at flat index i*cols + j.
  // Try it yourself: std::cout << dump_raw(m) << '\n';
  REQUIRE(m.data() == Matrix::Data{1, 2, 3, 4, 5, 6});
}

TEST_CASE("zeros factory makes a fresh matrix of zeros", "[experiments]") {
  auto m = Matrix::zeros(3, 4);
  REQUIRE(m.rows() == 3);
  REQUIRE(m.cols() == 4);
  REQUIRE(m.data().size() == 12);
  for (auto x : m.data()) {
    REQUIRE(x == 0.0);
  }
}

TEST_CASE("eye places ones on the diagonal", "[experiments]") {
  auto m = Matrix::eye(3);
  REQUIRE(m.rows() == 3);
  REQUIRE(m.cols() == 3);
  // 3x3 identity laid out row-major:
  REQUIRE(m.data() == Matrix::Data{
                          1, 0, 0,
                          0, 1, 0,
                          0, 0, 1});
}

TEST_CASE("from_rows matches the primary constructor's layout", "[experiments]") {
  auto a = Matrix(2, 2, {1, 2, 3, 4});
  auto b = Matrix::from_rows({{1, 2}, {3, 4}});
  REQUIRE(a.data() == b.data());
  REQUIRE(a.rows() == b.rows());
  REQUIRE(a.cols() == b.cols());
}

TEST_CASE("random is reproducible with a seed", "[experiments]") {
  // Same seed -> same data. Different seed -> different data.
  auto a = Matrix::random(2, 2, /*seed=*/42);
  auto b = Matrix::random(2, 2, /*seed=*/42);
  auto c = Matrix::random(2, 2, /*seed=*/43);
  REQUIRE(a.data() == b.data());
  REQUIRE(a.data() != c.data());
}

// ----------------------------------------------------------------------
// Inspection — see the raw storage without going through operator()
// ----------------------------------------------------------------------

TEST_CASE("dump_raw shows shape and flat data", "[experiments]") {
  auto m = Matrix::from_rows({{1, 2}, {3, 4}});
  auto s = dump_raw(m);
  // The shape numbers and the flat data should both appear in the string.
  REQUIRE(s.find("rows=2") != std::string::npos);
  REQUIRE(s.find("cols=2") != std::string::npos);
  REQUIRE(s.find("1") != std::string::npos);
  REQUIRE(s.find("4") != std::string::npos);
}

// ----------------------------------------------------------------------
// Memory — why C++ doubles are cheap (preview of chapters 5 and 8)
// ----------------------------------------------------------------------

TEST_CASE("a single C++ double is exactly 8 bytes", "[experiments]") {
  // C++ is the opposite of Python here: a `double` is inline in the
  // buffer, no heap-allocated wrapper, no refcount, no type tag. The
  // sizeof is exactly the IEEE-754 double's 8 bytes. This is one of
  // the four reasons your C++ matmul will beat your Python matmul by
  // orders of magnitude even before any -O3 optimisation.
  REQUIRE(sizeof(double) == 8);
}

TEST_CASE("larger matrices use more memory", "[experiments]") {
  auto small = Matrix::zeros(2, 2);    // 4 doubles
  auto large = Matrix::zeros(20, 20);  // 400 doubles
  REQUIRE(memory_bytes(small) < memory_bytes(large));
}

TEST_CASE("a 256x256 matrix fits in ~0.5 MB", "[experiments]") {
  // 256 * 256 * 8 bytes = 524,288 bytes (the data buffer).
  // Compare to the Python equivalent in chapter 5: ~3-4× larger
  // because every Python float is a heap-allocated object with ~28
  // bytes of overhead. C++'s std::vector<double> is essentially a
  // packed array of doubles, just like NumPy's underlying buffer.
  auto m = Matrix::zeros(256, 256);
  const std::size_t bytes = memory_bytes(m);
  const std::size_t buffer_bytes = 256 * 256 * sizeof(double);
  REQUIRE(bytes >= buffer_bytes);
  // Slack for sizeof(Matrix) plus any std::vector reserve overhead.
  REQUIRE(bytes < buffer_bytes + 256);
}
