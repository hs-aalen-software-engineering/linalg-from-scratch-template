# 5b — Building and testing C++ projects: a primer

**Read this before chapter 6.** The Python half assumed we knew `pip` (or here, `uv`) and `pytest`. The C++ half drops us into CMake, Ninja, FetchContent, and Catch2 — four tools most of us have not seen yet. This chapter explains what each one does, why we need it, and what the commands in chapters 6–9 actually do under the hood. There are no exercises here; once we're through, the build output of chapters 6–9 should read as something familiar rather than something to copy-paste.

If you've already built a multi-file C++ project before — `make`, CMake at school, or anything similar — feel free to skim. The walk-through of *this project's* `CMakeLists.txt` near the end is the part worth reading even then.

## Why this chapter exists

Until this course, most of us have only compiled this kind of C++ project:

```bash
g++ hello.cpp -o hello
./hello
```

One source file. One command. No configuration. That's fine for a single `main`, but it stops scaling the moment a project has:

- More than one `.cpp` file.
- An external library nobody on the team wrote (Eigen, Catch2, …).
- Multiple compile flags for different scenarios (debug vs. release, with vs. without optimisation).
- A test suite separate from the binary that ships.

This project has all four. Without a build system we'd be typing 30-line `g++` invocations by hand, copy-pasting include paths, and rebuilding the world every time someone changes a comment. So the C++ ecosystem invented build systems. This chapter is our tour of the ones we use.

## The 30-second mental model

```
                    cpp/CMakeLists.txt        (you write this — high-level recipe)
                              │
                              │ cmake -B build -S . -G Ninja
                              ▼
                    cpp/build/build.ninja     (CMake generates this — low-level recipe)
                              │
                              │ cmake --build build      (≡ ninja -C build)
                              ▼
                    cpp/build/test_matrix     (the actual binaries)
                              │
                              │ ctest --test-dir build
                              ▼
                       PASS / FAIL output
```

Three tools, one pipeline:

1. **CMake** reads your `CMakeLists.txt` and *generates* a build plan. It does not compile anything itself.
2. **Ninja** (or Make, or MSBuild) executes that plan: invokes `g++`, links the binaries, and tracks dependencies so the second build is fast.
3. **CTest** is CMake's test runner — it knows which binaries are tests and reports pass/fail.

We drive all three through `cmake ...` commands. Most of the time we never type `ninja` or `ctest` directly, even though they're the tools doing the work.

## Make, the original build automator

Skip this section if you've used `make` before — go straight to [CMake](#cmake-is-a-meta-build-system).

`make` (1976) is the great-grandparent of every build tool we use today. You write a `Makefile` listing **rules**:

```makefile
hello: hello.cpp
	g++ hello.cpp -o hello
```

A rule has three parts: a *target* (`hello`), its *dependencies* (`hello.cpp`), and the *recipe* to build the target (`g++ hello.cpp -o hello`). When you type `make hello`, it checks whether `hello.cpp` is newer than `hello`. If yes, it runs the recipe. If no, it does nothing — your binary is already up to date. That **incremental** model is the whole point of a build system.

Real Makefiles for non-trivial projects get long fast: every `.cpp` becomes its own rule, we'd list every header dependency by hand, and cross-platform support (`gcc` on Linux, `clang` on macOS, `cl.exe` on Windows) is brittle. That's where CMake comes in.

## CMake is a *meta* build system

CMake (2000) is one level of indirection above Make. We don't write `Makefile`s; we write a `CMakeLists.txt` that describes the project at a higher level:

> "This project produces a binary called `test_matrix` from `tests/test_matrix.cpp`. It needs the C++17 standard. It links against Catch2 and Eigen."

CMake then *generates* the actual build files. It can generate a `Makefile` for `make`, a `build.ninja` for Ninja, a Visual Studio solution for MSVC, or an Xcode project for Apple. You don't change your `CMakeLists.txt` to switch between them — you change one flag.

```bash
cmake -B build -S . -G Ninja          # generate Ninja files
cmake -B build -S . -G "Unix Makefiles" # generate a Makefile
cmake -B build -S . -G "Visual Studio 17 2022"  # generate a VS solution
```

Same source, different generated output. That's what we mean by "meta build system": CMake itself doesn't compile anything — it teaches another tool how to compile.

We don't change `CMakeLists.txt` to switch between Make / Ninja / VS — we change one flag on the command line.

