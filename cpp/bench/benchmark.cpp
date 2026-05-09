// Benchmark the three C++ matmul variants (ijk, ikj, eigen).
//
// Run from cpp/:
//   cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
//   ./build/benchmark > ../results/cpp.csv
//
// To compare optimisation levels, build into separate directories:
//   cmake -B build-O0 -DCMAKE_BUILD_TYPE=Debug          && cmake --build build-O0
//   cmake -B build-O2 -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build-O2
//   cmake -B build-O3 -DCMAKE_BUILD_TYPE=Release        && cmake --build build-O3
// then run each `./build-OX/benchmark` and append the rows.
//
// CSV columns (matching the Python format so plot_results.py can merge):
//   variant,size,time_ms,peak_kb,runs
// Memory: this program does not measure peak RSS itself. Run the binary under
// `/usr/bin/time -v` to get the peak. The CSV reports peak_kb as 0.

#include "matrix.hpp"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

#ifndef BENCH_BUILD_TYPE
#define BENCH_BUILD_TYPE "unknown"
#endif

double median(std::vector<double> v) {
  std::sort(v.begin(), v.end());
  return v[v.size() / 2];
}

template <typename Method>
double time_call_ms(const Matrix& A, const Matrix& B, Method method) {
  auto t0 = std::chrono::high_resolution_clock::now();
  Matrix C = (A.*method)(B);
  auto t1 = std::chrono::high_resolution_clock::now();
  // touch a value so the optimiser cannot eliminate the call
  volatile double sink = C(0, 0);
  (void)sink;
  return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

template <typename Method>
double bench(const std::string& name, std::size_t n, Method method, int runs) {
  auto A = Matrix::random(n, n, 11);
  auto B = Matrix::random(n, n, 22);

  // warm-up
  time_call_ms(A, B, method);

  std::vector<double> times;
  times.reserve(runs);
  for (int r = 0; r < runs; ++r) {
    times.push_back(time_call_ms(A, B, method));
  }
  const double t = median(std::move(times));
  // CSV row to stdout
  std::cout << name << "_" << BENCH_BUILD_TYPE << "," << n << "," << std::fixed
            << std::setprecision(3) << t << ",0," << runs << "\n";
  // Human echo to stderr so a tee'd CSV stays clean
  std::cerr << "  " << std::setw(20) << name << "  n=" << std::setw(5) << n
            << "  time=" << std::setw(10) << t << " ms\n";
  return t;
}

}  // namespace

int main() {
  // CSV header
  std::cout << "variant,size,time_ms,peak_kb,runs\n";
  std::cerr << "Build type: " << BENCH_BUILD_TYPE << "\n";

  const std::vector<std::size_t> sizes = {16, 32, 64, 128, 256, 512, 1024};
  const int runs = 5;

  std::cerr << "[matmul_ijk]\n";
  for (auto n : sizes) {
    bench("matmul_ijk", n, &Matrix::matmul_ijk, runs);
  }

  std::cerr << "[matmul_ikj]\n";
  for (auto n : sizes) {
    bench("matmul_ikj", n, &Matrix::matmul_ikj, runs);
  }

  std::cerr << "[matmul_eigen]\n";
  for (auto n : sizes) {
    bench("matmul_eigen", n, &Matrix::matmul_eigen, runs);
  }

  return 0;
}
