# Build Your Own Matrix Class — Python and C++ Deep Dive

> Optional / advanced deep-dive for Lecture 5. 1–2 weeks. Reflection-only deliverable (no graded points).

You have used `np.matmul` happily for several notebooks. This assignment forces you to rebuild the part of NumPy you depended on — first in Python, then in C++ — and to measure exactly how much engineering NumPy and Eigen have done on your behalf.

## What you build

1. A `Matrix` class — flat row-major storage, slicing, transpose, equality, vectors as 1×n / n×1 special cases.
2. Three matmul implementations (per language) — entry-wise, column-by-column, outer-product sum. They compute the same thing three ways. The lecture's [linear algebra companion exercises](https://github.com/L3GJ0N/ai_supported_software_development_hs_aalen/blob/main/prompts/lecture-ideas/lecture_5/exercises/linear_algebra_for_transformers_exercises.md) Exercise 6.4 proves they are equivalent. You verify it experimentally.
3. Unit tests that catch your bugs — first time using `pytest` and `Catch2` in this course.
4. Benchmarks — runtime via `time.perf_counter` / `std::chrono`, peak memory via `tracemalloc` / `/usr/bin/time -v`. First time measuring performance.
5. A comparison plot showing pure Python vs NumPy vs naive C++ vs `-O3` C++ vs Eigen — five lines on log–log axes.

## How to start

1. **Click the GitHub Classroom invitation link** (provided on the course site).
2. **Open the repo in a Codespace** (recommended) or **open in VS Code locally** and choose "Reopen in Container" when prompted. The Devcontainer pre-installs Python, `uv`, g++, CMake, Eigen, and Catch2 — you don't install anything yourself.
3. **Read [`docs/00-getting-started.md`](docs/00-getting-started.md)** and follow the chapters in order.

## Repository layout

```
linalg-from-scratch/
├── README.md                this file
├── ai_diary.md              your D5 reflection — fill it in
├── docs/                    chapter-by-chapter guide (start here)
├── python/                  Part A — Python skeleton + tests + benchmarks
├── cpp/                     Part B — C++ skeleton + tests + benchmarks
├── results/                 your benchmark CSVs and plots land here
├── .devcontainer/           pre-configured environment (auto-loaded)
└── .github/workflows/       CI — runs your tests on every push
```

## Deliverable

The single deliverable is the AI Diary entry in `ai_diary.md`. You answer five questions about what you measured and what surprised you. The benchmark plot you produce is part of that reflection. Your tests should pass and your code should compile, but there is no autograded score — the goal is the *aha moments*, not points.

## Help

- The chapters in [`docs/`](docs/) are written to be self-contained.
- Each chapter has explicit hints and references back to the lecture.
- AI assistance (Copilot, Claude, Cursor) is encouraged. The C++ part especially is short on ceremony if you let an assistant handle syntax.
- The original course-wide hint applies: if you are stuck on C++ for more than 20 minutes on a syntax question, ask the assistant. This is not a C++ language test.

## License

Course materials. Use freely for learning. Attribution appreciated if you reuse the structure.
