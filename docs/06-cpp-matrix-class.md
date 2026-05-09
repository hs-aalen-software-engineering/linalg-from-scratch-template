# 6 — C++ Matrix class

**Goal**: implement the basic operations of [`cpp/include/matrix.hpp`](../cpp/include/matrix.hpp). Same shape contract as the Python version; same TODOs. By the end the C++ tests build and the chapter-6 tests pass.

If C++ is new to you, that's expected. Use the assistant for syntax. The TODO bodies are 3–10 lines each.

## The shape contract (same as Python)

A `Matrix` has `rows()`, `cols()`, and a flat row-major `std::vector<double>` data buffer. Element `(i, j)` lives at `data[i * cols + j]`. Vectors are matrices of shape `(n, 1)` or `(1, n)`.

## Why `std::vector<double>` and not a 2-D structure

Same reason NumPy is row-major: a single contiguous block is cache-friendly. The CPU's prefetcher is happy walking forward through memory; it is not happy chasing pointers between rows of a `std::vector<std::vector<double>>`. Choosing the storage layout is the most important design decision for matmul performance.

## What to fill in

Open [`cpp/include/matrix.hpp`](../cpp/include/matrix.hpp). The TODO blocks for chapter 6:

1. `operator()(i, j) const` and `operator()(i, j)` — TODO 6.1 — read/write the flat index
2. `approx_equal(other, tol)` — TODO 6.2 — same shape, all entries within `tol`
3. `transpose()` — TODO 6.3 — new Matrix of shape `(cols, rows)` with `result(j, i) = (*this)(i, j)`
4. `slice(r0, r1, c0, c1)` — TODO 6.4 — half-open intervals, copy the sub-block

The provided constructors, helper static functions (`zeros`, `eye`, `from_rows`, `random`), and the `check_matmul_compatible` helper are correct — don't change them.

## Build and run the tests

From `cpp/`:

```bash
cmake -B build -S .              # configure (writes build/)
cmake --build build              # compile (parallel by default in modern CMake)
ctest --test-dir build --output-on-failure
```

You're done with chapter 6 when the basic tests (tagged `[basic]` in [`cpp/tests/test_matrix.cpp`](../cpp/tests/test_matrix.cpp)) pass. Filter with:

```bash
ctest --test-dir build -R basic --output-on-failure
```

The matmul tests (tagged `[matmul]`) will keep failing — that's chapter 7.

## Hints

<details>
<summary>I'm getting compile errors I can't decipher.</summary>

Three things help:

1. Read the *first* error message; subsequent ones are usually cascades from the first.
2. Paste the error into the assistant — it has seen this dance before.
3. Search for the exact text on cppreference.com or Stack Overflow. Modern errors usually have a one-line fix.

The most common errors students hit:
- *"no matching function for call to..."* — you forgot a `const` on a member function or a parameter type doesn't match.
- *"undefined reference to..."* — you declared a method but didn't define it. Since this header is header-only, "didn't define" means you wrote a function declaration but no body inside the class.
- *"cannot bind non-const lvalue reference..."* — you tried to assign to something returned `const`. Add the non-const overload.

</details>

<details>
<summary>Why two `operator()` overloads — one const, one mutable?</summary>

`const Matrix& m` callers must be able to read `m(i, j)` without modifying. Non-const callers want to write `m(i, j) = 7.5`. The const overload returns by value (`double`), the non-const returns by reference (`double&`). Both compute the same flat index; the *return type* is what differs.

</details>

<details>
<summary>How do I test approx_equal with non-trivial floating-point?</summary>

The provided tests use exact equality where the expected values are integers (so floating-point representation is exact) and tolerance-based equality elsewhere. Implement with:

```cpp
if (rows_ != other.rows_ || cols_ != other.cols_) return false;
for (std::size_t k = 0; k < data_.size(); ++k) {
  if (std::abs(data_[k] - other.data_[k]) > tol) return false;
}
return true;
```

`#include <cmath>` is already in the header.

</details>

→ Continue with [07 — Three matmuls in C++](07-cpp-three-matmuls.md)
