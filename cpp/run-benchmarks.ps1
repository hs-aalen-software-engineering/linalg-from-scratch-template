<#
.SYNOPSIS
  Configure, build, and run the C++ benchmark at one or more optimisation
  levels, then (optionally) regenerate the comparison plot.

.DESCRIPTION
  For each level in -Levels, this script:
    1. cmake -B build-OX -DCMAKE_BUILD_TYPE=<mapping>
    2. cmake --build build-OX --parallel --target benchmark
    3. ./build-OX/benchmark.exe > ../results/cpp-OX.csv

  After all benchmarks run, it calls Python's plot_results.py via uv
  (unless -NoPlot is given). The plot script picks up every CSV in
  results/, so cpp-OX.csv lines appear automatically alongside python.csv.

  Heads up: -O0 is slow (estimate 30-60 minutes for a full size sweep
  because the un-optimised entrywise/columnwise variants don't get
  inlining or vectorisation). Use -Levels O3 for a quick check.

.PARAMETER Levels
  Which optimisation levels to run. Default: all three (O0, O2, O3).
  Mapping to CMAKE_BUILD_TYPE: O0 -> Debug, O2 -> RelWithDebInfo, O3 -> Release.

.PARAMETER NoPlot
  Skip the final plot_results.py step.

.EXAMPLE
  ./run-benchmarks.ps1
  Run all three levels (O0, O2, O3) and regenerate the plot.

.EXAMPLE
  ./run-benchmarks.ps1 -Levels O3
  Only -O3 (~1-3 minutes), then plot.

.EXAMPLE
  ./run-benchmarks.ps1 -Levels O2,O3 -NoPlot
  Skip the slow O0 build and don't run the plot script.
#>
param(
  [ValidateSet("O0", "O2", "O3")]
  [string[]]$Levels = @("O0", "O2", "O3"),

  [switch]$NoPlot
)

$ErrorActionPreference = "Stop"

# Map each level to the CMake build type that produces it.
$buildTypes = @{
  "O0" = "Debug"
  "O2" = "RelWithDebInfo"
  "O3" = "Release"
}

# Always run from cpp/ (the script's own directory) regardless of CWD.
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $scriptDir
try {
  $resultsDir = Resolve-Path (Join-Path $scriptDir "..\results")

  foreach ($level in $Levels) {
    $type = $buildTypes[$level]
    $buildDir = "build-$level"
    $csv = Join-Path $resultsDir "cpp-$level.csv"

    Write-Host "`n=== [$level] Configure -> $buildDir (CMAKE_BUILD_TYPE=$type) ===" -ForegroundColor Cyan
    # Quote the -D argument explicitly. Without quotes, PowerShell mangles
    # `-DCMAKE_BUILD_TYPE=$type` when passing to native commands and CMake
    # ends up emitting a multi-config-style rules.ninja with literal `$type`
    # placeholders that single-config Ninja can't parse.
    cmake -B $buildDir "-DCMAKE_BUILD_TYPE=$type"
    if ($LASTEXITCODE -ne 0) {
      throw "cmake configure (level $level) exited with code $LASTEXITCODE"
    }

    Write-Host "`n=== [$level] Build $buildDir (target: benchmark) ===" -ForegroundColor Cyan
    # --target benchmark skips test_matrix and its catch_discover_tests
    # post-step, which only matters when running tests, not benchmarks.
    cmake --build $buildDir --parallel --target benchmark
    if ($LASTEXITCODE -ne 0) {
      throw "cmake --build (level $level) exited with code $LASTEXITCODE"
    }

    Write-Host "`n=== [$level] Run benchmark -> $csv ===" -ForegroundColor Cyan
    & ".\$buildDir\benchmark.exe" > $csv
    if ($LASTEXITCODE -ne 0) {
      throw "benchmark.exe (level $level) exited with code $LASTEXITCODE"
    }
    Write-Host "Wrote $csv" -ForegroundColor Green
  }

  if (-not $NoPlot) {
    Write-Host "`n=== Plot ===" -ForegroundColor Cyan
    Push-Location (Join-Path $scriptDir "..\python")
    try {
      uv run python bench/plot_results.py
    } finally {
      Pop-Location
    }
  }
} finally {
  Pop-Location
}

Write-Host "`nDone." -ForegroundColor Green
