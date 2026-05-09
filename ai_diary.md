# AI Diary — D5: Build Your Own Matrix Class

Fill in your answers below. Keep them short and specific. Paste the actual numbers from your benchmark runs, not what you remember reading somewhere.

> Hardware affects results. Note your CPU model and clock here so the numbers are reproducible.

**Hardware**: _e.g., Intel i7-1260P @ 2.1 GHz, 16 GB RAM, Linux Codespace_

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

## Q2 — C++ ijk vs ikj

Same algorithm, same complexity, different loop order. Did `ikj` beat `ijk`? By how much? Explain the difference in terms of cache lines and stride.

**Numbers (size 512×512):**

| Variant | -O0 (ms) | -O2 (ms) | -O3 (ms) |
|---|---|---|---|
| ijk | | | |
| ikj | | | |

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

The plot must show, on log–log axes for at least three sizes:

- Pure Python (your best variant)
- NumPy
- Naive C++ (best of ijk / ikj at `-O3`)
- Eigen
