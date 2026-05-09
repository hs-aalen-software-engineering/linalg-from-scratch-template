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

Three useful features:

- `pytest.raises(SomeError)` — expect an exception
- `@pytest.mark.parametrize("x", [1, 2, 3])` — same test, multiple inputs
- The `-v` flag — verbose; the `-k` flag — filter by name; the `--lf` flag — only the tests that failed last time

```bash
cd python
uv run pytest -v                       # everything
uv run pytest -v -k transpose          # only tests with "transpose" in the name
uv run pytest -v --lf                  # only the previously failing tests
```

## What to do

1. Read [`python/tests/test_matrix.py`](../python/tests/test_matrix.py). All the chapter-1 tests are pre-written. Get them green first (you should have done this in chapter 1 — re-run `pytest -v` to confirm).

2. Add **three new tests** of your own at the bottom of the file. Required:

   - One test that checks a *property* of `transpose` other than involution (e.g., that `transpose` of a matrix of zeros is still zeros, or that it commutes with `Matrix.eye`).
   - One test that checks a *property* of `slice` you didn't see in the existing tests (e.g., slicing the entire matrix returns an equal matrix; slicing twice composes).
   - One test that catches an *invalid* operation (use `pytest.raises`).

3. Confirm your three tests fail when you deliberately break the corresponding method (commit nothing — just add a `+ 1` to verify the test catches it, then revert).

You are not graded on the tests themselves, but the AI-Diary deliverables expect that you have done this exercise.

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
