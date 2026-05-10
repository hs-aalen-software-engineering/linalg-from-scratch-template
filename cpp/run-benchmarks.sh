#!/usr/bin/env bash
# Configure, build, and run the C++ benchmark at one or more optimisation
# levels, then (optionally) regenerate the comparison plot.
#
# For each level in $LEVELS, this script:
#   1. cmake -B build-$LEVEL -DCMAKE_BUILD_TYPE=<mapping>
#   2. cmake --build build-$LEVEL --parallel --target benchmark
#   3. ./build-$LEVEL/benchmark > ../results/cpp-$LEVEL.csv
#
# After all benchmarks run, it calls Python's plot_results.py via uv
# (unless --no-plot is given). The plot script picks up every CSV in
# results/, so cpp-$LEVEL.csv lines appear automatically alongside
# python.csv.
#
# Heads up: -O0 is slow (estimate 30-60 minutes for a full size sweep
# because the un-optimised entrywise/columnwise variants don't get
# inlining or vectorisation). Pass O3 alone for a quick check.
#
# Usage:
#   ./run-benchmarks.sh                  # all three levels + plot
#   ./run-benchmarks.sh O3               # -O3 only + plot (~1-3 minutes)
#   ./run-benchmarks.sh O2 O3            # -O2 and -O3 + plot
#   ./run-benchmarks.sh --no-plot        # all three, skip the plot
#   ./run-benchmarks.sh O3 --no-plot     # combinable

set -euo pipefail

# Map level -> CMAKE_BUILD_TYPE
declare -A BUILD_TYPES=(
  [O0]="Debug"
  [O2]="RelWithDebInfo"
  [O3]="Release"
)

# Parse: positional args are levels, --no-plot is the only flag.
LEVELS=()
PLOT=1
for arg in "$@"; do
  case "$arg" in
    --no-plot)
      PLOT=0
      ;;
    O0|O2|O3)
      LEVELS+=("$arg")
      ;;
    -h|--help)
      sed -n '2,/^set -/p' "$0" | sed 's/^# \?//; $d'
      exit 0
      ;;
    *)
      echo "Unknown argument: $arg" >&2
      echo "Usage: $0 [O0|O2|O3 ...] [--no-plot]" >&2
      exit 1
      ;;
  esac
done

# Default: all three.
if [ ${#LEVELS[@]} -eq 0 ]; then
  LEVELS=(O0 O2 O3)
fi

# Always run from cpp/ (the script's own directory) regardless of CWD.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"
RESULTS_DIR="$(cd "$SCRIPT_DIR/.." && pwd)/results"
mkdir -p "$RESULTS_DIR"

for LEVEL in "${LEVELS[@]}"; do
  TYPE="${BUILD_TYPES[$LEVEL]}"
  BUILD_DIR="build-$LEVEL"
  CSV="$RESULTS_DIR/cpp-$LEVEL.csv"

  printf '\n=== [%s] Configure -> %s (CMAKE_BUILD_TYPE=%s) ===\n' "$LEVEL" "$BUILD_DIR" "$TYPE"
  cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$TYPE"

  printf '\n=== [%s] Build %s (target: benchmark) ===\n' "$LEVEL" "$BUILD_DIR"
  # --target benchmark skips test_matrix and its catch_discover_tests
  # post-step, which only matters when running tests, not benchmarks.
  cmake --build "$BUILD_DIR" --parallel --target benchmark

  printf '\n=== [%s] Run benchmark -> %s ===\n' "$LEVEL" "$CSV"
  "./$BUILD_DIR/benchmark" > "$CSV"
  echo "Wrote $CSV"
done

if [ "$PLOT" -eq 1 ]; then
  printf '\n=== Plot ===\n'
  cd "$SCRIPT_DIR/../python"
  uv run python bench/plot_results.py
fi

printf '\nDone.\n'
