# 1 — Python Matrix class

**Goal**: implement the basic operations of [`python/matrix/matrix.py`](../python/matrix/matrix.py) so the existing tests pass — except the matmul tests, which come in chapter 3.

## The shape contract

A `Matrix` has a shape `(rows, cols)` and a flat row-major data array of length `rows * cols`. The element at logical position `(i, j)` lives at flat position `data[i * cols + j]`. This is the same layout NumPy uses by default, the same layout the C++ skeleton uses, and the same layout BLAS expects.

The formula `data[i * cols + j]` is the heart of the whole assignment. The matrix is rectangular in our heads but memory is a single line of cells, so we need a deterministic translation in both directions. Spend a few minutes here — every method you will write reads or writes through this formula.

### The picture

A `2 × 3` matrix lives in memory as 6 contiguous cells:

```text
Logical view (rectangular)            Memory view (flat, length 6)

         j=0    j=1    j=2             flat:   0    1    2    3    4    5
       ┌──────┬──────┬──────┐          data = [ a ,  b ,  c ,  d ,  e ,  f ]
  i=0  │  a   │  b   │  c   │                  └─── row 0 ───┘└─── row 1 ───┘
       ├──────┼──────┼──────┤
  i=1  │  d   │  e   │  f   │
       └──────┴──────┴──────┘
```

**Row-major** means we lay row 0 down first, then row 1, then row 2, …. Walking forward through `data` walks across a row, hits the right edge, and continues at the start of the next row.

### Forward: `(i, j)` → flat index

Read the formula left to right:

```text
data[ i * cols + j ]
      └───┬───┘ └┬┘
          │      └─── how far across the current row
          └────────── how many full rows to skip first
                      (each row is `cols` elements wide)
```

Worked examples on the `2 × 3` matrix above (`cols = 3`):

| `(i, j)` | computation     | flat index | element |
| -------- | --------------- | ---------- | ------- |
| `(0, 0)` | `0 * 3 + 0 = 0` | 0          | `a`     |
| `(0, 2)` | `0 * 3 + 2 = 2` | 2          | `c`     |
| `(1, 0)` | `1 * 3 + 0 = 3` | 3          | `d`     |
| `(1, 2)` | `1 * 3 + 2 = 5` | 5          | `f`     |

This is the formula your `__getitem__`, `__setitem__`, `transpose`, `slice`, and all three matmul variants will use, over and over.

### Reverse: flat index `k` → `(i, j)`

Inverting integer arithmetic:

```python
i = k // cols    # which row? (integer division — count full rows passed)
j = k  % cols    # how far across that row? (remainder)
```

Worked example, same `2 × 3` matrix (`cols = 3`), flat index `k = 4`:

```text
i = 4 // 3 = 1          → row 1
j = 4  % 3 = 1          → column 1
data[4] = 'e'           ✓ matches the diagram above
```

You'll need the reverse direction less often than the forward one — most code goes `(i, j) → flat` — but `__repr__` and a few tests are cleaner when you can iterate `for k in range(len(data))` and recover `(i, j)`.

### Why row-major (and not column-major)?

If we stored columns first instead — **column-major** — the same six cells would sit in memory as `[a, d, b, e, c, f]` and the formula would be `data[j * rows + i]`. Same data, different traversal order. We pick row-major because:

- It's NumPy's default and C's natural array layout.
- Walking forward through `data` walks across a row — matching the way we naturally write nested loops `for i: for j:`.
- It sets up the cache-locality discussion in [chapter 7](07-cpp-three-matmuls.md): for fixed `i` and increasing `j`, the addresses `data[i * cols + j]` are *consecutive*, so the CPU's prefetcher streams them one cache line at a time. Strided access (e.g., walking down a column) costs more.

Eigen and Fortran default to column-major. BLAS accepts either via a flag. The choice is conventional, not deep — but once you've picked, you have to be consistent everywhere or your indexing will be silently wrong.

### Vectors as matrices

