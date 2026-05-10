// Catch2 unit tests for the C++ Matrix class.
//
// Run via: ctest --test-dir build --output-on-failure
//      or: ./build/test_matrix
//
// The tests mirror the Python ones (tests/test_matrix.py) so that getting
// them all green in both languages is a strong signal that your matmul
// works. The cat-sat-on-it reference is here too.
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "matrix.hpp"

#include <cmath>
#include <vector>

// ----------------------------------------------------------------------
// Chapter 6 — basic operations
// ----------------------------------------------------------------------

TEST_CASE("zeros has the requested shape and is all zero", "[basic]") {
  auto m = Matrix::zeros(2, 3);
  REQUIRE(m.rows() == 2);
  REQUIRE(m.cols() == 3);
  for (std::size_t i = 0; i < 2; ++i) {
    for (std::size_t j = 0; j < 3; ++j) {
      REQUIRE(m(i, j) == 0.0);
    }
  }
}

TEST_CASE("eye is the multiplicative identity for matmul", "[basic]") {
  const std::size_t n = 5;
  auto I = Matrix::eye(n);
  auto A = Matrix::random(n, n, /*seed=*/42);
  // Exercise the columnwise variant by default — mirrors Python's __matmul__.
  REQUIRE(I.matmul_columnwise(A) == A);
}

TEST_CASE("setitem then getitem round-trip", "[basic]") {
  auto m = Matrix::zeros(3, 3);
  m(1, 2) = 7.5;
  REQUIRE(m(1, 2) == 7.5);
  REQUIRE(m(0, 0) == 0.0);
}

TEST_CASE("from_rows shape mismatch throws", "[basic]") {
  REQUIRE_THROWS_AS(Matrix::from_rows({{1, 2, 3}, {4, 5}}), std::invalid_argument);
}

TEST_CASE("transpose is its own inverse", "[basic]") {
  auto A = Matrix::random(4, 7, /*seed=*/1);
  REQUIRE(A.transpose().transpose() == A);
}

TEST_CASE("transpose swaps the shape", "[basic]") {
  auto A = Matrix::zeros(2, 5);
  auto AT = A.transpose();
  REQUIRE(AT.rows() == 5);
  REQUIRE(AT.cols() == 2);
}

TEST_CASE("slice gives an independent copy", "[basic]") {
  auto A = Matrix::from_rows({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}});
  auto S = A.slice(0, 2, 1, 3);
  REQUIRE(S == Matrix::from_rows({{2, 3}, {5, 6}}));
  S(0, 0) = -99.0;
  REQUIRE(A(0, 1) == 2.0);
}

// ----------------------------------------------------------------------
// Chapter 7 — vector-space primitives, matvec, three matmul views
// ----------------------------------------------------------------------

namespace {

struct KnownProduct {
  Matrix A;
  Matrix B;
  Matrix C;
};

KnownProduct small_known_product() {
  auto A = Matrix::from_rows({{1, 2}, {3, 4}, {5, 6}});       // (3, 2)
  auto B = Matrix::from_rows({{7, 8, 9}, {10, 11, 12}});      // (2, 3)
  // C[0,0] = 1*7 + 2*10 = 27, etc.
  auto C = Matrix::from_rows({{27, 30, 33}, {61, 68, 75}, {95, 106, 117}});
  return {std::move(A), std::move(B), std::move(C)};
}

}  // namespace

TEST_CASE("matmul_entrywise matches the known small product", "[matmul]") {
  auto kp = small_known_product();
  REQUIRE(kp.A.matmul_entrywise(kp.B) == kp.C);
}

TEST_CASE("matmul_columnwise matches the known small product", "[matmul]") {
  auto kp = small_known_product();
  REQUIRE(kp.A.matmul_columnwise(kp.B) == kp.C);
}

TEST_CASE("matmul_outerproduct matches the known small product", "[matmul]") {
  auto kp = small_known_product();
  REQUIRE(kp.A.matmul_outerproduct(kp.B) == kp.C);
}

TEST_CASE("the three hand-written variants agree on random matrices", "[matmul]") {
  auto A = Matrix::random(6, 4, /*seed=*/11);
  auto B = Matrix::random(4, 5, /*seed=*/22);
  auto c1 = A.matmul_entrywise(B);
  auto c2 = A.matmul_columnwise(B);
  auto c3 = A.matmul_outerproduct(B);
  REQUIRE(c1 == c2);
  REQUIRE(c2 == c3);
}

TEST_CASE("each hand-written variant agrees with Eigen on random matrices", "[matmul]") {
  auto A = Matrix::random(8, 6, /*seed=*/4);
  auto B = Matrix::random(6, 7, /*seed=*/5);
  auto target = A.matmul_eigen(B);
  REQUIRE(A.matmul_entrywise(B) == target);
  REQUIRE(A.matmul_columnwise(B) == target);
  REQUIRE(A.matmul_outerproduct(B) == target);
}

TEST_CASE("matmul shape mismatch throws", "[matmul]") {
  auto A = Matrix::zeros(2, 3);
  auto B = Matrix::zeros(4, 5);
  REQUIRE_THROWS_AS(A.matmul_entrywise(B), std::invalid_argument);
}

TEST_CASE("matrix-vector via matmul (column vector)", "[matmul]") {
  auto A = Matrix::from_rows({{1, 2}, {3, 4}});
  auto x = Matrix::from_rows({{5}, {6}});  // (2, 1)
  auto y = A.matmul_entrywise(x);
  REQUIRE(y.rows() == 2);
  REQUIRE(y.cols() == 1);
  REQUIRE(y == Matrix::from_rows({{17}, {39}}));
}

TEST_CASE("associativity holds across variants", "[matmul]") {
  auto A = Matrix::random(3, 4, /*seed=*/1);
  auto B = Matrix::random(4, 5, /*seed=*/2);
  auto C = Matrix::random(5, 2, /*seed=*/3);
  auto left = A.matmul_entrywise(B).matmul_entrywise(C);
  auto right = A.matmul_entrywise(B.matmul_entrywise(C));
  REQUIRE(left == right);
}

// ----------------------------------------------------------------------
// Cat-sat-on-it reference (lecture-anchored)
// ----------------------------------------------------------------------

TEST_CASE("cat-sat-on-it scores match the lecture", "[lecture]") {
  // q (1x2) and K (5x2) crafted so unscaled scores are [3, 5, 4, 2, 6].
  auto q = Matrix::from_rows({{1.0, 0.0}});
  auto K = Matrix::from_rows({{3.0, 0.0}, {5.0, 0.0}, {4.0, 0.0}, {2.0, 0.0}, {6.0, 0.0}});

  auto scores = q.matmul_entrywise(K.transpose());
  REQUIRE(scores.rows() == 1);
  REQUIRE(scores.cols() == 5);
  const std::vector<double> expected = {3.0, 5.0, 4.0, 2.0, 6.0};
  for (std::size_t j = 0; j < expected.size(); ++j) {
    REQUIRE_THAT(scores(0, j), Catch::Matchers::WithinAbs(expected[j], 1e-9));
  }
}
