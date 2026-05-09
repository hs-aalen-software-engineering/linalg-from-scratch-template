# 1 — Python Matrix class

**Goal**: implement the basic operations of [`python/matrix/matrix.py`](../python/matrix/matrix.py) so the existing tests pass — except the matmul tests, which come in chapter 3.

## The shape contract

A `Matrix` has a shape `(rows, cols)` and a flat row-major data array of length `rows * cols`. The element at `(i, j)` lives at `data[i * cols + j]`. This is the same layout NumPy uses by default and the same layout you will use in C++.

Vectors in this exercise are matrices: a column vector is `(n, 1)`, a row vector is `(1, n)`. There is no separate `Vector` class. Once `Matrix @ Matrix` works, matrix-vector, vector-matrix, dot product, and outer product all fall out of it for free.

## What to fill in

Open [`python/matrix/matrix.py`](../python/matrix/matrix.py). The methods marked with TODO need bodies. They are listed in the order to do them in:

1. `__getitem__` — read `data[i * cols + j]`
2. `__setitem__` — write `data[i * cols + j]`
3. `__eq__` — same shape, all entries equal up to `1e-9`
4. `__repr__` — any readable format with the numbers in row order
5. `transpose` — return a new Matrix of shape `(cols, rows)` with `out[j, i] = self[i, j]`
6. `slice` — return a new Matrix containing rows `[r0, r1)` and columns `[c0, c1)`

**Do not** modify the constructor or the helper class methods (`zeros`, `eye`, `from_rows`, `random`). They're already correct and the rest of the assignment depends on their exact behaviour.

## Verify

After each TODO, re-run:

```bash
cd python && uv run pytest tests/test_matrix.py::test_construction_zeros -v
```

Then keep narrowing in by test name until everything in chapter 1 (the `# Chapter 1` block in `tests/test_matrix.py`) is green:

- `test_construction_zeros`
- `test_setitem_then_getitem`
- `test_from_rows_shape_mismatch_raises` *(passes already)*
- `test_repr_round_trip_is_readable`
- `test_transpose_involution`
- `test_transpose_swaps_shape`
- `test_slice_returns_independent_data`

`test_construction_eye_is_identity_for_matmul` will still fail until chapter 3 (it depends on `__matmul__` which dispatches to `matmul_columnwise`).

## Hints (open if stuck)

<details markdown="1">
<summary>Index out of range — what's the right error to raise?</summary>

Python's convention is `IndexError` for bad indices. `ValueError` is for bad values (a 5-element list when you needed 6). Stick with the convention so test diagnostics look right.

</details>

<details markdown="1">
<summary>Why `__eq__` with a tolerance?</summary>

Floating-point arithmetic isn't associative. `(a + b) + c` and `a + (b + c)` can differ in the last bit. Once your three matmul variants agree on a small example, they may differ by `1e-15` on a random one — that's not a bug, that's IEEE-754. A tolerance of `1e-9` is loose enough to ignore the noise and tight enough to catch real bugs.

</details>

<details markdown="1">
<summary>`slice` — half-open or closed intervals?</summary>

Half-open: `[r0, r1) × [c0, c1)`. So `slice((0, 2), (1, 3))` of a `3×3` returns rows 0, 1 and columns 1, 2 — a `2×2` block. This matches Python list slicing and NumPy.

</details>

→ Continue with [02 — Python unit tests](02-python-tests.md)
