# 3 — Three matmul implementations

**Goal**: build matrix multiplication from primitives. By the end you'll have implemented two vector-space operations (`+`, scalar `*`), the matrix-vector product, and three different views of matrix-matrix multiplication — and you will have seen how the decomposed view (`matvec` → `matmul_columnwise`) lets the high-level methods read like the math, with no explicit nested for loops.

The linear-algebra companion proves (Exercise 6.4) that the three matmul views compute the same thing. You will turn that proof into running code.

## How we'll build it

The chapter has six TODOs. They're in dependency order: each one uses what came before. The progression is the lesson — you start from atoms (matrix addition, scalar multiplication) and end with three different decompositions of matmul, each of which says something different about what matrix multiplication *is*.

```text
   TODO 3.1  Matrix.__add__               vector-space operation 1
   TODO 3.2  Matrix.__mul__ (scalar)      vector-space operation 2
                       │
                       ▼
   TODO 3.3  matvec                       linear combination of A's columns
                       │
                       ▼
   TODO 3.4  matmul_columnwise            view 2  (decomposed via matvec)

   TODO 3.5  matmul_entrywise             view 1  (brute-force triple loop)
   TODO 3.6  matmul_outerproduct          view 3  (rank-1 accumulation)
```

Views 1 and 3 are kept deliberately as the *brute-force counterpoints*: explicit nested loops, no decomposition. They're the reference points you compare the decomposed view against — both for correctness (all three must agree on the same input) and for the chapter-4 benchmarks (same flop count, possibly different runtime).

## The three views

Let $A \in \mathbb{R}^{m \times k}$ and $B \in \mathbb{R}^{k \times n}$. The product $C = AB \in \mathbb{R}^{m \times n}$ can be computed in three ways:

**View 1 — entry-wise (textbook)**

$$C_{ij} = \sum_{p=1}^{k} A_{ip} B_{pj}$$

A triple loop. Each output entry is a dot product of one row of $A$ with one column of $B$. The most direct translation of the formula into code, and the one you implement *last* (TODO 3.5) as the brute-force baseline.

**View 2 — column-by-column** (the one we decompose)

$$C_{:,j} = A \cdot B_{:,j} \qquad \text{for each } j = 1, \ldots, n$$

Each column of $C$ is the matrix-vector product of $A$ with one column of $B$. The math says: *"matrix-times-matrix" is "matrix-times-column-vector" applied once per column*. So in code we write that decomposition literally — `matvec` is the primitive (TODO 3.3), `matmul_columnwise` is the loop on top of it (TODO 3.4). Internally still $O(mkn)$, but the *structure* of the code mirrors the *structure* of the math.

And `matvec` itself decomposes one level further: matrix-vector is a *linear combination of the columns of A*, with the entries of the input vector as coefficients. That's why TODO 3.3 only needs `__add__` (TODO 3.1) and scalar `__mul__` (TODO 3.2). No nested for loops anywhere on the path from primitives → matmul_columnwise — that's the structural point.

**View 3 — sum of outer products**

$$C = \sum_{p=1}^{k} A_{:,p} \, B_{p,:}^{\top}$$

Each term is a rank-1 $m \times n$ matrix obtained from one column of $A$ and one row of $B$. The sum has rank up to $k$. Same total work, very different decomposition — and the decomposition pattern that shows up in low-rank approximations, attention mechanisms, and SVD.

## What to fill in

In [`python/matrix/matrix.py`](../python/matrix/matrix.py), in this order:

1. **`Matrix.__add__`** — TODO 3.1 — matrix addition (same shape required). The first vector-space operation. Once this is in place, `A + B` returns a new Matrix.
2. **`Matrix.__mul__`** — TODO 3.2 — scalar multiplication, the second vector-space operation. `A * 2.0` and `2.0 * A` (the right-hand form is provided as `__rmul__`) both work after this.
3. **`matvec`** — TODO 3.3 — matrix-vector product as a linear combination of the columns of `self`, with the entries of `vec` as coefficients. Built from `__add__` and `__mul__` only — no explicit nested for loops.
4. **`matmul_columnwise`** — TODO 3.4 — view 2 of matmul. The j-th column of the output is `self.matvec(other.get_column(j))`; assemble those columns into the result with the provided `Matrix.from_columns` factory. No explicit nested for loops here either.
5. **`matmul_entrywise`** — TODO 3.5 — view 1 of matmul, the textbook triple loop. *Deliberately* nested loops — that's the brute-force baseline.
6. **`matmul_outerproduct`** — TODO 3.6 — view 3 of matmul. Accumulate rank-1 contributions in place into a single output. Also has nested loops, but for a different reason: the *structure* is a sum, the inner loops are the rank-1 outer product itself.

The three `matmul_*` methods may not call each other (each implements its own view from scratch). But `matmul_columnwise` *must* call `matvec`, and `matvec` *must* call `__add__` and `__mul__` — that's the whole point of decomposing them.

