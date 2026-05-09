# 7 — Three matmuls in C++

**Goal**: implement `matmul_ijk`, `matmul_ikj`, and `matmul_eigen`. Then build at three optimisation levels and compare. This is where the cache-locality and compiler-optimisation aha-moments live.

## ijk vs ikj — same algorithm, different speed

For `C = A * B` with row-major storage, the textbook triple-loop comes in two natural flavours:

**ijk** (the order you wrote in Python):

```cpp
for (i = 0..M)
  for (j = 0..N)
    for (k = 0..K)
      C(i, j) += A(i, k) * B(k, j);
```

The innermost loop touches `B(k, j)` — row `k` of `B`. As `k` increments, you stride forward by `cols(B)` doubles, jumping over a cache line each time. Slow.

**ikj** (what you'll write next):

```cpp
for (i = 0..M)
  for (k = 0..K)
    for (j = 0..N)
      C(i, j) += A(i, k) * B(k, j);
```

Now the innermost loop touches `B(k, j)` for fixed `k` and increasing `j` — row `k` of `B`, walking *forward through memory*, one cache line at a time. The CPU prefetcher loves this. Same flop count, 5–10× faster on most modern hardware.

This is *not* an algorithmic improvement. It's about how the data flows through the cache hierarchy.

## What to fill in

In [`cpp/include/matrix.hpp`](../cpp/include/matrix.hpp):

- `matmul_ijk` — TODO 7.1 — straightforward triple loop, ijk order
- `matmul_ikj` — TODO 7.2 — same logic, reordered loops
- `matmul_eigen` — TODO 7.3 — `Eigen::Map` to wrap the data, return `A * B`

The Eigen body is the most fiddly because of the type names. The TODO has the entire body in a comment — you can copy it almost verbatim.

## Build at three optimisation levels

This is the second aha-moment. The compiler has *enormous* leverage over your loop. Configure three separate build directories so you can compare:

```bash
cd cpp
cmake -B build-O0 -DCMAKE_BUILD_TYPE=Debug          && cmake --build build-O0 --parallel
cmake -B build-O2 -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build-O2 --parallel
cmake -B build-O3 -DCMAKE_BUILD_TYPE=Release        && cmake --build build-O3 --parallel
```

Then verify each builds and tests pass:

```bash
ctest --test-dir build-O3 --output-on-failure
```

## Verify

Run the matmul-tagged tests in any of the build directories:

```bash
ctest --test-dir build-O3 -R matmul --output-on-failure
```

You're done with chapter 7 when:

- All `[matmul]`-tagged tests pass.
- All three matmul implementations agree (`test the three variants agree on random matrices`).
- Eigen agrees with your hand-written variants.

## Hints

<details>
<summary>What's the right body for matmul_ikj?</summary>

```cpp
Matrix matmul_ikj(const Matrix& other) const {
  check_matmul_compatible(*this, other);
  Matrix C(rows_, other.cols_);
  for (std::size_t i = 0; i < rows_; ++i) {
    for (std::size_t k = 0; k < cols_; ++k) {
      const double a_ik = (*this)(i, k);
      for (std::size_t j = 0; j < other.cols_; ++j) {
        C(i, j) += a_ik * other(k, j);
      }
    }
  }
  return C;
}
```

The `const double a_ik` lift is a standard trick: hoist a value out of the innermost loop so the compiler doesn't reload it from memory each iteration. The compiler often does this for you, but writing it explicitly makes the intent clear.

</details>

<details>
<summary>What's the right body for matmul_eigen?</summary>

```cpp
Matrix matmul_eigen(const Matrix& other) const {
  check_matmul_compatible(*this, other);
  using EigenMat = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
  Eigen::Map<const EigenMat> A(data_.data(), rows_, cols_);
  Eigen::Map<const EigenMat> B(other.data_.data(), other.rows_, other.cols_);
  Matrix C(rows_, other.cols_);
  Eigen::Map<EigenMat> Ceig(C.data_.data(), C.rows(), C.cols());
  Ceig.noalias() = A * B;
  return C;
}
```

`Eigen::Map` is a zero-copy view: it interprets your existing `std::vector<double>` as an Eigen matrix without allocating. `.noalias()` tells Eigen the destination doesn't overlap with the operands, so it can write straight into `C` without an intermediate buffer.

</details>

<details>
<summary>Why does `-O3` matter so much?</summary>

At `-O0` the compiler does a literal translation. Every variable is on the stack, every load is from memory, no loop unrolling, no vectorisation. The hot inner loop runs at maybe 100 MFLOPS — slower than your phone in 2010.

At `-O2` and `-O3` the compiler unrolls loops, vectorises with SSE/AVX, registers values, and sometimes recognises matrix multiplication and emits much tighter code. A `1024×1024` matmul that takes 30 seconds at `-O0` can take under a second at `-O3`.

This is the same gap between your Python and NumPy — not because Python and `-O0` C++ are similar, but because both are paying *some* per-element overhead, and the optimised C++ pays *none*.

</details>

→ Continue with [08 — C++ vs Eigen](08-cpp-vs-eigen.md)