## Ninja, a faster Make

Ninja (2011) is a build executor like Make, designed to be **fast** and **dumb**. Where a Makefile invites you to write logic and macros, a `build.ninja` file is a flat list of "to make X, run command Y; X depends on Z." It's not really meant to be written by hand — CMake (or another generator) writes it for you, and Ninja executes it in parallel as fast as your CPU allows.

Why prefer Ninja over Make? On a project this size, the difference is small. On a large codebase, Ninja can be 2–3× faster at incremental builds because its dependency tracking is more efficient. We chose Ninja because it's the modern default, it's identical on Windows and Linux, and most students have never seen it — so this is a chance to learn it.

We can verify that `cmake --build build` is calling Ninja under the hood by passing `--verbose`:

```bash
cmake --build build --verbose    # prints every g++ invocation
```

The output is `g++ -O3 -std=c++17 -Iinclude ... tests/test_matrix.cpp -o test_matrix`. CMake assembled that command from our `CMakeLists.txt`; Ninja just ran it.

## The two-step CMake flow

Every CMake-based project goes through the same two phases. Knowing which phase we're in saves a lot of time when something fails.

### Phase 1 — Configure: `cmake -B build -S . -G Ninja`

- `-S .` says "the source tree (with `CMakeLists.txt`) is here, the current directory."
- `-B build` says "put all generated files into `./build/`."
- `-G Ninja` says "generate Ninja files, not Makefiles."

What CMake does during configure:

1. Detects the compiler (`g++`, `clang++`, `cl.exe`) and tests that it works.
2. Checks for required features (does the compiler support C++17?).
3. Resolves dependencies (`find_package`, `FetchContent` — see below).
4. Walks our `CMakeLists.txt` and writes `build/build.ninja`.
5. Caches everything in `build/CMakeCache.txt` so the next configure is fast.

The first configure of this project takes 1–2 minutes because Catch2 (and possibly Eigen) is downloaded. Subsequent configures are instant.

### Phase 2 — Build: `cmake --build build`

This is just a portable wrapper for "run the generator." For us it's equivalent to:

```bash
ninja -C build
```

CMake invokes Ninja, which reads `build/build.ninja`, figures out what's stale, and runs the necessary `g++` commands in parallel. After the first full build, an incremental rebuild (after editing one file) is usually under a second.

### Phase 3 — Test: `ctest --test-dir build`

`ctest` is shipped with CMake. It looks at `build/CTestTestfile.cmake` (also generated during configure) to find all the test executables and runs them in sequence, reporting pass/fail.

```bash
ctest --test-dir build --output-on-failure       # run all tests, print stdout for failures
ctest --test-dir build -R basic                  # only tests matching "basic"
ctest --test-dir build --rerun-failed            # re-run last failures only
```

We can also run a test binary directly: `./build/test_matrix` produces the same output as Catch2 reports it. `ctest` adds parallelism and result aggregation.

## Walking through *this project's* `CMakeLists.txt`

Open [`cpp/CMakeLists.txt`](../cpp/CMakeLists.txt) alongside this section. Lines below are paraphrased; the file you're reading is authoritative.

```cmake
cmake_minimum_required(VERSION 3.20)
project(linalg_from_scratch CXX)
```

The minimum CMake version we depend on, and the project name. `CXX` says "this is a C++ project" (no C, no Fortran). If you have an older CMake, you'll get an error here.

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

C++17, mandatory, no GCC-specific extensions. This means CMake will pass `-std=c++17` (not `-std=gnu++17`) to the compiler.

```cmake
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
```

Default to a Release (i.e., `-O3`) build if the user didn't pick one. Without this, single-config generators like Ninja default to "no build type at all" — meaning no optimisation, but also no debug symbols. Our chapter 7 deliberately switches between `Debug`, `RelWithDebInfo`, and `Release` to compare.

```cmake
include(FetchContent)
```

Loads CMake's built-in module for downloading dependencies. See the next section.

```cmake
FetchContent_Declare(Catch2 GIT_REPOSITORY ... GIT_TAG v3.5.4)
FetchContent_MakeAvailable(Catch2)
```

Downloads Catch2 from GitHub and makes its CMake targets available to us. After this line, `Catch2::Catch2WithMain` is a target we can link against.

```cmake
find_package(Eigen3 3.4 QUIET NO_MODULE)
if(NOT Eigen3_FOUND)
  ... FetchContent_Declare(Eigen3 ...) ...
endif()
```

