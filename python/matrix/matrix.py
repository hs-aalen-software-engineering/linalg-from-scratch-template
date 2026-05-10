"""
Matrix class — flat row-major storage.

Vectors are matrices of shape (n, 1) (column) or (1, n) (row); there is no
separate Vector class. This matches the column-vector convention used in the
lecture: q = W x means q is a column vector.

Storage: a single flat list of length rows*cols, indexed as i*cols + j.
This mirrors C arrays and NumPy's default order. Accessing data[i*cols+j]
is the same memory pattern that makes the cache-locality story work
the same way in C++ later in the assignment.
"""

import random as _random
from collections.abc import Iterable
from typing import Self


class Matrix:
    # ------------------------------------------------------------------
    # Construction (provided — do not change)
    # ------------------------------------------------------------------
    def __init__(self, rows_data: list[list[float]]):
        """Construct a Matrix from a list of rows (NumPy-style).

            Matrix([[1, 2, 3],
                    [4, 5, 6]])     # shape (2, 3)

        The shape is deduced from the input. Use the factory classmethods
        below for shape-only construction (`zeros`, `eye`, `random`) or
        when you already have a flat row-major buffer (`from_flat`).
        """
        if not rows_data:
            raise ValueError("Matrix: at least one row required")
        nrows = len(rows_data)
        ncols = len(rows_data[0])
        if ncols == 0:
            raise ValueError("Matrix: rows must have at least one element")
        if any(len(r) != ncols for r in rows_data):
            raise ValueError("Matrix: all rows must have the same length")
        self.rows: int = nrows
        self.cols: int = ncols
        # Preallocate the flat buffer at the final size, then write by index.
        # (See chapter 1's "preallocate, don't grow" rule.)
        flat = [0.0] * (nrows * ncols)
        for i, row in enumerate(rows_data):
            for j, x in enumerate(row):
                flat[i * ncols + j] = float(x)
        self.data: list[float] = flat

    @property
    def shape(self) -> tuple[int, int]:
        return (self.rows, self.cols)

    # ------------------------------------------------------------------
    # Helper constructors (provided)
    # ------------------------------------------------------------------
    @classmethod
    def from_flat(cls, rows: int, cols: int, data: Iterable[float]) -> Self:
        """Construct a Matrix from a flat row-major buffer of length rows*cols.

        Used internally by zeros/eye/random and by code that already has its
        data laid out flat (e.g., the matmul implementations and the NumPy
        bridge). Bypasses the list-of-lists path in __init__.
        """
        if rows <= 0 or cols <= 0:
            raise ValueError(f"shape must be positive, got ({rows}, {cols})")
        flat = [float(x) for x in data]
        if len(flat) != rows * cols:
            raise ValueError(
                f"data length {len(flat)} does not match shape ({rows}, {cols}) = {rows * cols}"
            )
        m = cls.__new__(cls)
        m.rows = rows
        m.cols = cols
        m.data = flat
        return m

    @classmethod
    def zeros(cls, rows: int, cols: int) -> Self:
        return cls.from_flat(rows, cols, [0.0] * (rows * cols))

    @classmethod
    def eye(cls, n: int) -> Self:
        m = cls.zeros(n, n)
        for i in range(n):
            m.data[i * n + i] = 1.0
        return m

    @classmethod
    def from_rows(cls, rows: list[list[float]]) -> Self:
        """Alias for the primary constructor: `Matrix(rows)` does the same thing.

        Kept for readability — `Matrix.from_rows([[1, 2], [3, 4]])` is more
        self-documenting than `Matrix([[1, 2], [3, 4]])` in test code.
        """
        return cls(rows)

    @classmethod
    def from_columns(cls, columns: list[Self]) -> Self:
        """Stack a list of column matrices into one wide matrix.

        Each element of `columns` must be a (rows, 1) Matrix of the same height.
        The result is a (rows, len(columns)) Matrix whose j-th column is
        `columns[j]`. This is the inverse of `get_column`.

        Used by `matmul_columnwise` to assemble per-column matvec results
        into a single output without an explicit nested copy loop at the
        call site.
        """
        if not columns:
            raise ValueError("from_columns: at least one column required")
        rows = columns[0].rows
        n = len(columns)
        for j, col in enumerate(columns):
            if col.rows != rows or col.cols != 1:
                raise ValueError(
                    f"from_columns: column {j} has shape {col.shape}, expected ({rows}, 1)"
                )
        out = cls.zeros(rows, n)
        for j, col in enumerate(columns):
            for i in range(rows):
                out.data[i * n + j] = col.data[i]
        return out

    @classmethod
    def random(cls, rows: int, cols: int, seed: int | None = None) -> Self:
        rng = _random.Random(seed)
        flat = [rng.uniform(-1.0, 1.0) for _ in range(rows * cols)]
        return cls.from_flat(rows, cols, flat)

    # ------------------------------------------------------------------
    # Chapter 1 — element access (TODO)
    # ------------------------------------------------------------------
    def __getitem__(self, key: tuple[int, int]) -> float:
        # TODO 1.1: return self.data at flat index i*cols + j.
        # Raise IndexError if i or j is out of range.
        raise NotImplementedError("TODO 1.1 — implement __getitem__")

    def __setitem__(self, key: tuple[int, int], value: float) -> None:
        # TODO 1.2: assign self.data[i*cols + j] = value.
        # Raise IndexError if i or j is out of range.
        raise NotImplementedError("TODO 1.2 — implement __setitem__")

    # ------------------------------------------------------------------
    # Chapter 1 — equality and repr (TODO)
    # ------------------------------------------------------------------
    def __eq__(self, other: object) -> bool:
        # TODO 1.3: return True iff other is a Matrix with the same shape
        # and all entries equal up to a small tolerance (1e-9).
        # Hint: math.isclose or abs(a-b) < tol works fine.
        raise NotImplementedError("TODO 1.3 — implement __eq__")

    def __repr__(self) -> str:
        # TODO 1.4: return a human-readable string that round-trips back through
        # the constructor. Suggested format (matches NumPy / np.array style):
        #   Matrix([[1.0, 2.0, 3.0],
        #           [4.0, 5.0, 6.0]])
        # The exact format does not matter, but it should be parseable as a
        # Matrix(...) call so that copy-pasting a debug print into a test works.
        raise NotImplementedError("TODO 1.4 — implement __repr__")

    # ------------------------------------------------------------------
    # Chapter 1 — transpose and slice (TODO)
    # ------------------------------------------------------------------
    def transpose(self) -> Self:
        # TODO 1.5: return a new Matrix whose entry (j, i) equals self[i, j].
        # Shape becomes (cols, rows). This must NOT mutate self.
        # Rule: preallocate the result with the final shape; do not append.
        raise NotImplementedError("TODO 1.5 — implement transpose")

    def slice(self, row_range: tuple[int, int], col_range: tuple[int, int]) -> Self:
        # TODO 1.6: return a new Matrix containing rows [r0, r1) and cols [c0, c1).
        # Shape becomes (r1 - r0, c1 - c0). This must NOT alias self.data.
        # Hint: Python's slice semantics are half-open: [start, stop).
        # Rule: preallocate the result with the final shape; do not append.
        raise NotImplementedError("TODO 1.6 — implement slice")

    # ------------------------------------------------------------------
    # Helpers (provided) — composed on top of `slice`
    #
    # Per our shape contract, vectors ARE matrices: a row is a (1, cols)
    # Matrix, a column is a (rows, 1) Matrix. These helpers are sugar over
    # `slice` so the matmul variants in chapter 3 don't have to spell out
    # the half-open-interval form every time. They are not separate TODOs;
    # once you implement `slice` (TODO 1.6), both work for free.
    # ------------------------------------------------------------------
    def get_row(self, i: int) -> Self:
        """Return row i as a fresh (1, cols) Matrix (not a view)."""
        return self.slice((i, i + 1), (0, self.cols))

    def get_column(self, j: int) -> Self:
        """Return column j as a fresh (rows, 1) Matrix (not a view)."""
        return self.slice((0, self.rows), (j, j + 1))

    def _check_matmul_compatible(self, other: Self) -> None:
        if self.cols != other.rows:
            raise ValueError(
                f"matmul shape mismatch: ({self.rows}, {self.cols}) @ ({other.rows}, {other.cols})"
            )

    # ==================================================================
    # Chapter 3 — vector-space primitives, matvec, and three matmul views
    # ==================================================================
    # The chapter builds matmul up from primitives. The order matters; each
    # TODO uses what came before. Every method below is short — at most one
    # for-loop over a single dimension — except matmul_entrywise (the
    # explicit triple-loop baseline) and matmul_outerproduct (in-place
    # rank-1 accumulation), which are kept as the brute-force counterpoints.

    # ------------------------------------------------------------------
    # Vector-space operations on matrices
    # ------------------------------------------------------------------
    def __add__(self, other: Self) -> Self:
        """
        Matrix addition: C = A + B, requiring both operands to have the same shape.

        Linear-algebra context: matrices of a fixed shape form a *vector space*.
        Addition is the first of its two defining operations. Geometrically,
        if A and B are linear maps, A + B is the map that applies both to the
        same input and adds the outputs.
        """
        # TODO 3.1: return a new Matrix with the same shape as self/other,
        # whose entries are the elementwise sum. Raise ValueError on shape
        # mismatch.
        raise NotImplementedError("TODO 3.1 — implement Matrix.__add__")

    def __mul__(self, scalar: float) -> Self:
        """
        Scalar multiplication: scale every entry of the matrix by `scalar`.

        Linear-algebra context: the second vector-space operation. Together
        with addition, scalar multiplication makes (rows, cols)-matrices a
        real vector space. Geometrically, `c * A` is the linear map A whose
        outputs are uniformly scaled by c.

        Note: this is the elementwise scaling `c * A_ij`, *not* the matmul
        operator @. We expose it as `*` because that's the NumPy /
        textbook convention.
        """
        # TODO 3.2: return a new Matrix with the same shape, where each
        # entry is multiplied by `scalar`. (See __rmul__ below — we get
        # the right-hand form `2.0 * A` for free by calling this.)
        raise NotImplementedError("TODO 3.2 — implement Matrix.__mul__")

    def __rmul__(self, scalar: float) -> Self:
        """
        Right-hand scalar multiplication so that `2.0 * A` works the same
        as `A * 2.0`. Provided — delegates to __mul__.
        """
        return self * scalar

    # ------------------------------------------------------------------
    # Matrix-vector product — the primitive matmul is built on
    # ------------------------------------------------------------------
    def matvec(self, vec: Self) -> Self:
        """
        Matrix-vector product: A @ v, where v is a (cols, 1) column matrix
        and the result is a (rows, 1) column matrix.

        Linear-algebra context: this implementation makes the *column-space*
        view of matrix-vector multiplication explicit. The product A @ v is
        a linear combination of the columns of A, with the entries of v as
        coefficients:

            A @ v = v[0] * A[:, 0]
                  + v[1] * A[:, 1]
                  + ...
                  + v[k-1] * A[:, k-1]

        Read it as: "the i-th entry of v says how much of the i-th column of
        A to take; the output is the sum of those scaled columns." This is
        why a matrix's column space *is* the set of all possible outputs of
        A @ v as v varies.
        """
        if vec.rows != self.cols or vec.cols != 1:
            raise ValueError(
                f"matvec shape mismatch: {self.shape} @ {vec.shape} "
                f"(expected vec to be ({self.cols}, 1))"
            )
        # TODO 3.3: implement matvec as the linear combination of self's
        # columns described in the docstring. The two operations you need
        # are matrix addition (TODO 3.1) and scalar multiplication
        # (TODO 3.2), applied to columns of self. There should be no
        # nested for loops in your implementation — that's the structural
        # point of building this on top of the vector-space primitives.
        raise NotImplementedError("TODO 3.3 — implement matvec")

    # ------------------------------------------------------------------
    # Three views of matrix-matrix multiplication
    # ------------------------------------------------------------------
    def matmul_columnwise(self, other: Self) -> Self:
        """
        Matrix-matrix product — view 2 of three: column-by-column.

        Linear-algebra context: matrix-matrix multiplication is matrix-vector
        multiplication applied independently to each column of the right
        operand, with the results stacked as columns of the output:

            C[:, j] = A @ B[:, j]   for each column j of B.

        Read it as: "B is a stack of input vectors; multiplying by A applies
        the same linear map to each one." Same flop count as the other two
        views, but the *structure* of the code mirrors this story — the
        outer loop walks columns, the per-column work is a single matvec.
        """
        self._check_matmul_compatible(other)
        # TODO 3.4: the j-th column of the output is self.matvec applied
        # to the j-th column of `other`. Build a list of those output
        # columns and assemble them into the result with the provided
        # `Matrix.from_columns` factory. There should be no explicit
        # nested for loops at this level — the algorithmic work moved
        # into matvec, the assembly moved into from_columns.
        raise NotImplementedError("TODO 3.4 — implement matmul_columnwise")

    def matmul_entrywise(self, other: Self) -> Self:
        """
        Matrix-matrix product — view 1 of three: entry-wise textbook formula.

            C[i, j] = sum_k A[i, k] * B[k, j]

        Linear-algebra context: the most elementary view — each output entry
        is a dot product of one row of A with one column of B. Three explicit
        nested loops, no decomposition into primitives. We keep this method
        deliberately as the *brute-force baseline*: a comparison point for
        benchmarks (chapter 4) and a reminder of what the math literally
        says before any abstraction is applied.
        """
        self._check_matmul_compatible(other)
        # TODO 3.5: implement the explicit triple loop directly per the
        # formula above. Unlike the columnwise variant, this method is
        # supposed to have nested loops — that's its whole point.
        raise NotImplementedError("TODO 3.5 — implement matmul_entrywise")

    def matmul_outerproduct(self, other: Self) -> Self:
        """
        Matrix-matrix product — view 3 of three: sum of outer products.

            C = sum_k A[:, k] @ B[k, :]

        where each term is a rank-1 (rows x cols) matrix obtained from
        column k of A and row k of B.

        Linear-algebra context: a different decomposition of the same total
        work. The sum has rank up to k. Outer-product accumulation is how
        you'd describe the operation if you were thinking of A and B as
        sums of rank-1 pieces — useful in low-rank approximation, attention
        mechanisms (Q K^T), and SVD.

        Memory note: do *not* allocate every rank-k intermediate as its own
        Matrix and store them in a list — that uses k times more memory
        than necessary. Accumulate in place into a single output.
        """
        self._check_matmul_compatible(other)
        # TODO 3.6: implement the sum-of-outer-products view per the
        # docstring formula. The outer product of a (rows, 1) column
        # vector with a (1, cols) row vector is the matrix whose entry
        # at (i, j) is `col[i, 0] * row[0, j]`. Accumulate the rank-1
        # contributions in place into a single output Matrix — do not
        # build a list of rank-1 matrices and sum them at the end.
        raise NotImplementedError("TODO 3.6 — implement matmul_outerproduct")

    # ------------------------------------------------------------------
    # NumPy reference (provided) — used in Chapter 5 for the speedup chart
    # ------------------------------------------------------------------
    def matmul_numpy(self, other: Self) -> Self:
        import numpy as np

        self._check_matmul_compatible(other)
        a = np.array(self.data, dtype=np.float64).reshape(self.rows, self.cols)
        b = np.array(other.data, dtype=np.float64).reshape(other.rows, other.cols)
        c = a @ b
        return Matrix.from_flat(self.rows, other.cols, c.flatten().tolist())

    # ------------------------------------------------------------------
    # Default operator @ -> column-wise (you can change this if you like)
    # ------------------------------------------------------------------
    def __matmul__(self, other: Self) -> Self:
        return self.matmul_columnwise(other)
