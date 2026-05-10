# 2 — Unit tests, motivated

**Goal**: understand what `pytest` is doing under the hood, and add three of your own tests for behaviour that nobody has written down yet.

## Why test at all

Matrix multiplication has many edge cases — shape mismatches, transposes, identity, zero matrices, associativity — and the *same* bug can pass small tests and fail on a `5×7` matrix. You will be writing three matmul variants in chapter 3 and then asking them to agree. The only way that question is meaningful is if you trust each variant individually first.

Tests give you that trust. They are also, in this assignment, the safety net for *refactoring*: if you decide your `transpose` is wrong and rewrite it, the tests catch any regression you introduce.

This is the first time tests appear in this course. Other tests in the [course test directory](https://github.com/L3GJ0N/ai_supported_software_development_hs_aalen/tree/main/prompts/testat) are part of exam preparation; you will see them again later in the semester.

## How `pytest` works (briefly)

```python
def test_thing():
    assert something == expected
```

`pytest` finds every `test_*` function and runs it. If any `assert` fails, that test fails and the rest still run. There is no setup ceremony — a function is a test.

Useful features you'll see in `tests/test_matrix.py`:

- `pytest.raises(SomeError)` — expect an exception
- `@pytest.mark.parametrize("x", [1, 2, 3])` — same test, multiple inputs
- `@pytest.mark.<name>` — *markers*, custom labels you can filter by. The skeleton uses `@pytest.mark.ch1` and `@pytest.mark.ch3` so you can run just one chapter's tests at a time. Marker names must be registered in `pyproject.toml` under `[tool.pytest.ini_options]` (otherwise pytest warns).
- Command-line flags: `-v` (verbose), `-k <pattern>` (filter by name), `-m <expr>` (filter by marker), `--lf` (only the tests that failed last time).

```bash
cd python
uv run pytest -v                       # everything
uv run pytest -v -m ch1                # only chapter-1 tests
uv run pytest -v -m "ch1 or ch3"       # union of two markers
uv run pytest -v -k transpose          # only tests with "transpose" in the name
uv run pytest -v --lf                  # only the previously failing tests
```

The `-m` and `-k` flags compose: `-v -m ch3 -k matmul` runs chapter-3-tagged tests whose name contains "matmul."

## Why tests live in their own file

Open `tests/test_matrix.py` next to `matrix/matrix.py` and notice the layout: the implementation and the tests sit side-by-side, but in different folders. Two reasons we do this:

- **Separation of concerns.** A library should be importable without dragging its tests along. Anyone who installs `matrix-from-scratch` to use the class shouldn't be paying for hundreds of test functions to load.
- **Visual clarity.** When you scan the project, it's immediately obvious which file is "the code" and which file is "what the code is supposed to do." That helps you (and code reviewers) keep the two in sync.

The first line of `test_matrix.py` that does anything substantive is:

```python
from matrix import Matrix
```

How does that work? You haven't installed `matrix` from anywhere — it's a folder sitting next to your `tests/` folder. The full picture:

```text
python/
  pyproject.toml         ← marks `python/` as the project root
  matrix/__init__.py     ← presence of __init__.py makes `matrix/` a package
  matrix/matrix.py       ← defines the Matrix class
  tests/__init__.py      ← presence of __init__.py makes `tests/` a package
  tests/test_matrix.py   ← imports `from matrix import Matrix`
```

When you run `uv run pytest`, pytest walks up from the test file looking for the project root. It finds `python/pyproject.toml` and uses that directory as the *rootdir*. Pytest then prepends `python/` to Python's import path before loading any test files. From inside `test_matrix.py`, `from matrix import Matrix` resolves `matrix/` as a top-level package — both `matrix/` and `tests/` are direct children of `python/`, so they can find each other.

A few clarifications worth nailing down:

- **`pyproject.toml` itself doesn't *cause* the import to work** — pytest's rootdir-discovery does. The file is the *marker* pytest uses to find the project root, and the `[tool.pytest.ini_options]` block inside it configures pytest's behaviour (which directories to search, which markers exist, etc.).
- **The two `__init__.py` files matter.** They tell Python "these directories are packages." If you removed `tests/__init__.py`, pytest would treat `tests/` as a "rootless" test directory and the rootdir-walk would behave differently — it's safer to have it.
- **No `pip install` is needed** for the import to work. The whole mechanism is pytest-driven; `uv sync` only installs *dependencies* (numpy, matplotlib, pytest, ruff). The `matrix` package is reachable solely because pytest puts the project root on `sys.path` at startup.

This is a pattern you will see in almost every Python project that has a tests folder. It's worth understanding now rather than copy-pasting it later.

## What's in `pyproject.toml`?

You've now seen `pyproject.toml` referenced from three angles: `uv sync` reads it to install dependencies, ruff reads it for lint settings, pytest uses it to find the project root *and* for its own configuration. It's the single configuration file modern Python projects center on. Here's what each block in our copy does:

```toml
[project]
name = "matrix-from-scratch"
version = "0.1.0"
description = "Build your own Matrix class — Lecture 5 D5 deep-dive"
requires-python = ">=3.12"
dependencies = ["numpy>=2.0", "matplotlib>=3.8"]
```

The `[project]` table is the standard ([PEP 621](https://peps.python.org/pep-0621/)) project metadata. `name`, `version`, `description` are self-explanatory. `requires-python = ">=3.12"` means `uv` will refuse to install into an older interpreter — important because we use `typing.Self` (added in 3.11) and rely on built-in generics like `list[float]`. `dependencies` are the *runtime* packages your code imports: NumPy for the comparison benchmark in chapter 5, Matplotlib for the plot.

```toml
[dependency-groups]
dev = ["pytest>=8.0", "ruff>=0.4"]
```

Dev-only tools that aren't needed to *use* the matrix package, only to *develop* it. `uv sync` installs the `dev` group by default; `uv sync --no-dev` would skip it. Pytest and ruff don't appear in `dependencies` because nothing inside `matrix/` imports them — they're tooling, not library deps.

```toml
[tool.pytest.ini_options]
testpaths = ["tests"]
python_files = ["test_*.py"]
markers = [
  "ch1: chapter 1 — basic Matrix operations (...)",
  "ch3: chapter 3 — matmul (...)",
]
```

Pytest configuration. `testpaths = ["tests"]` tells pytest *where* to look — so `uv run pytest` without arguments only searches `tests/`, not the whole repo. `python_files = ["test_*.py"]` is the file-name pattern pytest considers; this matches the default, but spelling it out makes the intent obvious. `markers` registers our custom `@pytest.mark.ch1` / `@pytest.mark.ch3` decorators — without registration, pytest emits a warning every run.

There is no `addopts` line. Some projects set `addopts = "-q --tb=short"` to make the default output terser, but for learning what pytest actually does, the default output (one line per test, full tracebacks on failure) is more useful. If you want quieter output for a particular run, pass the flag yourself: `uv run pytest -q`.

```toml
[tool.ruff]
line-length = 100
target-version = "py312"

[tool.ruff.lint]
select = ["E", "F", "I", "B", "UP"]
ignore = ["E501"]
```

Ruff is our linter and formatter (we use it instead of black + flake8 + isort — same job, much faster, single tool). `target-version = "py312"` lets it suggest modern syntax (e.g., `list[int]` over `List[int]`). The `select` codes turn on rule families: `E`/`F` are pycodestyle/pyflakes basics, `I` sorts imports, `B` catches likely bugs (mutable default arguments, unused loop variables), `UP` flags out-of-date syntax. We `ignore = ["E501"]` (line-too-long) because `line-length = 100` already controls that — without the ignore, ruff would also complain at the default 79.

One thing the file is *missing*: a `[build-system]` table. Most published Python packages have one (it tells `pip` how to build a wheel). We omit it because `matrix-from-scratch` is never installed — it's read directly from the repo by pytest, which uses the `pyproject.toml` only as a *rootdir marker* and config source.

## What to do

1. Read [`python/tests/test_matrix.py`](../python/tests/test_matrix.py). All the chapter-1 tests are pre-written. Get them green first (you should have done this in chapter 1 — re-run `pytest -v` to confirm).

2. Add **three new tests** of your own at the bottom of the file. Required:

   - One test that checks a *property* of `transpose` other than involution (e.g., that `transpose` of a matrix of zeros is still zeros, or that it commutes with `Matrix.eye`).
   - One test that checks a *property* of `slice` you didn't see in the existing tests (e.g., slicing the entire matrix returns an equal matrix; slicing twice composes).
   - One test that catches an *invalid* operation (use `pytest.raises`).

3. Confirm your three tests fail when you deliberately break the corresponding method (commit nothing — just add a `+ 1` to verify the test catches it, then revert).

You are not graded on the tests, but writing them is the cheapest way to find a bug in your own code before chapter 3 amplifies it across three matmul variants.

## Hints

<details markdown="1">
<summary>What's a "good" test to add?</summary>

Tests that fail when reasonable bugs are introduced. The bad version of "test that `transpose` is correct" is to copy the implementation into the test — same code, same bug. The good version is to use a *property* the implementation doesn't directly mention (e.g., for any matrix `A`, the diagonal of `A.transpose()` equals the diagonal of `A`).

</details>

<details markdown="1">
<summary>How small is too small?</summary>

`Matrix.from_rows([[1]])` is fine. The point of a test is to be unambiguous, not to look real.

</details>

→ Continue with [03 — Three matmul implementations](03-python-three-matmuls.md)
