# 7 — Three matmuls in C++

**Goal**: build matrix multiplication in C++ from the *same* primitives as in chapter 3 — `operator+`, scalar `operator*`, `matvec`, and three views of matmul. Then build at three optimisation levels and watch the same algorithm run at very different speeds depending on (a) which view you wrote and (b) which `-O` flag you compiled with.

> **This is the same design as chapter 3.** The methods are the same, only the syntax changes. If chapter 3 is fresh in your head, the algebra is done — this chapter is mostly typing. The two C++-specific lessons (*cache locality* and *compiler optimisation*) sit on top of an unchanged build-up.

## How we'll build it

Six TODOs, in dependency order. Each one uses what came before. Same tree as chapter 3 — the labels are just C++:

```text
   TODO 7.1  Matrix::operator+              vector-space operation 1
   TODO 7.2  Matrix::operator* (scalar)     vector-space operation 2
                       │
                       ▼
   TODO 7.3  matvec                         linear combination of A's columns
                       │
                       ▼
   TODO 7.4  matmul_columnwise              view 2  (decomposed via matvec)

   TODO 7.5  matmul_entrywise               view 1  (brute-force triple loop)
   TODO 7.6  matmul_outerproduct            view 3  (rank-1 accumulation)
```

`matmul_eigen` is **provided** — it's the C++ analogue of `matmul_numpy` (the reference we benchmark against, not a TODO).

## What you need to know that wasn't in chapter 3

In Python, the three views differed only in *structure*. Their wall-clock times differed by maybe 20% — Python's per-element interpreter overhead drowned out everything else. In C++ that overhead vanishes, and a different effect dominates: **cache locality**. Same algorithm, different memory-access pattern, *very* different runtime.

This isn't a separate algorithm. It's the same flop count. It's about how the data flows through the cache hierarchy.

### Why `matmul_entrywise` walks memory badly

The natural triple loop for `C(i, j) = sum_k A(i, k) * B(k, j)` is `(i, j, k)` — i and j are the indices of `C`, and `k` is the inner sum. The innermost loop touches `B(k, j)`: as `k` increments by one, the address of `B(k, j)` jumps forward by `cols(B)` doubles. On a typical CPU that's 8 doubles per cache line, so every other or every fourth iteration triggers a cache miss. Memory bandwidth, not flop throughput, becomes the bottleneck.

### Why `matmul_outerproduct` walks memory well

If you write `C = sum_k column_k(A) ⊗ row_k(B)` and accumulate the rank-1 contributions one `k` at a time, the natural loop order is `(k, i, j)` — k outer, i in the middle, j innermost. The innermost loop touches `B(k, j)` and `C(i, j)` for varying `j` and *fixed* `k, i`. Both walk forward through memory, one cache line at a time. The CPU prefetcher loves this. Same flop count, typically 5–10× faster at `-O3` for medium `n`.

This is why we keep both `matmul_entrywise` and `matmul_outerproduct` as separate methods even though they compute the same thing. The algebraic decomposition you choose dictates the natural loop order, which dictates the memory-access pattern, which dictates the wall-clock time. *That* is the C++ chapter's main lesson.

### Where does `matmul_columnwise` sit?

It's the slowest of the three on most C++ implementations — but for a different reason than `matmul_entrywise`. Each `matvec` call inside `matmul_columnwise` allocates fresh column matrices (one per scalar-mul, one per add) and frees them at the end of the iteration. Even at `-O3`, the malloc/free traffic is a real cost. The decomposition is structurally beautiful but allocation-heavy. Notice this in your benchmark numbers — it's the C++ counterpart of the same observation you made in Python.

## What to fill in

In [`cpp/include/matrix.hpp`](../cpp/include/matrix.hpp), in this order:

1. **`Matrix::operator+`** — TODO 7.1 — elementwise matrix addition (same shape required). The first vector-space operation.
2. **`Matrix::operator*`** — TODO 7.2 — scalar multiplication (`A * 2.0`). The free function below the class gives you `2.0 * A` for free by calling this. Note: there is *no* `Matrix * Matrix` operator; we use named methods so it's always clear which view you're getting.
3. **`matvec`** — TODO 7.3 — matrix-vector product as a linear combination of the columns of `*this`. Built from `operator+` and `operator*` only — no nested for loops in your implementation.
4. **`matmul_columnwise`** — TODO 7.4 — view 2 of matmul. The j-th column of the output is `matvec(other.get_column(j))`; assemble those columns with the provided `Matrix::from_columns` factory. No nested for loops here either.
5. **`matmul_entrywise`** — TODO 7.5 — view 1, the textbook triple loop. *Deliberately* nested loops — that's the brute-force baseline.
6. **`matmul_outerproduct`** — TODO 7.6 — view 3. Accumulate rank-1 contributions in place into a single output. Choose the loop order that walks contiguous memory in the innermost loop (the "Memory note" in the header tells you which one).

The three `matmul_*` methods may not call each other (each implements its own view). But `matmul_columnwise` *must* call `matvec`, and `matvec` *must* call `operator+` and `operator*` — that's the whole point of decomposing them.

