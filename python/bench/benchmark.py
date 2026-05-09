"""
Benchmark the four Python matmul implementations:
    matmul_entrywise   — three nested loops
    matmul_columnwise  — column-by-column matrix-vector
    matmul_outerproduct — sum of rank-1 outer products
    matmul_numpy        — NumPy reference

Output: results/python.csv with columns
    variant,size,time_ms,peak_kb,runs

Notes:
- Median of `runs` measurements per cell. One warm-up run is discarded.
- Pure-Python variants are O(n^3) IN PYTHON, so the largest size you can
  afford in a few minutes is ~256. NumPy can scale to 1024 easily.
- Peak memory is measured per call via tracemalloc. NumPy's C-level
  buffers are not fully tracked by tracemalloc — read the printout with
  that caveat. For a fair memory comparison, look at the C++ numbers.
"""
from __future__ import annotations

import csv
import statistics
import sys
import time
import tracemalloc
from pathlib import Path

# Allow running this script as `python bench/benchmark.py` from python/
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from matrix import Matrix  # noqa: E402


# Sizes per variant. Pure-Python triple loops are slow — keep their max small.
SIZES = {
    "matmul_entrywise": [16, 32, 64, 128, 256],
    "matmul_columnwise": [16, 32, 64, 128, 256],
    "matmul_outerproduct": [16, 32, 64, 128, 256],
    "matmul_numpy": [16, 32, 64, 128, 256, 512, 1024],
}

RUNS = 5


def time_one_call(method, A: Matrix, B: Matrix) -> tuple[float, int]:
    """Return (elapsed_ms, peak_bytes) for a single call."""
    tracemalloc.start()
    t0 = time.perf_counter()
    method(B)
    t1 = time.perf_counter()
    _, peak = tracemalloc.get_traced_memory()
    tracemalloc.stop()
    return ((t1 - t0) * 1000.0, peak)


def benchmark_variant(method_name: str, sizes: list[int], runs: int) -> list[dict]:
    rows: list[dict] = []
    for n in sizes:
        A = Matrix.random(n, n, seed=11)
        B = Matrix.random(n, n, seed=22)
        method = getattr(A, method_name)

        # Warm-up: discard. CPython JIT is nonexistent but BLAS / module
        # imports / first-touch page faults all benefit from one extra call.
        time_one_call(method, A, B)

        times: list[float] = []
        peaks: list[int] = []
        for _ in range(runs):
            t_ms, peak_bytes = time_one_call(method, A, B)
            times.append(t_ms)
            peaks.append(peak_bytes)

        rows.append(
            {
                "variant": method_name,
                "size": n,
                "time_ms": statistics.median(times),
                "peak_kb": statistics.median(peaks) // 1024,
                "runs": runs,
            }
        )
        print(
            f"  {method_name:>22s}  n={n:>4d}  "
            f"time={statistics.median(times):>10.2f} ms  "
            f"peak={statistics.median(peaks) // 1024:>6d} kB"
        )
    return rows


def main() -> None:
    here = Path(__file__).resolve().parent
    results_dir = here.parent.parent / "results"
    results_dir.mkdir(exist_ok=True)
    out = results_dir / "python.csv"

    all_rows: list[dict] = []
    for method_name, sizes in SIZES.items():
        print(f"\n[{method_name}]")
        try:
            all_rows.extend(benchmark_variant(method_name, sizes, RUNS))
        except NotImplementedError as e:
            print(f"  SKIP (not implemented yet): {e}")

    with out.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["variant", "size", "time_ms", "peak_kb", "runs"])
        writer.writeheader()
        for row in all_rows:
            writer.writerow(row)
    print(f"\nWrote {len(all_rows)} rows to {out}")


if __name__ == "__main__":
    main()
