# 6 — C++ Matrix class

**Goal**: implement the basic operations of [`cpp/src/matrix.cpp`](../cpp/src/matrix.cpp). Same shape contract as the Python version; same TODOs. By the end the C++ tests build and the chapter-6 tests pass.

> **Before you start.** If you haven't already, read [5b — Building and testing C++ projects: a primer](05b-cpp-build-system.md). It explains the `cmake` / `ninja` / `ctest` commands you'll be running below — what each one does, why we need them, and how to read their output. The chapter assumes you've seen the primer; if you skip it, the commands will work but you won't know *why*.

If C++ is new to you, that's expected. Use the assistant for syntax. The TODO bodies are 3–10 lines each.

## The shape contract (same as Python)

A `Matrix` has `rows()`, `cols()`, and a flat row-major `std::vector<double>` data buffer. Element `(i, j)` lives at `data[i * cols + j]`. Vectors are matrices of shape `(n, 1)` or `(1, n)`.

## Why `std::vector<double>` and not a 2-D structure

Same reason NumPy is row-major: a single contiguous block is cache-friendly. The CPU's prefetcher is happy walking forward through memory; it is not happy chasing pointers between rows of a `std::vector<std::vector<double>>`. Choosing the storage layout is the most important design decision for matmul performance.

The class exposes this type as a public alias `Matrix::Data` so callers can write the meaningful name instead of repeating `std::vector<double>` everywhere:

```cpp
class Matrix {
public:
  using Data = std::vector<double>;
  // ...
  const Data& data() const noexcept;
};
```

This is the standard library's own convention — `std::vector::value_type`, `std::map::key_type` — promoted to a project-specific name. The compiler treats `Matrix::Data` and `std::vector<double>` as the same type; the alias is a readability lever, not a new abstraction. We avoid `using namespace std;` (which pollutes everything), and we keep `std::size_t` in the public header (it's idiomatic and expected by users). Inside `matrix.cpp` and `matrix_debug.cpp`, a file-scope `using std::size_t;` lets the implementation drop the `std::` prefix locally — that's safe because a `.cpp` file is not a header, so the using-declaration doesn't leak to anyone.

## Two files: header and source

Unlike Python, where one `.py` file holds both the class shape and its method bodies, C++ splits a class across two files:

```text
cpp/
├── include/matrix.hpp   ← declarations (the API a caller sees)
└── src/matrix.cpp       ← definitions (the bodies you implement)
```

[`cpp/include/matrix.hpp`](../cpp/include/matrix.hpp) lists *what* a `Matrix` is — its constructors, member functions, return types, signatures. Read it once and you have the API at a glance: ~90 lines, no implementation noise. [`cpp/src/matrix.cpp`](../cpp/src/matrix.cpp) is *how* each method works: the bodies, the validation, the algorithms. **Your TODOs all live in `matrix.cpp`** — that's where you'll spend chapter 6 and chapter 7.

This split exists for three reasons:

1. **Compile-time isolation.** When you change a method *body* in `matrix.cpp`, only that one file recompiles. If everything were in the header (the "header-only" pattern), every translation unit that includes `matrix.hpp` would re-parse and re-compile every body — much slower for non-trivial classes.
2. **Encapsulation of dependencies.** `matrix.cpp` includes `<Eigen/Dense>` because `matmul_eigen` needs it. The header doesn't. Tests, the benchmark, and any future consumer of `matrix.hpp` get a small, focused header without dragging in Eigen.
3. **A C++ idiom worth learning.** Almost every non-trivial C++ class is split this way. Reading and writing the split is a basic skill.

A few methods stay in the header anyway: trivial accessors (`rows()`, `cols()`, `data()`) and one-line delegates (`operator==`, `operator!=`, the free `2.0 * matrix` overload). These are short enough that the inlining benefit outweighs the extra recompilation cost. Anything longer goes in the .cpp.

## What to fill in

Open [`cpp/src/matrix.cpp`](../cpp/src/matrix.cpp). The TODO blocks for chapter 6:

