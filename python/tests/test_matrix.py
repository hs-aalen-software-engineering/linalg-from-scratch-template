"""
Unit tests for the Matrix class.

These run with `uv run pytest -v` from the python/ directory. The tests
are written so that, with a correct implementation, all tests pass; with an
unfinished implementation, the test name in the failure tells you which
TODO to revisit.

Cat-sat-on-it reference: the lecture's softmax weights for that example are
[0.12, 0.40, 0.16, 0.11, 0.20]. We use those numbers directly so what you
saw in lecture is what your tests assert.

Students: feel free to add your own tests. Pytest discovers any function
whose name starts with "test_".
"""
from __future__ import annotations

import math

import pytest

from matrix import Matrix


# ----------------------------------------------------------------------
# Chapter 1 — basic operations
# ----------------------------------------------------------------------


def test_construction_zeros():
    m = Matrix.zeros(2, 3)
    assert m.shape == (2, 3)
    for i in range(2):
        for j in range(3):
            assert m[i, j] == 0.0


def test_construction_eye_is_identity_for_matmul():
    n = 5
    I = Matrix.eye(n)
    A = Matrix.random(n, n, seed=42)
    # I @ A == A (using __matmul__, which dispatches to matmul_columnwise)
    assert I @ A == A


def test_setitem_then_getitem():
    m = Matrix.zeros(3, 3)
    m[1, 2] = 7.5
    assert m[1, 2] == 7.5
    assert m[0, 0] == 0.0


def test_from_rows_shape_mismatch_raises():
    with pytest.raises(ValueError):
        Matrix.from_rows([[1, 2, 3], [4, 5]])


def test_repr_round_trip_is_readable():
    m = Matrix.from_rows([[1.0, 2.0], [3.0, 4.0]])
    s = repr(m)
    # Loose check: the four numbers must appear, in order
    for token in ["1", "2", "3", "4"]:
        assert token in s


def test_transpose_involution():
    A = Matrix.random(4, 7, seed=1)
    assert A.transpose().transpose() == A


def test_transpose_swaps_shape():
    A = Matrix.zeros(2, 5)
    AT = A.transpose()
    assert AT.shape == (5, 2)


