# Your results — D5: Build Your Own Matrix Class

This file is **optional**. There is no graded deliverable for D5; the goal is the *aha moments*, not points. But the chapters keep pointing at numbers worth noticing — the Python vs NumPy gap, the C++ entrywise vs outerproduct gap (cache locality), Eigen vs your `-O3` C++ — and writing them down in one place is the single best way to make those observations stick. Use this file (or a notebook, or anywhere else) as that place.

The structure below mirrors the questions chapters 4, 5, 8, and 9 raise. Fill in whichever feel useful; skip whichever don't. Paste the actual numbers from your benchmark runs, not what you remember reading somewhere.

> Hardware affects results. Note your CPU model and clock here so the numbers are reproducible.

**Hardware**: _e.g., Intel i7-1260P @ 2.1 GHz, 16 GB RAM, Windows 11 native (or Ubuntu 22.04 native, or WSL2 Ubuntu)_

---

## Q1 — Which Python matmul was fastest?

Three variants: `matmul_entrywise`, `matmul_columnwise`, `matmul_outerproduct`. Same complexity O(n³). Did you predict the order before running? Were you right?

**Numbers (size 256×256, milliseconds, median of 5 runs):**

| Variant | Time (ms) | Peak memory (MB) |
|---|---|---|
| entrywise | | |
| columnwise | | |
| outerproduct | | |

**Your reasoning:**

_..._

---

## Q2 — C++ entrywise vs outerproduct (cache locality)

Same total work, same flop count, different memory-access pattern. Did `matmul_outerproduct` beat `matmul_entrywise`? By how much? Explain the difference in terms of cache lines and stride.

**Numbers (size 512×512):**

| Variant            | -O0 (ms) | -O2 (ms) | -O3 (ms) |
| ------------------ | -------- | -------- | -------- |
| matmul_entrywise   |          |          |          |
| matmul_outerproduct|          |          |          |

**Cache-line explanation:**

_..._

---

## Q3 — NumPy vs your best Python matmul

How big is the gap (factor)? List at least three things NumPy does that your Python does not.

**Speedup of NumPy over your best Python (size 256×256):** _×_

**Why NumPy is fast:**

1. _..._
2. _..._
3. _..._

---

## Q4 — Eigen vs your best C++ matmul

How big is the gap? Smaller than the Python/NumPy gap? Why?

**Speedup of Eigen over your best `-O3` C++ (size 1024×1024):** _×_

**Why Eigen is fast (be specific — expression templates, BLAS dispatch, vectorisation, blocking):**

_..._

---

## Q5 — Production decision

Ignore the assignment. You have a 1024×1024 matrix multiplication to do once. What do you actually use, and why? (One paragraph, max five sentences.)

_..._

---

## The plot

Paste or link to `results/comparison.png` here:

![Comparison plot](results/comparison.png)

The plot is most informative when it shows, on log–log axes for at least three sizes:

- Pure Python (your best variant)
- NumPy
- Naive C++ (best of `matmul_entrywise` / `matmul_outerproduct` at `-O3`)
- Eigen