Vectors in this exercise are matrices: a column vector is `(n, 1)`, a row vector is `(1, n)`. There is no separate `Vector` class. Once `Matrix @ Matrix` works, matrix-vector, vector-matrix, dot product, and outer product all fall out of it for free.

### Constructing matrices

The class provides four ways to build a `Matrix`. The first one is the one you'll use in tests and almost everywhere in user code; the rest are factory classmethods for the situations the primary constructor doesn't cover.

```python
# 1. Primary — list-of-lists, NumPy style. Shape is deduced from the input.
M = Matrix([[1, 2, 3],
            [4, 5, 6]])                  # shape (2, 3)

# 2. Empty matrix of a given shape — the C++-style "shape-only" form.
Z = Matrix.zeros(2, 3)                   # 2×3 of zeros

# 3. Identity / random — convenience.
I = Matrix.eye(3)                        # 3×3 identity
R = Matrix.random(2, 3, seed=42)         # reproducible random

# 4. From a flat row-major buffer — used internally by the matmul variants
#    and any code that already has its data laid out flat. Bypasses the
#    list-of-lists path.
F = Matrix.from_flat(2, 3, [1, 2, 3, 4, 5, 6])

# `Matrix.from_rows([[...]])` is also available as a verbose alias for #1 —
# it does the same thing as `Matrix([[...]])`.
```

The primary constructor matches what you saw in the lecture and in the NumPy introduction (`np.array([[1, 2], [3, 4]])`). The shape comes from the data you passed in: count the outer list to get `rows`, count the first inner list to get `cols`, and validate that every other inner list has the same length.

When you write the matmul variants in chapter 3, you will allocate the output with `Matrix.zeros(self.rows, other.cols)` and fill it in by index — never with `Matrix([[...]])`, because at that point you do not yet have the data, only the shape.

> **Try it before you start.** [`python/tests/test_experiments.py`](../python/tests/test_experiments.py) is a playground of *passing* tests that exercises every construction pattern above (plus two helpers — `dump_raw` and `memory_bytes` from `matrix.debug` — that let you peek at the raw flat buffer and the memory footprint without going through `__repr__`). Run it with `uv run pytest tests/test_experiments.py -v` to see what the provided machinery produces, then modify the tests, drop in `print(dump_raw(m))` calls, and re-run with `-s` to see your output. Get familiar with the data model here before you start writing TODOs.

## Rule: preallocate, don't grow

A matrix has a known shape from the moment it's created, so its underlying storage should be allocated once at full size. **Do not use `list.append`, `list.extend`, `+=` on lists, or list-comprehension-into-`append` patterns to build up the data buffer.** Allocate the result list with the final length and write into it by index.

✅ Good:

```python
out = [0.0] * (rows * cols)
out[i * cols + j] = value
```

❌ Bad:

```python
out = []
out.append(value)
```

Why this rule:

- **Mental model.** Matrices are fixed-shape objects. NumPy, Eigen, and the C++ `std::vector<double>` you'll write later all preallocate. Writing `append` here trains a reflex you'll have to unlearn in the C++ half.
- **Memory and copies.** A Python `list` is a heap-allocated pointer array with a *capacity* that is usually larger than its length. When you `append` past the capacity, CPython grows the buffer (roughly geometrically: `new_capacity ≈ old_capacity × 1.125 + 6`) and may `memcpy` the entire array to a new region. For a `256×256` matmul filled with `append` that's about 17 reallocations of an array that ends at 65,536 entries — wall-clock cost is small, but it's overhead the algorithm doesn't ask for.
- **C++ analogue.** `std::vector::push_back` has the *exact* same surprise: amortised O(1), occasional O(n) on grow. The C++ skeleton's data buffer is preallocated for the same reason. One reflex covers both halves.

The provided constructors and helpers (`zeros`, `eye`, `from_flat`, `random`, and the primary list-of-lists `__init__`) already follow this rule. Your TODOs — `transpose`, `slice`, all three matmul variants — should too: every output `Matrix` is created with `Matrix.zeros(out_rows, out_cols)` (or `Matrix.from_flat(rows, cols, flat)` if you have flat data) and then filled in by index.

