# Build Your Own Matrix Class — Python and C++ Deep Dive

> Optional / advanced deep-dive for Lecture 5. 1–2 weeks. No graded deliverable — the goal is the *aha moments*, not points.

You have used `np.matmul` happily for several notebooks. This assignment forces you to rebuild the part of NumPy you depended on — first in Python, then in C++ — and to measure exactly how much engineering NumPy and Eigen have done on your behalf.

## What you build

1. A `Matrix` class — flat row-major storage, slicing, transpose, equality, vectors as 1×n / n×1 special cases.
2. Three matmul implementations (per language) — entry-wise, column-by-column, outer-product sum. They compute the same thing three ways. The lecture's [linear algebra companion exercises](https://github.com/L3GJ0N/ai_supported_software_development_hs_aalen/blob/main/prompts/lecture-ideas/lecture_5/exercises/linear_algebra_for_transformers_exercises.md) Exercise 6.4 proves they are equivalent. You verify it experimentally.
3. Unit tests that catch your bugs — first time using `pytest` and `Catch2` in this course.
4. Benchmarks — runtime via `time.perf_counter` / `std::chrono`, peak memory via `tracemalloc` (Python) and `/usr/bin/time -v` on Linux/macOS or `PeakWorkingSet64` on Windows (C++). First time measuring performance.
5. A comparison plot showing pure Python vs NumPy vs naive C++ vs `-O3` C++ vs Eigen — five lines on log–log axes.

## How to start

1. **Click the GitHub Classroom invitation link** (provided on the course site) and clone the repo locally.
2. **Set up your machine** by following the OS-specific guide once — 10–15 minutes:
   - [Windows native setup](docs/setup-windows.md) (MSYS2 + mingw-w64, or Visual Studio Build Tools)
   - [macOS native setup](docs/setup-macos.md) (Homebrew + Apple Clang, Apple Silicon or Intel)
   - [Ubuntu / Linux native setup](docs/setup-ubuntu.md) (also covers WSL2)

   You already have Python and `uv` from earlier in the course; the guides only add a C++ toolchain (g++/MSVC, CMake, Ninja). Eigen and Catch2 are fetched automatically by CMake — you don't install them yourself.

   Don't want to install anything? The repo also ships a [Devcontainer](.devcontainer/devcontainer.json) as a fallback (requires Docker or a GitHub Codespace). It is **not** the recommended path — use it only if a native install gives you trouble.
3. **Read [`docs/00-getting-started.md`](docs/00-getting-started.md)** and follow the chapters in order.

## Repository layout

```
linalg-from-scratch/
├── README.md                this file
├── your_results.md          optional — record your benchmark numbers and observations here
├── docs/                    chapter-by-chapter guide (start here)
├── python/                  Part A — Python skeleton + tests + benchmarks
├── cpp/                     Part B — C++ skeleton + tests + benchmarks
├── results/                 your benchmark CSVs and plots land here
├── .devcontainer/           optional Docker-based fallback environment
└── .github/workflows/       CI — runs your tests on every push
```

## Working through it

There is no graded deliverable. Tests should pass and code should compile, but there is no autograded score — the goal is the *aha moments*, not points. The chapters keep pointing at numbers worth noticing (Python vs NumPy, C++ entrywise vs outerproduct cache locality, Eigen vs your `-O3` C++); writing them down in one place is the single best way to make those observations stick. [`your_results.md`](your_results.md) is an optional template if you want a place to do that — or use a notebook, or anywhere else.

## Help

- The chapters in [`docs/`](docs/) are written to be self-contained.
- Each chapter has explicit hints and references back to the lecture.
- AI assistance (Copilot, Claude, Cursor) is encouraged. The C++ part especially is short on ceremony if you let an assistant handle syntax.
- The original course-wide hint applies: if you are stuck on C++ for more than 20 minutes on a syntax question, ask the assistant. This is not a C++ language test.

## License

Course materials. Use freely for learning. Attribution appreciated if you reuse the structure.
