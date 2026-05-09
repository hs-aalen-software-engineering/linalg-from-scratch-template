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
from __future__ import annotations

import random as _random
from typing import Iterable


class Matrix:
    # ------------------------------------------------------------------
    # Construction (provided — do not change)
    # ------------------------------------------------------------------
    def __init__(self, rows: int, cols: int, data: Iterable[float] | None = None):
        if rows <= 0 or cols <= 0:
            raise ValueError(f"shape must be positive, got ({rows}, {cols})")
        self.rows: int = rows
        self.cols: int = cols
        if data is None:
            self.data: list[float] = [0.0] * (rows * cols)
        else:
            flat = [float(x) for x in data]
            if len(flat) != rows * cols:
                raise ValueError(
                    f"data length {len(flat)} does not match shape ({rows}, {cols}) = {rows * cols}"
                )
            self.data = flat

    @property
    def shape(self) -> tuple[int, int]:
        return (self.rows, self.cols)

    # ------------------------------------------------------------------
    # Helper constructors (provided)
    # ------------------------------------------------------------------
    @classmethod
    def zeros(cls, rows: int, cols: int) -> Matrix:
        return cls(rows, cols)

    @classmethod
    def eye(cls, n: int) -> Matrix:
        m = cls(n, n)
        for i in range(n):
            m.data[i * n + i] = 1.0
        return m

    @classmethod
    def from_rows(cls, rows: list[list[float]]) -> Matrix:
        if not rows:
            raise ValueError("from_rows: at least one row required")
        nrows = len(rows)
        ncols = len(rows[0])
        if any(len(r) != ncols for r in rows):
            raise ValueError("from_rows: all rows must have the same length")
        flat: list[float] = []
        for r in rows:
            flat.extend(float(x) for x in r)
        return cls(nrows, ncols, flat)

    @classmethod
    def random(cls, rows: int, cols: int, seed: int | None = None) -> Matrix:
        rng = _random.Random(seed)
        flat = [rng.uniform(-1.0, 1.0) for _ in range(rows * cols)]
        return cls(rows, cols, flat)

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
        # TODO 1.4: return a human-readable string.
        # Suggested format:
        #   Matrix(2, 3)
        #   [[1.0, 2.0, 3.0],
        #    [4.0, 5.0, 6.0]]
        # The exact format does not matter, but it should round-trip readably.
        raise NotImplementedError("TODO 1.4 — implement __repr__")

    # ------------------------------------------------------------------
    # Chapter 1 — transpose and slice (TODO)
    # ------------------------------------------------------------------
    def transpose(self) -> Matrix:
        # TODO 1.5: return a new Matrix whose entry (j, i) equals self[i, j].
        # Shape becomes (cols, rows). This must NOT mutate self.
        raise NotImplementedError("TODO 1.5 — implement transpose")

    def slice(self, row_range: tuple[int, int], col_range: tuple[int, int]) -> Matrix:
        # TODO 1.6: return a new Matrix containing rows [r0, r1) and cols [c0, c1).
        # Shape becomes (r1 - r0, c1 - c0). This must NOT alias self.data.
        # Hint: Python's slice semantics are half-open: [start, stop).
        raise NotImplementedError("TODO 1.6 — implement slice")

    # ------------------------------------------------------------------
    # Helpers (provided) — exposed because the matmul variants need them
    # ------------------------------------------------------------------
    def get_row(self, i: int) -> list[float]:
        """Return row i as a fresh Python list (not a view)."""
        return self.data[i * self.cols : (i + 1) * self.cols]

    def get_column(self, j: int) -> list[float]:
        """Return column j as a fresh Python list (not a view)."""
        return [self.data[i * self.cols + j] for i in range(self.rows)]

    @staticmethod
    def _check_matmul_compatible(a: Matrix, b: Matrix) -> None:
        if a.cols != b.rows:
            raise ValueError(
                f"matmul shape mismatch: ({a.rows}, {a.cols}) @ ({b.rows}, {b.cols})"
            )

    # ------------------------------------------------------------------
    # Chapter 3 — three matmul implementations (TODO)
    # ------------------------------------------------------------------
    def matmul_entrywise(self, other: Matrix) -> Matrix:
        """
        View 1 — entry-wise (textbook) formula.
            C[i, j] = sum_k A[i, k] * B[k, j]

        Three nested loops. The slowest variant. A useful baseline.
        """
        Matrix._check_matmul_compatible(self, other)
        # TODO 3.1: implement the triple loop. Allocate a result Matrix of
        # shape (self.rows, other.cols), fill C[i, j] entry by entry.
        raise NotImplementedError("TODO 3.1 — implement matmul_entrywise")

    def matmul_columnwise(self, other: Matrix) -> Matrix:
        """
        View 2 — column-by-column.
            C[:, j] = A @ B[:, j]   for each column j of B

        Each column of C is the matrix-vector product A applied to one column of B.
        Internally still O(n^3) operations, but the inner work is "matrix times
        vector", emphasising the linear-map-on-a-vector view.
        """
        Matrix._check_matmul_compatible(self, other)
        # TODO 3.2: for each column j in [0, other.cols):
        #   1. extract column j of other (use other.get_column(j) or build the
        #      (other.rows, 1) column matrix).
        #   2. compute the matrix-vector product A @ b_j (your own code, no
        #      cheating with matmul_entrywise — write the two-loop version).
        #   3. write the result vector into column j of the output.
        raise NotImplementedError("TODO 3.2 — implement matmul_columnwise")

    def matmul_outerproduct(self, other: Matrix) -> Matrix:
        """
        View 3 — sum of outer products.
            C = sum_k A[:, k] @ B[k, :]
        where each term is a rank-1 (rows x cols) matrix.

        Watch the memory: if you allocate every rank-k intermediate as its
        own (rows, cols) matrix and store them in a list, you blow up memory
        by a factor of K = self.cols. The right thing is to accumulate
        in place into a single C.
        """
        Matrix._check_matmul_compatible(self, other)
        # TODO 3.3: initialise an output Matrix of zeros with the right shape.
        #   For each k in [0, self.cols):
        #     col_k = self.get_column(k)             # vector of length rows
        #     row_k = other.get_row(k)               # vector of length cols
        #     for i in range(rows):
        #       for j in range(cols):
        #         C[i, j] += col_k[i] * row_k[j]
        # Accumulate in place. Do NOT collect rank-1 matrices in a list.
        raise NotImplementedError("TODO 3.3 — implement matmul_outerproduct")

    # ------------------------------------------------------------------
    # NumPy reference (provided) — used in Chapter 5 for the speedup chart
    # ------------------------------------------------------------------
    def matmul_numpy(self, other: Matrix) -> Matrix:
        import numpy as np

        Matrix._check_matmul_compatible(self, other)
        a = np.array(self.data, dtype=np.float64).reshape(self.rows, self.cols)
        b = np.array(other.data, dtype=np.float64).reshape(other.rows, other.cols)
        c = a @ b
        return Matrix(self.rows, other.cols, c.flatten().tolist())

    # ------------------------------------------------------------------
    # Default operator @ -> column-wise (you can change this if you like)
    # ------------------------------------------------------------------
    def __matmul__(self, other: Matrix) -> Matrix:
        return self.matmul_columnwise(other)