For Eigen, we *prefer* a system install (on Ubuntu, `apt install libeigen3-dev` puts it where `find_package` can find it). If the system doesn't have it (Windows, minimal Linux setups), we fall back to fetching it from GitLab.

This "find first, fetch as fallback" pattern is good practice: faster configures when the dep is already there, but the project still works on a clean machine.

```cmake
add_library(matrix INTERFACE)
target_include_directories(matrix INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(matrix INTERFACE Eigen3::Eigen)
target_compile_features(matrix INTERFACE cxx_std_17)
```

Defines a virtual library called `matrix`. `INTERFACE` means "header-only — there's no `.cpp` to compile, only headers to include." Anything that links against `matrix` automatically gets:
- `cpp/include/` on its include path (so `#include "matrix.hpp"` works),
- Eigen on its include path,
- C++17 enabled.

```cmake
enable_testing()
add_executable(test_matrix tests/test_matrix.cpp)
target_link_libraries(test_matrix PRIVATE matrix Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(test_matrix)
```

The test executable. `tests/test_matrix.cpp` is compiled and linked against:
- `matrix` (so it can `#include "matrix.hpp"`),
- `Catch2::Catch2WithMain` — Catch2's `main()`, so we don't write our own.

`catch_discover_tests` is the magic that lets `ctest` see every individual `TEST_CASE(...)` in the file as a separate ctest entry. Without it, `ctest` would see one giant "test_matrix" binary and report a single pass/fail.

```cmake
add_executable(benchmark bench/benchmark.cpp)
target_link_libraries(benchmark PRIVATE matrix)
target_compile_definitions(benchmark PRIVATE BENCH_BUILD_TYPE="${CMAKE_BUILD_TYPE}")
```

The benchmark binary. The compile definition is what lets the benchmark print which optimisation level it was built at — useful when comparing `cpp-O0.csv` to `cpp-O3.csv`.

That's the whole file. ~50 lines, and it produces a working test suite, a benchmark, two external dependencies resolved, and three build configurations on demand. The same project written as raw `g++` commands would be hundreds of lines of shell scripting.

## External dependencies — `find_package` vs `FetchContent`

Two patterns appear in this project, and they're the two patterns we'll see everywhere:

**`find_package(SomeLib)`** asks: "Is `SomeLib` already installed somewhere CMake knows about?" It checks system paths, environment variables, and a few CMake-specific search locations. Fast (no download), but requires the library to be installed on the user's machine first. We use this for Eigen on Linux/Ubuntu where `apt install libeigen3-dev` puts it in the standard place.

**`FetchContent_Declare(...)` + `FetchContent_MakeAvailable(...)`** says: "If `SomeLib` isn't here, clone it from this Git URL at this tag, and treat the clone as part of our project." Slower (downloads on first configure), but it works on a fresh machine with no system installs. We use this unconditionally for Catch2 (small, header-mostly, fast to fetch) and as a fallback for Eigen.

The downloads land in `cpp/build/_deps/`. If we ever want to wipe them and re-download, `Remove-Item -Recurse cpp/build` (PowerShell) or `rm -rf cpp/build` (bash) does the job — `_deps/` is part of the build directory and is `.gitignore`d.

## Tests in C++ — Catch2 vs pytest

We've used `pytest` in chapters 1–4. Catch2 is the same idea: a test framework that finds our tests, runs them, and tells us what failed. The mapping is:

| Concept                | Python (pytest)                  | C++ (Catch2)                                    |
| ---------------------- | -------------------------------- | ----------------------------------------------- |
| A test                 | `def test_foo(): assert ...`     | `TEST_CASE("foo", "[tag]") { REQUIRE(...); }`   |
| Tag / filter           | `@pytest.mark.foo`               | `[tag]` strings in the second argument          |
| Assertion              | `assert x == 7`                  | `REQUIRE(x == 7)`                               |
| Approximate            | `pytest.approx(0.1)`             | `Catch::Approx(0.1)`                            |
| Run from command line  | `pytest -v`                      | `./test_matrix` or `ctest --output-on-failure`  |
| Run only matching name | `pytest -k transpose`            | `./test_matrix "transpose"` or `ctest -R trans` |

The C++ version has a tiny bit more ceremony — `TEST_CASE` is a macro, we `#include <catch2/...>` headers, and we link against the framework. But the workflow is the same: write a test, run it, watch it fail, make it pass.