1. `operator()(i, j) const` and `operator()(i, j)` — TODO 6.1 — read/write the flat index
2. `approx_equal(other, tol)` — TODO 6.2 — same shape, all entries within `tol`
3. `transpose()` — TODO 6.3 — new Matrix of shape `(cols, rows)` with `result(j, i) = (*this)(i, j)`
4. `slice(r0, r1, c0, c1)` — TODO 6.4 — half-open intervals, copy the sub-block

The provided constructors, helper static functions (`zeros`, `eye`, `from_rows`, `from_columns`, `random`), and the `check_matmul_compatible` helper are correct — don't change them. (They're at the top of `matrix.cpp`, before the TODO sections.)

## Try it before you touch a TODO

[`cpp/tests/test_experiments.cpp`](../cpp/tests/test_experiments.cpp) is a playground file modelled on Python's [`tests/test_experiments.py`](../python/tests/test_experiments.py). Every test in it passes from day one, because it exercises only the *provided* parts of the class — constructors, factory static methods, and the `rows()` / `cols()` / `data()` accessors — none of the chapter-6 TODOs. Use it as a starting point: read it, modify it, add your own tests with `std::cout` calls, drop in `dump_raw(m)` to see what the row-major buffer looks like.

The two helpers in [`cpp/include/matrix_debug.hpp`](../cpp/include/matrix_debug.hpp) mirror Python's `matrix/debug.py`:

- `dump_raw(m)` — returns a string like `Matrix(rows=2, cols=3, data=[1, 2, 3, 4, 5, 6])`. Useful before you implement `operator()` (TODO 6.1) — you can still see what's in memory.
- `memory_bytes(m)` — `sizeof(Matrix) + capacity * sizeof(double)`. Compare with the Python equivalent: a Python `float` is heap-allocated with ~28 bytes of overhead per element; a C++ `double` is inline at 8 bytes. That difference is one of the four reasons your C++ matmul will beat your Python matmul.

## Build and run the tests

From `cpp/`:

```bash
cmake -B build -S .              # configure (writes build/)
cmake --build build              # compile (parallel by default in modern CMake)
ctest --test-dir build --output-on-failure
```

The first time you run this, expect:

- All `[experiments]` tests pass (9 cases). The provided machinery is correct.
- All `[basic]` and `[matmul]` tests fail with `std::logic_error("TODO ...")`. That's expected — they're your work for chapters 6 and 7.

You're done with chapter 6 when the `[basic]` tests pass. The `[matmul]` tests will keep failing — that's chapter 7.

## Tags and filtering

Both test files compile into a single `test_matrix` executable. Every `TEST_CASE` is tagged with a Catch2 tag in square brackets:

| Tag             | What it covers                                | Count |
| --------------- | --------------------------------------------- | ----: |
| `[experiments]` | Provided machinery; passes on day one         |     9 |
| `[basic]`       | Chapter 6 TODOs (operator(), transpose, etc.) |     7 |
| `[matmul]`      | Chapter 7 TODOs (the matmul views)            |     8 |
| `[lecture]`     | The cat-sat-on-it reference                   |     1 |

Catch2 tags work via the executable directly. `ctest -R` filters by test *name* (the description after `TEST_CASE`), not by tag, so for tag filtering use the binary:

```bash
./build/test_matrix "[experiments]"          # only experiments — should be all green
./build/test_matrix "[basic]"                # only chapter 6
./build/test_matrix "[matmul]"               # only chapter 7
./build/test_matrix "[basic],[experiments]"  # union (both green sets while you work on ch6)
./build/test_matrix "~[matmul]"              # exclude matmul (everything except ch7)
```

Quote the brackets — most shells (PowerShell included) treat `[...]` as a glob otherwise. This is the same pattern as Python's `pytest -m ch1`, just with different syntax.

If you want ctest's nicer summary output and the brackets are giving you trouble, run the binary directly with `--reporter compact` instead of going through ctest.

## A typical workflow

The intended cycle while you're working through chapter 6:

