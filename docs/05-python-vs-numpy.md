# 5 — Python vs NumPy

**Goal**: explain (in writing) why your best pure-Python matmul is 100–1000× slower than `numpy.matmul` for the same algorithmic complexity.

This chapter is short on doing and long on thinking. The doing part is one extra plot panel. The thinking part is worth writing down somewhere — [`your_results.md`](../your_results.md) has a Q3 stub if you want a template, or use any notebook.

## The headline number

Open `results/python.csv`. Find the row for `matmul_numpy` at `n = 256` and the rows for your three variants at the same size. Compute the ratio of your fastest pure-Python time to NumPy's time. That's the **speedup factor**. For most students this is between 100× and 1000×.

That number is your aha-moment. The same big-O. The same flop count. NumPy is faster by *orders of magnitude*. Why?

## The four big reasons

NumPy isn't fast because of one trick. It's the cumulative effect of four engineering choices, each of which costs Python:

1. **No interpreter overhead per element.** Every `data[i*cols + j]` in your code goes through the Python interpreter: byte-code dispatch, attribute lookup, type check, allocation of intermediate floats. NumPy's inner loop is a tight C `for (...)` that the interpreter never sees. For a `256×256` matmul, the interpreter is in the loop ≈ 16 million times in your version and ≈ 0 times in NumPy's.

2. **Native floating-point representation.** A Python `float` is a heap-allocated object with a refcount, a type pointer, and the IEEE-754 double — about 28 bytes. NumPy's array is a flat block of `double` (8 bytes each). 3.5× less memory and, more importantly, contiguous memory means the CPU's prefetcher can stream the data without indirection.

3. **BLAS dispatch for large enough sizes.** When the matrices cross a size threshold (typically a few tens of rows), NumPy hands the work to a tuned BLAS implementation — OpenBLAS, MKL, or Apple's Accelerate. These are decades-old, hand-tuned libraries with cache blocking, register tiling, and SIMD. You won't out-write them in an afternoon.

4. **SIMD.** A modern CPU's AVX2 unit does four `double` multiplications per cycle. AVX-512 does eight. BLAS uses these. Your Python loop processes one number at a time. That alone is a 4–8× factor before any of the other reasons kick in.

There are smaller things too — branch prediction, memory alignment, multi-threading via OpenMP for big sizes — but those four explain most of the gap.

## What to write down

Pick the format you like (the Q3 stub in [`your_results.md`](../your_results.md), a notebook, anywhere) and capture: the NumPy-vs-best-Python speedup factor at `n = 256` from your CSV, and three of the four reasons above in your own words. The point isn't the artifact — it's that explaining the gap forces you to commit to a model of *why*, which a passive read of these docs doesn't.

You don't need to update the plot for this chapter — it already shows NumPy on the same axes. Take a moment to look at it. A log–log plot makes a 1000× gap into a fixed vertical offset; if your NumPy line is roughly parallel to your `entrywise` line but two decades below it, that's the picture.

## Optional curiosity

If you want to verify reason 1 (interpreter overhead), measure how long pure Python takes to execute *just* the indexing — no math. Replace the inner sum in `matmul_entrywise` with `_ = self[i, p] + other[p, j]` and time it. The number will be close to your matmul time, because the *math* in your matmul is essentially free; the *overhead* is everything.

→ Continue with [5b — Building and testing C++ projects: a primer](05b-cpp-build-system.md)

That primer is short and read-only — no exercises. It explains CMake, Ninja, FetchContent, and Catch2 *before* you touch them in chapter 6, so the build commands in chapters 6–9 read as something you understand rather than something you copy-paste.
