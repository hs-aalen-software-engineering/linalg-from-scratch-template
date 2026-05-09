# 5 — Python vs NumPy

**Goal**: explain (in writing) why your best pure-Python matmul is 100–1000× slower than `numpy.matmul` for the same algorithmic complexity.

This chapter is short on doing and long on thinking. The doing part is one extra plot panel. The thinking part is what you'll write in `ai_diary.md`.

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

## What to write

Open `ai_diary.md`. Fill in **Q3**: NumPy vs your best Python matmul, and the three things NumPy does that you don't. Use the four reasons above as a starting point but make your wording yours, not theirs. List actual numbers from your CSV.

You don't need to update the plot for this chapter — it already shows NumPy on the same axes. Take a moment to look at it. A log–log plot makes a 1000× gap into a fixed vertical offset; if your NumPy line is roughly parallel to your `entrywise` line but two decades below it, that's the picture.

## Optional curiosity

If you want to verify reason 1 (interpreter overhead), measure how long pure Python takes to execute *just* the indexing — no math. Replace the inner sum in `matmul_entrywise` with `_ = self[i, p] + other[p, j]` and time it. The number will be close to your matmul time, because the *math* in your matmul is essentially free; the *overhead* is everything.

→ Continue with [06 — C++ Matrix class](06-cpp-matrix-class.md)
