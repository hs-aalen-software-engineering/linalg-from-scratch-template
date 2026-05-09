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
ai_diary.md           the deliverable
```

## Open in the Devcontainer

The repo ships with a Devcontainer that pre-installs Python, `uv`, g++, CMake, Eigen, and Catch2. You don't need to install anything yourself.

**In a Codespace** (recommended, free tier covers this):
1. On the repo's GitHub page, press `.` (dot) to open the web editor, then `Ctrl/Cmd+Shift+P` → "Codespaces: Create codespace on main".
2. The Devcontainer builds automatically. Wait for the build to finish (≈ 3 min the first time).

**Locally** (VS Code with the *Dev Containers* extension):
1. Clone the repo.
2. Open the folder in VS Code.
3. When VS Code prompts "Reopen in Container", say yes. (If it doesn't, `Ctrl+Shift+P` → "Dev Containers: Reopen in Container".)

You're set up correctly when:

```bash
cd python && uv sync && uv run pytest -v
```

runs and shows a long list of FAILs (because you haven't filled in the TODOs yet) and:

```bash
cd cpp && cmake -B build -S . && cmake --build build && ctest --test-dir build
```

builds successfully and shows tests failing for the same reason. Failing tests this early is good — it means everything compiles.

## How to work

Go through chapters 1–9 in order. Each chapter is short. The Python half (chapters 1–5) is the bigger time investment because it's where you build the class. The C++ half (chapters 6–8) reuses your Python design — the structure is familiar, only the syntax is new.

You do not need to be a C++ expert. The skeleton is heavy on scaffolding and the TODOs are small. Use AI assistance for syntax — that is part of the point of this course.

## When you're stuck

- The chapters are self-contained and link directly to what you need.
- The lecture's chapter 7 explains attention and matmul; the linear algebra companion explains the three views.
- Ask the assistant for syntax help. Don't burn 30 minutes on a `std::` typo.
- Ask in the course's discussion channel if the issue is conceptual.

→ Continue with [01 — Python Matrix class](01-python-matrix-class.md)