## Python class mechanics for this exercise

If you've used Python classes before — `__init__`, `self`, attribute access — feel free to skim. Three Python-specific bits come up in the `Matrix` skeleton and are worth nailing down before you start filling in TODOs.

### Dunder methods are Python's operator hooks

Names like `__init__`, `__getitem__`, `__eq__`, `__repr__`, `__matmul__` use double underscores on both sides — pronounced "**dunder**" (**d**ouble **under**score). They are Python's hooks: writing certain *syntax* calls a specific dunder method on the object.

| You write             | Python calls                  | Why we implement it       |
| --------------------- | ----------------------------- | ------------------------- |
| `Matrix(...)`         | `Matrix.__init__(self, ...)`  | construction              |
| `m[i, j]`             | `m.__getitem__((i, j))`       | indexed read              |
| `m[i, j] = x`         | `m.__setitem__((i, j), x)`    | indexed write             |
| `m == other`          | `m.__eq__(other)`             | equality with tolerance   |
| `repr(m)`, `print(m)` | `m.__repr__()`                | debug-printable form      |
| `a @ b`               | `a.__matmul__(b)`             | matrix multiplication     |

By implementing these, your class gets to feel like a built-in: you index it with brackets, compare it with `==`, multiply it with `@` — the same syntax you'd use on a NumPy array. Without dunders you'd have to write `m.get(i, j)` and `m.set(i, j, x)` — verbose, and totally unlike `np.array`.

The double-underscore convention is Python saying "this name is reserved for the language; don't reuse it for ordinary purposes." There's nothing magical about the underscores themselves — it's just how the convention reads.

### `self` vs `cls`: instances vs classes

You'll see two patterns in the skeleton:

```python
def transpose(self) -> Matrix:
    # instance method — self is one specific matrix
    ...

@classmethod
def zeros(cls, rows, cols) -> Matrix:
    # class method — cls is the class itself
    return cls.from_flat(rows, cols, [0.0] * (rows * cols))
```

`self` is a reference to the *instance* the method is called on. When you write `m.transpose()`, Python passes `m` as the first argument — `self` inside `transpose` is the `m` you wrote.

`cls` is a reference to the *class* itself. When you write `Matrix.zeros(2, 3)`, Python passes `Matrix` (the class object, not any matrix) as the first argument — `cls` inside `zeros` *is* `Matrix`. The `@classmethod` decorator above the `def` is what tells Python "the first argument is the class, not an instance."

Why the distinction? Instance methods need access to a specific matrix's data — `self.rows`, `self.data`. Classmethods don't have a matrix yet; they're *building* one. They use `cls(...)` to construct so that subclassing keeps working: if some `SparseMatrix` ever inherits from `Matrix`, then `SparseMatrix.zeros(2, 3)` builds a `SparseMatrix` (because `cls` is `SparseMatrix` in that call), not a plain `Matrix`.

`self` and `cls` are *conventions*, not keywords. You could write `def transpose(this):` and Python wouldn't complain. But every Python programmer reads `self` automatically, so we follow the convention.

#### Where the names come from

