# 8 — C++ vs Eigen

**Goal**: run the C++ benchmark at three optimisation levels, run Eigen's reference, and add the rows to your CSV.

## Run the benchmark

The benchmark binary is built by CMake. With your three build directories from chapter 7:

```bash
cd cpp
./build-O0/benchmark > ../results/cpp-O0.csv
./build-O2/benchmark > ../results/cpp-O2.csv
./build-O3/benchmark > ../results/cpp-O3.csv
```

(The CSV is on stdout; progress messages go to stderr so the redirect stays clean.)

That gives you three CSVs alongside `results/python.csv`. The Python plot script picks them all up automatically.

## Measure peak memory

`std::chrono` measures time. Memory is measured by the OS. On Linux (the Codespace and the Devcontainer), `/usr/bin/time -v` reports peak resident-set size:

```bash
/usr/bin/time -v ./build-O3/benchmark 2>&1 | grep "Maximum resident"
```

You'll see one line: `Maximum resident set size (kbytes): N`. That's the largest the binary's working set ever was. For a `1024×1024` matmul with three matrices of doubles, the math is `3 × 1024² × 8 ≈ 24 MiB`. Eigen and the variants are within a factor of 2 of that. Compare to Python's `tracemalloc` numbers — usually Python's working set is 5–10× larger because of the per-element float boxing.

You don't need a separate run for each variant — the binary times all three and the OS reports the cumulative peak. If you care about per-variant peak, you can comment out two of the three blocks in `bench/benchmark.cpp` and re-run.

## Re-plot

```bash
cd ../python
uv run python bench/plot_results.py
```

`results/comparison.png` now has lines for everything. Look for:

- The Eigen line is below your `matmul_ikj` line at large sizes — that's BLAS dispatch + tiling + vectorisation.
- The `matmul_ikj` line is below `matmul_ijk` by a factor of 5–10× at `n = 512` and beyond — that's cache locality.
- All your `-O3` C++ lines are 50–100× faster than your fastest Python at the same size.
- `numpy` and Eigen are within a small constant factor of each other — both call BLAS for large sizes.

## What to write

Open `ai_diary.md`. Fill in:

- **Q2** — ijk vs ikj numbers and the cache-line explanation.
- **Q4** — Eigen vs your best `-O3` C++ numbers and what Eigen does that you don't.

For Q4, the things to mention:

- **Expression templates** — `A * B` in Eigen doesn't immediately allocate a matrix; it builds a *node* in an expression tree. The actual computation is fused with the assignment. You skip an intermediate buffer.
- **Cache blocking** — for matrices that don't fit in L1/L2, Eigen partitions the multiplication into tiles small enough to live in cache. You wrote three loops; Eigen writes six (with two outer levels of tiling).
- **Vectorisation** — Eigen uses SSE2 / AVX / AVX-512 intrinsics in its kernels. The compiler sometimes auto-vectorises your loops, but Eigen's hand-tuned kernels guarantee it.
- **BLAS dispatch** — for large-enough operands, Eigen forwards to OpenBLAS / MKL when available. You can verify by reading [Eigen's documentation](https://eigen.tuxfamily.org/dox/TopicUsingBlasLapack.html).

## Hints

<details>
<summary>What if my numbers don't match the expected story?</summary>

Some hardware surprises:

- **Apple Silicon (M-series)**: NumPy uses Apple's Accelerate framework, which is *very* fast. The NumPy / Eigen gap can vanish or even reverse for medium sizes.
- **Older CPUs without AVX**: the SIMD speedup is smaller (only SSE2 → 2× per double instead of AVX2 → 4×).
- **Codespaces / shared hosts**: noisy; the median over 5 runs may still wobble. Run twice and look at both.

If the qualitative story (Python ≪ NumPy, naive C++ ≪ Eigen, ijk < ikj at scale) is intact, you're done. The exact ratios are platform-dependent and the AI-Diary asks for *your* numbers, not the canonical ones.

</details>

<details>
<summary>Where does multi-threading fit in?</summary>

Eigen and BLAS implementations can use multiple threads for large matmuls. By default, Eigen uses one thread; BLAS often uses all cores. At `n = 1024`, OpenBLAS is multi-threaded by default and benefits from up to 4 cores roughly linearly. If your Codespace has 2 cores, the multi-thread scaling is bounded.

You can confirm by setting `OMP_NUM_THREADS=1` and re-running the NumPy benchmark; the time roughly doubles for large sizes. This is part of why NumPy is fast — but it's not the whole story; even single-threaded NumPy beats your loops by a wide margin.

</details>

→ Continue with [09 — Final comparison](09-final-comparison.md)