`std::vector::push_back` has the same trap as Python's `list.append`: amortised growth, occasional reallocation. The **preallocate, don't grow** rule from chapter 1 carries over one-for-one — every output Matrix is constructed at full size with `Matrix(rows, cols)` (or `Matrix::zeros(rows, cols)`) and filled in by index.

## Build at three optimisation levels

This is the second C++-specific lesson. The compiler has *enormous* leverage over your loop. Configure three separate build directories so you can compare:

```bash
cd cpp
cmake -B build-O0 -DCMAKE_BUILD_TYPE=Debug          && cmake --build build-O0 --parallel
cmake -B build-O2 -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build-O2 --parallel
cmake -B build-O3 -DCMAKE_BUILD_TYPE=Release        && cmake --build build-O3 --parallel
```

Then verify each one's tests pass:

```bash
ctest --test-dir build-O3 --output-on-failure
```

You'll see how each `-O` level changes the same code's behaviour in chapter 8.

## Verify

Run the matmul-tagged tests in any of the build directories:

```bash
ctest --test-dir build-O3 -R matmul --output-on-failure
```

You're done with chapter 7 when:

- All `[matmul]`-tagged tests pass.
- `the three hand-written variants agree on random matrices` — the experimental restatement of "different decompositions, same total work."
- `each hand-written variant agrees with Eigen on random matrices` — your variants match the reference.
- `matrix-vector via matmul (column vector)` — exercises `matvec` indirectly through `matmul_entrywise`.
- `cat-sat-on-it scores match the lecture` — the lecture-anchored sanity check.

## Hints

<details>
<summary>What does "linear combination of columns" mean for matvec?</summary>

If you write out

```text
       │ │       │           │
A v =  A₀ A₁ ... Aₖ₋₁    *   v
       │ │       │           │
                                =  v₀·A₀ + v₁·A₁ + ... + vₖ₋₁·Aₖ₋₁
```

each term `vₚ · Aₚ` is "scalar times column" — exactly `operator*` applied to the column matrix you get from `get_column(p)`. The full sum is repeated `operator+`. So `matvec` is one loop over `p`, accumulating scaled columns into a running total. No inner loop in *your* code — `operator+` and `operator*` each do an O(rows) walk, but you don't write that walk yourself in `matvec`.

</details>

<details>
<summary>How does matmul_columnwise end up without nested loops?</summary>

The math says `C(:, j) = A * B(:, j)` for each `j`. Translated literally:

1. Build a `std::vector<Matrix>` of output columns by looping `j` from 0 to `other.cols() - 1` and pushing back `matvec(other.get_column(j))`.
2. Hand the vector to `Matrix::from_columns(...)` — the provided factory stitches them into the result.

Single `for` loop in `matmul_columnwise` itself. The column-extraction work is in `slice` (chapter 6), the per-column compute is in `matvec` (TODO 7.3), and the stacking is in `from_columns` (provided).

</details>

<details>
<summary>The outer-product variant is not faster than entrywise. What did I do wrong?</summary>

Two common mistakes:

1. **You allocated each rank-1 contribution as its own Matrix** and added them in a chain. Each rank-1 matrix is `(rows × cols)` doubles — for `n = 256` that's 0.5 MB *per iteration*, and you do `k = 256` iterations. The allocator dominates. Fix: don't build rank-1 matrices at all. Walk the loops directly and accumulate into a single output:

   ```text
   for k in 0..K:
     for i in 0..M:
       for j in 0..N:
         C(i, j) += A(i, k) * B(k, j);
   ```

2. **You used the wrong loop order** — for example `(i, j, k)` with the rank-1 framing. The rank-1 view *naturally* gives you `(k, i, j)` if you accumulate in place: outer loop over the rank-1 terms, then walk the resulting matrix row by row. That's the cache-friendly order. Other orderings throw away the win.

The hoist `const double a_ik = (*this)(i, k);` outside the innermost loop is a standard trick — it tells the compiler (and a human reader) that this value doesn't change across `j`. Modern compilers usually do this for you, but writing it explicitly makes the intent clear.

</details>

<details>
<summary>Why does `-O3` matter so much?</summary>

At `-O0` the compiler does a literal translation. Every variable is on the stack, every load is from memory, no loop unrolling, no vectorisation. The hot inner loop runs at maybe 100 MFLOPS — slower than your phone in 2010.

At `-O2` and `-O3` the compiler unrolls loops, vectorises with SSE/AVX, registers values, and sometimes recognises matrix multiplication and emits much tighter code. A `1024×1024` matmul that takes 30 seconds at `-O0` can take under a second at `-O3`.

This is the same gap you measured between Python and NumPy — not because Python and `-O0` C++ are similar, but because both are paying *some* per-element overhead, and the optimised C++ pays *none*.

</details>

<details>
<summary>Why no `Matrix * Matrix` operator?</summary>

Eigen overloads `*` for matrix multiplication and that works there because Eigen has expression templates that fuse `A * B + C * D` into a single pass. We don't. In our class, having `*` mean *scalar* multiplication only — and naming the matmul methods explicitly — keeps the code honest about which view you're calling. `A.matmul_entrywise(B)` is harder to mis-read than `A * B`.

</details>

→ Continue with [08 — C++ vs Eigen](08-cpp-vs-eigen.md)
