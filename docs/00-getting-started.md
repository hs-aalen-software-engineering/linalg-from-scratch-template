# 0 — Getting started

Welcome. This chapter is a five-minute warm-up that gets your environment running.

## What this exercise is

You will rebuild the parts of NumPy and Eigen that you have been using for free — first in Python, then in C++. Three matmul variants in each language. Unit tests. Benchmarks. One plot. Five reflection questions. That's it.

The point is the *gap* you measure between your code and the libraries. Anyone who tells you "NumPy is fast because it's in C" hasn't been specific enough. By the end of this assignment you will be able to list the four or five engineering choices that make the gap real.

## What's in the repo

```
docs/                 you are here — work through chapters in order
python/matrix/        Matrix class skeleton with TODOs (Part A)
python/tests/         pytest tests (some pre-written, some you'll add)
python/bench/         benchmark + plot scripts (complete)
cpp/include/          Matrix.hpp skeleton with TODOs (Part B)
cpp/tests/            Catch2 tests
cpp/bench/            chrono-based benchmark
results/              your CSVs and plots land here
your_results.md       optional template for recording your benchmark numbers
```

## Set up your environment

You already have Python and `uv` from earlier in the course. The C++ half needs a compiler, CMake, and Ninja. Install them natively — there's no container to wait for and no Docker daemon to keep running.

Pick the guide for your OS and follow it once:

- [**Windows native setup**](setup-windows.md) — MSYS2 + mingw-w64 (recommended) or Visual Studio Build Tools.
- [**Ubuntu / Linux native setup**](setup-ubuntu.md) — `apt install build-essential cmake ninja-build`. Same guide covers WSL2.

Both guides take 10–15 minutes and end with the same verification commands.

You're set up correctly when, from the repo root:

```bash
cd python && uv sync && uv run pytest -v
```

runs and shows a long list of FAILs (because you haven't filled in the TODOs yet) and:

```bash
cd cpp && cmake -B build -S . && cmake --build build && ctest --test-dir build
```

builds successfully and shows tests failing for the same reason. Failing tests this early is good — it means everything compiles.

> **Devcontainer fallback.** The repo still ships a [Devcontainer](../.devcontainer/devcontainer.json) with everything pre-installed. It needs Docker (or a GitHub Codespace). Use it only if the native setup didn't work for you — see the *Fallback* section at the bottom of either OS guide.

## How to work

Go through chapters 1–9 in order. Each chapter is short. The Python half (chapters 1–5) is the bigger time investment because it's where you build the class. The C++ half (chapters 6–8) reuses your Python design — the structure is familiar, only the syntax is new.

You do not need to be a C++ expert. The skeleton is heavy on scaffolding and the TODOs are small. Use AI assistance for syntax — that is part of the point of this course.

## When you're stuck

- The chapters are self-contained and link directly to what you need.
- The lecture's chapter 7 explains attention and matmul; the linear algebra companion explains the three views.
- Ask the assistant for syntax help. Don't burn 30 minutes on a `std::` typo.
- Ask in the course's discussion channel if the issue is conceptual.

→ Continue with [01 — Python Matrix class](01-python-matrix-class.md)
