"""
Experiments and starter tests for the Matrix class.

This file is your playground. Every test below passes from day one
because it exercises only the *provided* parts of the class (the
constructor, the factory classmethods, and the raw `.rows` / `.cols` /
`.data` attributes). None of it depends on any TODO. Use these as a
starting point: modify them, add your own, drop in `print()` calls,
and re-run with `uv run pytest tests/test_experiments.py -v -s` to
see what comes out (the `-s` flag shows print output that pytest
otherwise captures).

Why this file lives next to `test_matrix.py`: that file checks the
TODOs you're going to implement. This one is for *learning* — getting
a feel for the data model before you change anything. There is no
chapter marker because every test is intended to pass before chapter 1
TODOs are written.

Two helpers from `matrix.debug` (also re-exported as `matrix.dump_raw`
and `matrix.memory_bytes`) are used below:

    dump_raw(m)        -> a string showing rows, cols, and the raw
                          flat data buffer. Use this before __repr__
                          (TODO 1.4) is implemented, or any time you
                          want to see the underlying storage.
    memory_bytes(m)    -> a rough estimate of how many bytes the
                          Matrix uses, including each Python float's
                          object overhead.

Read `matrix/debug.py` once — it's short, and you'll understand both
helpers and how they reach into the Matrix class without going through
any TODO method.
"""

import sys

from matrix import Matrix, dump_raw, memory_bytes

# ----------------------------------------------------------------------
# Construction — verify the provided machinery works
# ----------------------------------------------------------------------


def test_primary_constructor_takes_list_of_lists():
    """Matrix([[...]]) is the NumPy-style primary form."""
    m = Matrix([[1, 2, 3], [4, 5, 6]])
    assert m.rows == 2
    assert m.cols == 3
    # The data is stored row by row (row-major). Try this yourself:
    #   print(dump_raw(m))
    assert m.data == [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]


def test_zeros_factory_makes_a_fresh_matrix_of_zeros():
    m = Matrix.zeros(3, 4)
    assert m.shape == (3, 4)
    assert len(m.data) == 12
    assert all(x == 0.0 for x in m.data)


def test_eye_places_ones_on_the_diagonal():
    m = Matrix.eye(3)
    assert m.shape == (3, 3)
    # 3x3 identity laid out row-major:
    assert m.data == [
        1.0,
        0.0,
        0.0,
        0.0,
        1.0,
        0.0,
        0.0,
        0.0,
        1.0,
    ]


def test_from_flat_round_trips_a_known_buffer():
    """from_flat takes the same row-major layout you'd see in C / NumPy."""
    flat = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]
    m = Matrix.from_flat(2, 3, flat)
    assert m.shape == (2, 3)
    assert m.data == flat


def test_from_rows_is_an_alias_for_the_primary_constructor():
    a = Matrix([[1, 2], [3, 4]])
    b = Matrix.from_rows([[1, 2], [3, 4]])
    assert a.data == b.data
    assert a.shape == b.shape


def test_random_is_reproducible_with_a_seed():
    """Same seed -> same data. Different seed -> different data."""
    a = Matrix.random(2, 2, seed=42)
    b = Matrix.random(2, 2, seed=42)
    c = Matrix.random(2, 2, seed=43)
    assert a.data == b.data
    assert a.data != c.data


# ----------------------------------------------------------------------
# Inspection — see the raw storage without going through __repr__
# ----------------------------------------------------------------------


def test_dump_raw_shows_shape_and_flat_data():
    m = Matrix([[1, 2], [3, 4]])
    s = dump_raw(m)
    # The shape numbers and the flat data should both appear in the string.
    assert "rows=2" in s
    assert "cols=2" in s
    assert "1.0" in s
    assert "4.0" in s


# ----------------------------------------------------------------------
# Memory — why Python floats are expensive (preview of chapter 5)
# ----------------------------------------------------------------------


def test_a_single_python_float_is_at_least_24_bytes():
    """Every Python float is a heap object: type pointer, refcount,
    value. Compare to NumPy's 8-byte inline double — the per-element
    waste is one of the four reasons NumPy is faster."""
    assert sys.getsizeof(0.0) >= 24


def test_larger_matrices_use_more_memory():
    """memory_bytes grows roughly linearly with element count."""
    small = Matrix.zeros(2, 2)  # 4 floats
    large = Matrix.zeros(20, 20)  # 400 floats
    assert memory_bytes(small) < memory_bytes(large)


def test_python_matrix_dwarfs_the_numpy_equivalent():
    """A 256x256 matrix uses ~256*256*28 bytes for boxed floats alone,
    plus list overhead. NumPy would use 256*256*8 = 0.5 MB total. We
    expect at least a 3x ratio just for the data — chapter 5 makes
    this concrete with measured numbers."""
    m = Matrix.zeros(256, 256)
    bytes_used = memory_bytes(m)
    numpy_equivalent = 256 * 256 * 8  # float64
    assert bytes_used > 3 * numpy_equivalent
