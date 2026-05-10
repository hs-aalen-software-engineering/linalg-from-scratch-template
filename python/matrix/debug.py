"""
Inspection helpers for Matrix instances.

These functions are provided so you can experiment with the Matrix class
*before* the TODO methods are implemented — in particular before
`__repr__` (TODO 1.4). They reach directly into the public attributes
`rows`, `cols`, `data` and don't depend on any student code.

Use them in `tests/test_experiments.py` and any scratch tests of your
own to see what's actually stored in memory.
"""
import sys

from .matrix import Matrix


def dump_raw(m: Matrix) -> str:
    """Return a debug string showing the raw flat storage of a Matrix.

    Output looks like:

        Matrix(rows=2, cols=3, data=[1.0, 2.0, 3.0, 4.0, 5.0, 6.0])

    Useful before you implement `__repr__` (TODO 1.4), or any time you
    want to see what is actually in memory rather than the formatted
    view your `__repr__` will eventually produce.
    """
    return f"Matrix(rows={m.rows}, cols={m.cols}, data={m.data})"


def memory_bytes(m: Matrix) -> int:
    """Estimate the memory footprint of a Matrix in bytes.

    Sums three contributions:

    * `sys.getsizeof(m)` — the Matrix object itself (instance dict / slots).
    * `sys.getsizeof(m.data)` — the list's metadata + pointer array.
    * `sum(sys.getsizeof(x) for x in m.data)` — each Python `float`
      as a separate heap-allocated object.

    This is the *all-Python* footprint. A NumPy `np.float64` array of
    the same shape would use `len(arr) * 8` bytes (`itemsize=8`) for
    the data, with no per-element wrapper. The comparison is the
    foundation of chapter 5's "why NumPy is fast" discussion: each
    Python float carries ~28 bytes of object overhead beyond its
    8-byte value.
    """
    obj = sys.getsizeof(m)
    list_obj = sys.getsizeof(m.data)
    floats = sum(sys.getsizeof(x) for x in m.data)
    return obj + list_obj + floats