1. **Pick the next TODO** in [`cpp/src/matrix.cpp`](../cpp/src/matrix.cpp) (start with 6.1 — `operator()`). Write the body.
2. **Rebuild:**

   ```bash
   cmake --build build
   ```

   Only the changed `.cpp` plus the test binary recompile (Ninja's incremental build). Expect under 5 seconds on most laptops.

3. **Run only the tests for the chapter you're on:**

   ```bash
   ./build/test_matrix "[basic]"
   ```

   You see exactly which `[basic]` tests pass and which still throw `TODO 6.X`. Iterate until they're all green. Then move on to the next TODO.

4. **Keep `[experiments]` green throughout.** If a refactor accidentally breaks `dump_raw` or another provided piece, the `[experiments]` tests will catch it. A quick `./build/test_matrix "[experiments],[basic]"` is the union you want during chapter 6 work.

## Adding your own tests

When something behaves unexpectedly, the cheapest debugging move is to write a test that pins down the unexpected behaviour. Append a new `TEST_CASE` to whichever file fits its purpose — both files compile into the same `test_matrix` executable, and `cmake --build build` picks up new tests automatically (no need to re-run `cmake -B build`).

- **Exploratory tests, things that should always pass:** add to [`tests/test_experiments.cpp`](../cpp/tests/test_experiments.cpp), tagged `[experiments]`.
- **Tests that pin down a specific TODO's expected behaviour:** add to [`tests/test_matrix.cpp`](../cpp/tests/test_matrix.cpp), tagged `[basic]` (chapter 6) or `[matmul]` (chapter 7).

```cpp
TEST_CASE("eye times v should pick out v's entries", "[experiments]") {
  auto I = Matrix::eye(3);
  auto v = Matrix::from_rows({{1}, {2}, {3}});
  // ... your assertions, using dump_raw / memory_bytes / data() as needed
}
```

The tag goes in the second string argument. Pick the tag that matches *when this test should pass*, not what feature it tests.

## Using the debug helpers

When a test fails and you need to see what's actually happening, the two helpers in [`matrix_debug.hpp`](../cpp/include/matrix_debug.hpp) are usable from anywhere — including inside a `TEST_CASE`, your own scratch `main()`, or a `std::cerr` line in a TODO body.

```cpp
#include <iostream>
#include "matrix_debug.hpp"

// Inside a test or method:
std::cerr << "before transpose: " << dump_raw(m) << '\n';
auto t = m.transpose();
std::cerr << "after transpose:  " << dump_raw(t) << '\n';
```

Catch2 captures `std::cerr` by default and only prints it when a test fails — so your debug lines stay quiet on green runs and surface automatically on red ones. That's usually faster than reaching for a debugger when you just want to know whether your column was extracted correctly.

`memory_bytes(m)` is mostly useful for the chapter-5 / chapter-8 comparison work (showing that a C++ Matrix uses ~8 bytes per element vs Python's ~28), but it's also a quick sanity check that `slice` isn't accidentally allocating something the size of the original matrix.

## Hints

<details>
<summary>I'm getting compile errors I can't decipher.</summary>

Three things help:

1. Read the *first* error message; subsequent ones are usually cascades from the first.
2. Paste the error into the assistant — it has seen this dance before.
3. Search for the exact text on cppreference.com or Stack Overflow. Modern errors usually have a one-line fix.

The most common errors students hit:

- *"no matching function for call to..."* — you forgot a `const` on a member function or a parameter type doesn't match.
- *"undefined reference to..."* — you declared a method in `matrix.hpp` but didn't define it in `matrix.cpp` (or your signature in the .cpp doesn't match the declaration — e.g., a missing `const` on the method or a different parameter type makes it a *different* function from the linker's point of view).
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
for (size_t k = 0; k < data_.size(); ++k) {
  if (std::abs(data_[k] - other.data_[k]) > tol) return false;
}
return true;
```

`size_t` (unqualified) and `std::abs` both work without extra includes — `<cmath>` is already pulled into `matrix.cpp`, and the file-scope `using std::size_t;` at the top of `matrix.cpp` lets you drop the `std::` prefix on the loop counter.

</details>

→ Continue with [07 — Three matmuls in C++](07-cpp-three-matmuls.md)
