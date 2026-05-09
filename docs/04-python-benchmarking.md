# 4 — Benchmarking Python

**Goal**: measure runtime and peak memory of your three matmul variants and produce a CSV.

This is the second piece of new pedagogy in this assignment. After this chapter you will know how to time a function, what `tracemalloc` does, and why a single timed run is misleading.

## What's already done

[`python/bench/benchmark.py`](../python/bench/benchmark.py) is complete. It:

- Sweeps sizes `[16, 32, 64, 128, 256]` for the three pure-Python variants and adds `[512, 1024]` for `matmul_numpy`.
- Runs each (variant, size) cell five times. Discards a warm-up run. Reports the **median**.
- Times with `time.perf_counter` — the standard high-resolution monotonic clock.
- Measures peak memory with `tracemalloc.get_traced_memory()` between `start()` and `stop()`.
- Writes `results/python.csv` with columns `variant,size,time_ms,peak_kb,runs`.

[`python/bench/plot_results.py`](../python/bench/plot_results.py) is also complete. It reads any CSV in `results/` (so adding C++ later is automatic) and writes `results/comparison.png`.

## Run it

From `python/`:

```bash
uv run python bench/benchmark.py
```

This takes a couple of minutes — pure Python at `n = 256` is genuinely slow. Then:

```bash
uv run python bench/plot_results.py
```

You should now see `results/python.csv` and `results/comparison.png`.

## What to look at

Look at the CSV first. You should see something like:

```
variant,size,time_ms,peak_kb,runs
matmul_entrywise,16,0.51,12,5
matmul_entrywise,32,3.92,20,5
matmul_entrywise,64,31.40,30,5
matmul_entrywise,128,254.20,55,5
matmul_entrywise,256,2050.00,103,5
matmul_columnwise,16,0.55,15,5
...
```

(Numbers will differ. The shape — roughly `time_ms ≈ const · n³` for a triple loop — should be visible.)

The plot shows runtime on a log–log scale. A line of slope 3 means `time ~ n³`. Three of your four lines should look like that.

## Things to notice (write these in your AI diary as you go)

1. **Absolute order at `n = 256`**: Which variant is fastest? Which is slowest? The gap is usually within a factor of 2 — *all three are doing the same work*.

2. **`outerproduct` peak memory**: Did your implementation accumulate in place, or did it build a list of intermediate matrices? If you wrote it the wrong way, peak memory blows up by a factor of `k` and you'll see it here.

3. **`numpy` runtime at `n = 256`**: Compare to your fastest pure-Python variant at the same size. Note the ratio. You will explain it in chapter 5.

## Hints

<details markdown="1">
<summary>tracemalloc only sees Python allocations. What about the C arrays NumPy uses?</summary>

`tracemalloc` tracks memory the Python interpreter allocates. NumPy's data is allocated by the C extension and is not fully tracked. So the `peak_kb` column is honest for the pure-Python variants and an underestimate for `matmul_numpy`. That asymmetry is itself an interesting observation: NumPy is invisible to Python's memory tracker because it lives below the Python heap. C++ code in part B will be even more invisible.

For a fair memory comparison, look at the C++ numbers in chapter 8 — they're measured by the OS via `/usr/bin/time -v`.

</details>

<details markdown="1">
<summary>Why median, not mean?</summary>

OS scheduling jitters add positive outliers (a context switch in the middle of your loop adds 10 ms). Median is robust to those. Mean is not. The benchmark also discards one warm-up run because the very first call pays for module import, page faults, and the JIT-warm equivalents in the standard library.

</details>

<details markdown="1">
<summary>Can I make the benchmark go faster?</summary>

Two knobs: `RUNS = 5` at the top of `benchmark.py` (drop to 3 if you're impatient), or remove `n = 256` from the `SIZES` list for the slowest variant. Don't shrink the largest size below 128 — the cubic story isn't visible at small `n`.

</details>

→ Continue with [05 — Python vs NumPy](05-python-vs-numpy.md)
