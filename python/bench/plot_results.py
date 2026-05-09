"""
Plot benchmark results from results/python.csv (and optionally results/cpp.csv).

Produces results/comparison.png with one line per variant on log-log axes.

Run from the python/ directory:
    uv run python bench/plot_results.py

Reads any CSV files matching results/*.csv and merges them. Each CSV must have
columns: variant,size,time_ms,peak_kb,runs.
"""
from __future__ import annotations

import csv
import sys
from pathlib import Path

import matplotlib.pyplot as plt


def load_csvs(results_dir: Path) -> list[dict]:
    rows: list[dict] = []
    for csv_path in sorted(results_dir.glob("*.csv")):
        with csv_path.open() as f:
            reader = csv.DictReader(f)
            for r in reader:
                r["size"] = int(r["size"])
                r["time_ms"] = float(r["time_ms"])
                r["peak_kb"] = int(r["peak_kb"])
                r["source"] = csv_path.stem
                rows.append(r)
    return rows


def plot(rows: list[dict], out_png: Path) -> None:
    if not rows:
        print("No rows to plot.", file=sys.stderr)
        return

    # Group by (source, variant). Each group is one line on the chart.
    groups: dict[tuple[str, str], list[tuple[int, float]]] = {}
    for r in rows:
        key = (r["source"], r["variant"])
        groups.setdefault(key, []).append((r["size"], r["time_ms"]))

    plt.figure(figsize=(9, 6))
    for (source, variant), points in sorted(groups.items()):
        points.sort()
        xs = [p[0] for p in points]
        ys = [p[1] for p in points]
        label = f"{source}:{variant}"
        plt.loglog(xs, ys, marker="o", label=label)

    plt.xlabel("matrix size n (n × n × n)")
    plt.ylabel("median time (ms)")
    plt.title("Matrix multiplication — runtime vs size")
    plt.grid(True, which="both", alpha=0.3)
    plt.legend(fontsize=8, loc="best")
    plt.tight_layout()
    plt.savefig(out_png, dpi=140)
    print(f"Wrote {out_png}")


def main() -> None:
    here = Path(__file__).resolve().parent
    results_dir = here.parent.parent / "results"
    rows = load_csvs(results_dir)
    print(f"Loaded {len(rows)} rows from {results_dir}")
    plot(rows, results_dir / "comparison.png")


if __name__ == "__main__":
    main()