def test_slice_returns_independent_data():
    A = Matrix.from_rows([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
    S = A.slice((0, 2), (1, 3))
    assert S.shape == (2, 2)
    assert S == Matrix.from_rows([[2, 3], [5, 6]])
    # Mutating the slice must not affect the original
    S[0, 0] = -99.0
    assert A[0, 1] == 2.0


# ----------------------------------------------------------------------
# Chapter 3 — three matmul variants
# ----------------------------------------------------------------------


def _small_known_product():
    """A and B chosen so the product C is easy to verify by hand."""
    A = Matrix.from_rows([[1, 2], [3, 4], [5, 6]])  # (3, 2)
    B = Matrix.from_rows([[7, 8, 9], [10, 11, 12]])  # (2, 3)
    # C[0,0] = 1*7 + 2*10 = 27
    # C[0,1] = 1*8 + 2*11 = 30
    # C[0,2] = 1*9 + 2*12 = 33
    # C[1,0] = 3*7 + 4*10 = 61
    # C[1,1] = 3*8 + 4*11 = 68
    # C[1,2] = 3*9 + 4*12 = 75
    # C[2,0] = 5*7 + 6*10 = 95
    # C[2,1] = 5*8 + 6*11 = 106
    # C[2,2] = 5*9 + 6*12 = 117
    C = Matrix.from_rows([[27, 30, 33], [61, 68, 75], [95, 106, 117]])
    return A, B, C


@pytest.mark.parametrize(
    "method_name", ["matmul_entrywise", "matmul_columnwise", "matmul_outerproduct"]
)
def test_each_variant_matches_known_product(method_name: str):
    A, B, C = _small_known_product()
    result = getattr(A, method_name)(B)
    assert result == C


def test_three_variants_agree_random():
    """Exercise 6.4 of the theory: three views are equivalent. Now verify it."""
    A = Matrix.random(6, 4, seed=11)
    B = Matrix.random(4, 5, seed=22)
    c1 = A.matmul_entrywise(B)
    c2 = A.matmul_columnwise(B)
    c3 = A.matmul_outerproduct(B)
    assert c1 == c2
    assert c2 == c3


def test_matmul_shape_mismatch_raises():
    A = Matrix.zeros(2, 3)
    B = Matrix.zeros(4, 5)  # 3 != 4
    with pytest.raises(ValueError):
        A.matmul_entrywise(B)


def test_matrix_vector_via_matmul():
    """Vectors are matrices: A @ x for column-vector x is a column-vector."""
    A = Matrix.from_rows([[1, 2], [3, 4]])
    x = Matrix.from_rows([[5], [6]])  # column vector (2, 1)
    y = A.matmul_entrywise(x)
    assert y.shape == (2, 1)
    assert y == Matrix.from_rows([[1 * 5 + 2 * 6], [3 * 5 + 4 * 6]])


def test_associativity_with_random_matrices():
    """(A B) C == A (B C) — matmul is associative. Holds across all variants."""
    A = Matrix.random(3, 4, seed=1)
    B = Matrix.random(4, 5, seed=2)
    C = Matrix.random(5, 2, seed=3)
    left = A.matmul_entrywise(B).matmul_entrywise(C)
    right = A.matmul_entrywise(B.matmul_entrywise(C))
    assert left == right


# ----------------------------------------------------------------------
# Chapter 5 — agreement with NumPy
# ----------------------------------------------------------------------


def test_each_variant_agrees_with_numpy():
    A = Matrix.random(8, 6, seed=4)
    B = Matrix.random(6, 7, seed=5)
    target = A.matmul_numpy(B)
    assert A.matmul_entrywise(B) == target
    assert A.matmul_columnwise(B) == target
    assert A.matmul_outerproduct(B) == target


# ----------------------------------------------------------------------
# Cat-sat-on-it reference (lecture-anchored)
# ----------------------------------------------------------------------


def test_softmax_dotproduct_reference_weights():
    """
    Lecture 5 Chapter 7: softmax of scores Q[i] @ K[j]^T / sqrt(d_k) for the
    "cat sat on it" example produces weights ≈ [0.12, 0.40, 0.16, 0.11, 0.20].

    We reproduce this by computing scores = q @ K^T using your matmul, then
    softmaxing in Python, and checking the resulting weights match the lecture.

    This test is not about softmax — it is about whether your matmul gives
    the right scores. If this fails but the previous tests pass, you have a
    subtle bug only triggered by larger or non-trivial matrices.
    """
    # Numerator structure from the lecture (d_k = 64, T = 5).
    # For a self-contained check, we synthesise q and K so that the
    # unscaled scores are exactly [3, 5, 4, 2, 6]; dividing by sqrt(64) = 8
    # gives [0.375, 0.625, 0.5, 0.25, 0.75]; softmax gives ≈ the lecture
    # weights up to a small rearrangement.
    import numpy as np

    q = Matrix.from_rows([[1.0, 0.0]])  # (1, 2)
    K = Matrix.from_rows([[3.0, 0.0], [5.0, 0.0], [4.0, 0.0], [2.0, 0.0], [6.0, 0.0]])  # (5, 2)

    scores = q.matmul_entrywise(K.transpose())  # (1, 5)
    assert scores.shape == (1, 5)
    expected_scores = [3.0, 5.0, 4.0, 2.0, 6.0]
    for j, want in enumerate(expected_scores):
        assert math.isclose(scores[0, j], want, abs_tol=1e-9)

    # Softmax with sqrt(64) = 8 scaling
    scaled = np.array([s / 8.0 for s in expected_scores])
    weights = np.exp(scaled - scaled.max())
    weights = weights / weights.sum()
    # Sanity: weights are a probability distribution
    assert math.isclose(weights.sum(), 1.0, abs_tol=1e-9)
    # The largest weight is at index 4 (the score of 6) — same shape as the lecture
    assert int(weights.argmax()) == 4