Remember the **preallocate, don't grow** rule from [chapter 1](01-python-matrix-class.md#rule-preallocate-dont-grow): every output is constructed with `Matrix.zeros(...)` (or its `Matrix.from_flat(...)` form) and filled in by index. Do not build the data buffer with `append`, `extend`, or `+=` on lists. The C++ side will inherit this rule one-for-one (`std::vector::push_back` has identical semantics).

## Verify

```bash
cd python && uv run pytest tests/test_matrix.py -v -k matmul
```

You're done when:

- `test_each_variant_matches_known_product` passes for all three matmul methods.
- `test_three_variants_agree_random` passes — the experimental restatement of Exercise 6.4.
- `test_each_variant_agrees_with_numpy` passes — your variants match `numpy.matmul`.
- `test_associativity_with_random_matrices` and `test_matrix_vector_via_matmul` pass.
- `test_softmax_dotproduct_reference_weights` passes — the cat-sat-on-it scores match the lecture.

`test_matrix_vector_via_matmul` exercises `matvec` indirectly through `__matmul__` → `matmul_columnwise` → `matvec`, so it acts as a free correctness test for TODO 3.3.

## Hints

<details markdown="1">
<summary>What does it mean that "matrix-vector is a linear combination of columns"?</summary>

If you write out

$$
A v
\;=\;
\begin{bmatrix}
\mid & \mid & & \mid \\
A_{:,0} & A_{:,1} & \cdots & A_{:,k-1} \\
\mid & \mid & & \mid
\end{bmatrix}
\begin{bmatrix} v_0 \\ v_1 \\ \vdots \\ v_{k-1} \end{bmatrix}
\;=\;
v_0 \, A_{:,0} \;+\; v_1 \, A_{:,1} \;+\; \cdots \;+\; v_{k-1} \, A_{:,k-1}
$$

each term $v_p \, A_{:,p}$ is "scalar times column" — exactly `Matrix.__mul__` applied to the column matrix you get from `self.get_column(p)`. The full sum is repeated `Matrix.__add__`. So `matvec` is one loop over $p$, accumulating scaled columns into a running total. No inner loop — `__add__` and `__mul__` already each do an O(rows) walk, but you don't write that walk yourself in `matvec`.

This is also why a matrix's *column space* is precisely the set of all possible outputs of $A v$: every output is a linear combination of $A$'s columns.

</details>

<details markdown="1">
<summary>How does `matmul_columnwise` end up without nested for loops?</summary>

The math says $C_{:,j} = A \cdot B_{:,j}$ for each $j$. Translated literally:

1. For each output column index $j$, compute one column matrix: `self.matvec(other.get_column(j))`.
2. Stack those column matrices into the final output.

The first step is a single `for j in range(other.cols)` (or a list comprehension). The second step is one call to the provided `Matrix.from_columns(...)` factory, which takes a list of `(rows, 1)` matrices and assembles them. No inner loop appears in `matmul_columnwise` itself — the column-extraction work is in `slice` (chapter 1), the per-column compute is in `matvec` (TODO 3.3), and the stacking is in `from_columns` (provided).

That's the value of decomposing the operation into primitives: each layer is short, each layer mirrors a piece of the math, and the loop discipline lives in one place.

</details>

<details markdown="1">
<summary>The outer-product variant blew up my memory. What went wrong?</summary>

You probably built each rank-1 contribution as its own `Matrix` and stored them in a list (or used `out = out + outer(...)` per iteration). Both patterns allocate $k$ matrices' worth of intermediates that you don't need. Accumulate the rank-1 contribution *in place* into the same output Matrix on each iteration:

```text
out = Matrix.zeros(rows, cols)
for k in range(self.cols):
    col_k = self.get_column(k)        # (rows, 1) Matrix
    row_k = other.get_row(k)          # (1, cols) Matrix
    for i in range(rows):
        for j in range(cols):
            out[i, j] += col_k[i, 0] * row_k[0, j]
return out
```

This is the only memory-sensible version. You'll be glad you wrote it this way when you measure peak memory in chapter 4.

</details>

<details markdown="1">
<summary>Why is `matvec`'s "vector" a (cols, 1) Matrix and not a flat Python list?</summary>

Because vectors *are* matrices in our shape contract — a column vector is a `(rows, 1)` Matrix. Using a Matrix as the input keeps the data model consistent across the whole class: there's no point at which a row or column "becomes" a list and then "becomes" a matrix again. Mixing intermediate representations is a real code-smell — it forces every method to convert at the boundary, which is both noisy and a likely source of bugs.

A NumPy-style 1-D array (`shape=(n,)`) is a different design choice that NumPy can afford because it has separate types for 1-D and 2-D arrays. We don't, on purpose: it keeps the class small and uniform.

</details>

<details markdown="1">
<summary>Why doesn't column-wise read column-major?</summary>

It does, kind of — but only the column extraction does. Each call to `matvec` still walks the matrix's row-major data; the "column" is just the input. The point of view 2 isn't speed in pure Python; it's that you can describe matmul as "apply this linear map to a stack of input vectors." This view is also how multi-head attention is usually explained in the lecture.

</details>

→ Continue with [04 — Benchmarking Python](04-python-benchmarking.md)