Catch2 uses *tags* in square brackets to group tests. Look at [`cpp/tests/test_matrix.cpp`](../cpp/tests/test_matrix.cpp):

```cpp
TEST_CASE("zeros has the requested shape and is all zero", "[basic]") { ... }
TEST_CASE("matmul agrees with reference for 3×3 random", "[matmul]") { ... }
```

Chapter 6's tests are tagged `[basic]`. Chapter 7's are `[matmul]`. From `ctest` we can filter by tag:

```bash
ctest --test-dir build -R basic       # only [basic] tests
ctest --test-dir build -R matmul      # only [matmul] tests
```

That's how the chapter docs ask you to verify you've completed only one chapter at a time.

## Glossary (when reading build output)

These terms show up across CMake / Ninja / Catch2 output. Quick translations:

- **Configure** — what `cmake -B build` does. Generates the build files.
- **Generator** — the *kind* of build files CMake writes (`Ninja`, `Unix Makefiles`, `Visual Studio 17 2022`, …). Selected with `-G`.
- **Build type** — `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel`. Maps to compiler flags: roughly `-O0 -g`, `-O3`, `-O2 -g`, `-Os`. Set with `-DCMAKE_BUILD_TYPE=Release`.
- **Target** — anything CMake builds. An executable (`test_matrix`), a library (`matrix`), or a custom command. Each `add_executable(...)` or `add_library(...)` declares one.
- **`PRIVATE` / `PUBLIC` / `INTERFACE`** — link-and-include scope. `PRIVATE` = "I use it, my users don't see it." `PUBLIC` = "I use it, and so does anyone who links against me." `INTERFACE` = "I don't use it (I'm header-only), but my users do." For this project, header-only `matrix` uses `INTERFACE`; the test binary uses `PRIVATE`.
- **Cache** — `build/CMakeCache.txt`. Stores every variable from your last configure. Editing it directly is occasionally useful but usually a mistake; prefer re-configuring with new `-D...` arguments.
- **`-D<var>=<value>`** — sets a CMake variable on the command line. The most common one is `-DCMAKE_BUILD_TYPE=Debug`.
- **Fetched dependency** — a library pulled in via `FetchContent`. Lives in `build/_deps/<name>-src/` and `build/_deps/<name>-build/`.

## What's coming in chapters 6–9

- **Chapter 6** — we fill in the basic Matrix operations. One configure (`cmake -B build`), one build, run `[basic]` tests until they pass.
- **Chapter 7** — we fill in three matmul implementations and configure **three separate build directories** (`build-O0`, `build-O2`, `build-O3`) to compare optimisation levels. Same `CMakeLists.txt`, different `-DCMAKE_BUILD_TYPE` for each.
- **Chapter 8** — we run the benchmark binary at all three opt levels and compare to Eigen. Mostly executing what's already built.
- **Chapter 9** — one plot, one diary entry, done.

The build commands in each chapter assume we've read this primer. If a future chapter says `cmake -B build-O2 -DCMAKE_BUILD_TYPE=RelWithDebInfo`, we should now be able to read that as "configure a fresh build directory called `build-O2`, with the `RelWithDebInfo` build type, generating Ninja files for it."

## When something goes wrong

- **`CMake Error: ... CMakeCache.txt directory ... is different than the directory ... where CMakeCache.txt was created`** — your `build/` was generated somewhere else (different OS, different path, Devcontainer, …). Fix: delete `build/` and reconfigure. The cache is path-locked.
- **`CMake Error: Could NOT find <package>`** — a `find_package(...)` failed. Either install the system package (`apt install libeigen3-dev` on Ubuntu) or rely on the FetchContent fallback if the project provides one (we do, for Eigen).
- **`undefined reference to ...` at link time** — the `.cpp` was compiled but not linked against the library that defines the symbol. Check `target_link_libraries(...)` for the offending target.
- **Tests not picked up by `ctest`** — usually means `catch_discover_tests(test_matrix)` ran before the executable was built, or the executable doesn't exist. Re-run `cmake --build build` first.
- **A clean build is suspiciously slow on Windows** — you're working off `/mnt/c` from WSL. See the disclaimer at the top of [Path C in setup-windows.md](setup-windows.md#path-c--wsl2--ubuntu-linux-on-windows).

→ Continue with [06 — C++ Matrix class](06-cpp-matrix-class.md)