`self` was inherited from **Smalltalk** (1970s) via Modula-3 — both languages named the message receiver `self`, and Python kept the convention when Guido designed methods. The reason Python makes `self` *explicit* (not implicit like Java's `this`) is that `m.foo(x)` is exactly equivalent to `Matrix.foo(m, x)` — making the receiver visible in the `def` keeps that mental model honest.

`cls` is short for **`class`**. Why the abbreviation? Because `class` is a reserved keyword in Python (you use it to declare classes), so you literally can't write `def zeros(class, ...)` — the parser would reject it. A few old codebases use `klass` to dodge the same problem; PEP 8 settled the convention at `cls`.

#### Type-hinting `self` and `cls`

You can annotate them, but the type checker already infers them — so people don't bother with `self: Matrix` or `cls: type[Matrix]`. The useful annotation is the **return type**: how do you say "this method returns the same type as `self`/`cls`, including any subclass"?

The modern answer (Python 3.11+, [PEP 673](https://peps.python.org/pep-0673/)) is `typing.Self`:

```python
from typing import Self

class Matrix:
    def transpose(self) -> Self:                        # not -> Matrix
        ...

    @classmethod
    def zeros(cls, rows: int, cols: int) -> Self:       # not -> Matrix
        return cls.from_flat(rows, cols, [0.0] * (rows * cols))
```

`Self` means "the type of `self`/`cls` in this exact context, even if a subclass calls the method." If a hypothetical `SparseMatrix(Matrix)` ever calls `SparseMatrix.zeros(...)`, type checkers correctly infer the return type as `SparseMatrix`. With the older `-> Matrix` annotation they'd return `Matrix`, which is technically wrong.

Our skeleton uses `Self` throughout — that's why you see `from typing import Self` at the top of [`matrix.py`](../python/matrix/matrix.py) and `-> Self` on every method that constructs or returns a matrix.

### Python doesn't have function overloading

In C++ or Java, you can write multiple methods or constructors with different signatures and the compiler picks the right one at the call site:

```cpp
// C++ — three constructors, the compiler dispatches on argument types
Matrix(int rows, int cols);
Matrix(int rows, int cols, std::vector<double> data);
Matrix(std::vector<std::vector<double>> rows);
```

Python doesn't work that way. **Every method in a class has exactly one `def`.** If you write two `def __init__` blocks with different parameters, the second one *replaces* the first, silently — there is no error and no compile-time dispatch on argument types.

So how does our `Matrix` class support five different ways of constructing? Three Python idioms cover the use cases:

1. **Default arguments** — e.g., `def random(cls, rows, cols, seed=None)`. The caller can omit `seed`. One `def`, multiple call shapes.
2. **`*args` / `**kwargs`** — variadic arguments. Useful when you genuinely don't know how many. Not used in this skeleton.
3. **Factory classmethods** — what we use here. Each "alternative constructor" is a separate classmethod with a descriptive name:

   ```python
   Matrix([[1, 2], [3, 4]])              # primary __init__: list-of-lists
   Matrix.zeros(2, 3)                    # factory: shape only
   Matrix.from_flat(2, 3, [1, 2, 3, 4])  # factory: flat buffer
   Matrix.eye(3)                         # factory: identity
   Matrix.random(2, 3, seed=42)          # factory: random
   ```

   Each factory is a separate function with a clear name, so reading the call site tells you exactly what kind of matrix you're getting. This is also the pattern NumPy uses (`np.array`, `np.zeros`, `np.eye`, `np.random.rand`) — and the standard-library approach (`datetime.now`, `dict.fromkeys`, `Path.home`).

Factory classmethods are *not* "second-class" constructors. In Python they're the idiomatic way to give a class multiple construction paths.

## Naming conventions: when terse beats descriptive

If you've internalised "use descriptive variable names" — good, that rule covers most of the code you will ever write. But the skeleton you're about to read mixes long names like `self.rows`, `self.data`, and `from_flat(rows, cols, data)` with short ones like `nrows`, `ncols`, `flat`, and especially the loop indices `i`, `j`, `k`. The rule didn't break; the rule has a domain-specific exception. Knowing where the boundary sits is part of being able to read code in this field.

### The general rule (still true)

In most software — web backends, business logic, application code — descriptive names beat terse ones every time. `for user_id in active_user_ids:` is much better than `for u in uids:`, because the names tell you *what kind of thing you're iterating over*. PEP 8 explicitly says to avoid single-character names except in narrow loop bodies, and every modern style guide echoes that.

### The numerical-code exception

When the code implements a mathematical operation, the math itself uses short symbols. Matrix multiplication is

$$C_{ij} = \sum_{k} A_{ik} B_{kj}$$

When the textbook formula uses `i`, `j`, `k`, the Python that translates the formula is *more readable*, not less, when it reuses the same letters:

```python
for i in range(nrows):
    for j in range(ncols):
        for k in range(inner_dim):
            C[i, j] += A[i, k] * B[k, j]
```

That is the formula written line for line. Replacing it with

```python
for row_index in range(number_of_rows):
    for column_index in range(number_of_columns):
        for inner_index in range(inner_dimension):
            result_matrix[row_index, column_index] += (
                a_matrix[row_index, inner_index]
                * b_matrix[inner_index, column_index]
            )
```

breaks the visual mapping to the textbook expression. The verbose version takes longer to read despite being "more descriptive" — the eyes have to parse identifiers instead of recognising symbols.

This convention is older than Python — Fortran (1957), MATLAB, R, NumPy, SciPy, JAX, PyTorch, BLAS, LAPACK, Eigen all use terse names (`nrows`, `ncols`, `n`, `m`, `k`, `i`, `j`, `axis`, `dim`, `dtype`). When you read a numerical Python file, terse identifiers are a *signal* that says: "this is implementing math, expect the variables to mirror a formula."

### The layered rule we use in this codebase

So which length to use when? Look at the **scope and audience** of each identifier:

- **Class attributes and public parameters** — descriptive. They are long-lived and appear in many call sites. We use `self.rows`, `self.cols`, `self.data`, `from_flat(rows, cols, data)`.
- **Local variables in numerical methods** — terse, math-aligned. Short scope; the surrounding context tells the reader what they are. We use `nrows`, `ncols`, `flat`, `col_k`, `row_k`.
- **Loop indices over math objects** — single letters that match the formula. `i`, `j`, `k`, `m`, `n`, `p`.
- **Loop indices over collections of "things"** — descriptive even in numerical code. `for row in rows:` is fine because `row` is conceptually a row, not a math index.

You will see all four layers in [`matrix.py`](../python/matrix/matrix.py) and the chapter-3 matmul TODOs. They're not inconsistent — they're applying a single rule (match the *audience* of the identifier) at different scopes.

### What this means for you as a software engineer

Rules like "use descriptive names" exist because they tend to maximise readability. When the *reason* (readability) is better served by a different convention (mirroring the math), the rule bends. Recognising those situations is part of becoming fluent in a domain.

This is also a useful skill for code review: when a colleague writes `nrows` in numerical code, that's not "lazy"; it's domain-appropriate. When the same colleague writes `n_u` for `number_of_users` in a web handler, that's lazy. The cue is whether the code is implementing a formula or implementing a workflow.

PEP 8 itself anticipates this — the very first sentence of its "Naming Conventions" section says naming is the most important thing about good code, and the document follows up with "consistency within a project is more important than consistency with this style guide." Pick a layered convention and stick with it; don't fight it because a single subsystem doesn't match general-purpose style.

## What to fill in

Open [`python/matrix/matrix.py`](../python/matrix/matrix.py). The methods marked with TODO need bodies. They are listed in the order to do them in:

1. `__getitem__` — read `data[i * cols + j]`
2. `__setitem__` — write `data[i * cols + j]`
3. `__eq__` — same shape, all entries equal up to `1e-9`
4. `__repr__` — a readable format that round-trips back through `Matrix(...)`. The suggested NumPy-style form is `Matrix([[1.0, 2.0], [3.0, 4.0]])` so a debug print can be pasted straight into a test.
5. `transpose` — return a new Matrix of shape `(cols, rows)` with `out[j, i] = self[i, j]`
6. `slice` — return a new Matrix containing rows `[r0, r1)` and columns `[c0, c1)`

**Do not** modify the constructor or the helper classmethods (`zeros`, `eye`, `from_flat`, `from_rows`, `random`). They're already correct and the rest of the assignment depends on their exact behaviour.

> **`slice` unlocks two more helpers for free.** The provided `get_row` and `get_column` methods are one-liners that delegate to `slice`. Once your `slice` is correct, both helpers work — the chapter-3 matmul variants rely on them. This is also a small worked example of the SE principle "build a primitive, compose related operations on top": `slice` is the primitive; `get_row` and `get_column` are compositions.

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
