# 8 — C++ vs Eigen

**Goal**: run the C++ benchmark at three optimisation levels, run Eigen's reference, and add the rows to your CSV.

## Run the benchmark

The repo ships a one-shot script that does the whole pipeline — three configures, three builds, three benchmark runs, and the final plot — so this isn't nine separate commands. From `cpp/`:

```powershell
# Windows (PowerShell)
./run-benchmarks.ps1                  # all three levels + plot
./run-benchmarks.ps1 -Levels O3       # only -O3 (~1-3 min) + plot
./run-benchmarks.ps1 -NoPlot          # all three, no plot
```

```bash
# Linux / macOS / WSL
./run-benchmarks.sh                   # all three levels + plot
./run-benchmarks.sh O3                # only -O3 (~1-3 min) + plot
./run-benchmarks.sh --no-plot         # all three, no plot
```

> ⚠️ **`-O0` is slow.** A full `-O0` sweep takes ~30–60 minutes on a typical laptop because the entrywise and (especially) columnwise variants don't get inlining or vectorisation. For an iterative workflow, run `-O3` only and add the others when you're ready to wait.

If you want to see what the script does (or run it manually), it's the equivalent of:

```bash
cd cpp
cmake -B build-O0 -DCMAKE_BUILD_TYPE=Debug          && cmake --build build-O0 --parallel --target benchmark
cmake -B build-O2 -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build-O2 --parallel --target benchmark
cmake -B build-O3 -DCMAKE_BUILD_TYPE=Release        && cmake --build build-O3 --parallel --target benchmark
./build-O0/benchmark > ../results/cpp-O0.csv
./build-O2/benchmark > ../results/cpp-O2.csv
./build-O3/benchmark > ../results/cpp-O3.csv
```

(The CSV is on stdout; progress messages go to stderr so the redirect stays clean. `--target benchmark` skips `test_matrix` and Catch2's `catch_discover_tests` post-step, which doesn't matter when you only want the benchmark binary.)

That gives you three CSVs alongside `results/python.csv`. The Python plot script picks them all up automatically.

## Measure peak memory

`std::chrono` measures time. Memory is measured by the OS — and the OS-specific command differs:

**Linux / WSL / macOS** — `/usr/bin/time -v` reports peak resident-set size:

```bash
/usr/bin/time -v ./build-O3/benchmark 2>&1 | grep "Maximum resident"
```

You'll see `Maximum resident set size (kbytes): N`.

**Windows (PowerShell)** — query `PeakWorkingSet64` on the process object:

```powershell
$p = Start-Process -FilePath ".\build-O3\benchmark.exe" `
                   -RedirectStandardOutput "..\results\cpp-O3.csv" `
                   -PassThru -NoNewWindow -Wait
"Peak working set: {0:N0} KB" -f ($p.PeakWorkingSet64 / 1KB)
```

Both numbers report the largest the binary's working set ever was, in KB. For a `1024×1024` matmul with three matrices of doubles the math is `3 × 1024² × 8 ≈ 24 MiB`. Eigen and the variants are within a factor of 2 of that. Compare to Python's `tracemalloc` numbers — Python's working set is usually 5–10× larger because of the per-element float boxing.

You don't need a separate run for each variant — the binary times all three and the OS reports the cumulative peak. If you care about per-variant peak, you can comment out two of the three blocks in `bench/benchmark.cpp` and re-run.

## Re-plot

```bash
cd ../python
uv run python bench/plot_results.py
```

`results/comparison.png` now has lines for everything. Look for:

- The Eigen line is below your `matmul_outerproduct` line at large sizes — that's BLAS dispatch + tiling + vectorisation.
- The `matmul_outerproduct` line is below `matmul_entrywise` by a factor of 5–10× at `n = 512` and beyond — that's cache locality (same flop count, friendlier loop order).
- The `matmul_columnwise` line is the slowest of your three hand-written variants — the per-iteration matrix allocations from `matvec` cost more than the cache wins of any decomposition.
- All your `-O3` C++ lines are 50–100× faster than your fastest Python at the same size.
- `numpy` and Eigen are within a small constant factor of each other — both call BLAS for large sizes.

## What to write down

Two things worth capturing somewhere ([`your_results.md`](../your_results.md) has Q2 and Q4 stubs, or use any notebook):

- `matmul_entrywise` vs `matmul_outerproduct` numbers at `n = 512` across `-O0`/`-O2`/`-O3`, plus the cache-line explanation in your own words. Same flop count, different memory-access pattern — that gap is the heart of chapter 7.
- Eigen vs your best `-O3` C++ at `n = 1024`, plus what Eigen does that you don't.

For the Eigen comparison, the things to mention:

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
- **Laptops on battery / shared hosts**: noisy; the median over 5 runs may still wobble. Plug in and run twice, or look at both runs.

If the qualitative story (Python ≪ NumPy, naive C++ ≪ Eigen, `matmul_entrywise` ≪ `matmul_outerproduct` at scale) is intact, you're done. The exact ratios are platform-dependent — *your* numbers are the right ones to record, not the canonical-looking values from these docs.

</details>

<details>
<summary>Where does multi-threading fit in?</summary>

Eigen and BLAS implementations can use multiple threads for large matmuls. By default, Eigen uses one thread; BLAS often uses all cores. At `n = 1024`, OpenBLAS is multi-threaded by default and benefits from up to 4 cores roughly linearly. If your machine has only 2 cores, the multi-thread scaling is bounded.

You can confirm by setting `OMP_NUM_THREADS=1` and re-running the NumPy benchmark; the time roughly doubles for large sizes. This is part of why NumPy is fast — but it's not the whole story; even single-threaded NumPy beats your loops by a wide margin.

</details>

→ Continue with [09 — Final comparison](09-final-comparison.md)
