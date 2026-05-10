# Setup — macOS (native)

This guide covers macOS 13 (Ventura) or newer on both Apple Silicon (M-series) and Intel Macs. The steps are identical; where the two architectures differ (Homebrew install path, `gtime` vs `/usr/bin/time -l`), it's called out inline.

If you'd rather not touch your system at all, the [Devcontainer fallback](#fallback-the-devcontainer) at the bottom still works.

## What you need

| Tool | Used for | How you'll install it |
| --- | --- | --- |
| Python ≥ 3.12 + `uv` | the Python half | see [Step 1](#step-1--confirm-python-and-uv) if you skipped the earlier course setup |
| Apple Clang ≥ 14 (or GCC ≥ 11) | the C++ half | Xcode Command Line Tools (or `brew install gcc`) |
| CMake ≥ 3.20 | configures and builds | `brew install cmake` |
| Ninja | fast parallel builds | `brew install ninja` |
| Eigen 3 | the Eigen reference matmul | `brew install eigen` (optional — see below) |
| GNU time (`gtime`) | peak-memory measurement (chapter 8) | `brew install gnu-time` (optional — BSD `/usr/bin/time -l` works too) |

`eigen` is optional. If it isn't installed, CMake's [`FetchContent`](../cpp/CMakeLists.txt) downloads Eigen on first configure. Catch2 is always fetched.

`gnu-time` is also optional — macOS ships a BSD `time` at `/usr/bin/time` whose `-l` flag prints "maximum resident set size". The Linux command in chapter 8 uses GNU's `-v`; installing `gnu-time` lets you copy the chapter command verbatim as `gtime -v`. Pick whichever you prefer.

## Step 1 — Confirm Python and `uv`

Open Terminal (or iTerm2) and run:

```bash
python3 --version    # expect 3.12 or newer — pyproject.toml requires >=3.12
uv --version
```

If both print a version **and Python is 3.12 or newer**, skip to Step 2. Otherwise install what's missing — both pieces are quick.

> **About the macOS-bundled Python.** macOS ships a `python3` that's pinned to whatever the OS release shipped with — on macOS 13 it's 3.9, on macOS 14 it's 3.9 too, on macOS 15 it's 3.9 still. So the system `python3` is almost certainly *too old* for this repo. Don't try to upgrade it — install a private interpreter via `uv` below instead.

### Install `uv` (if missing)

The fastest path is the official installer — it drops a single static binary into `~/.local/bin/uv` and works regardless of whether you have Homebrew:

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

Or via Homebrew, if you already have it (Homebrew install itself is covered in Step 2):

```bash
brew install uv
```

Restart the shell (or `source ~/.zprofile`) so the new `PATH` takes effect, then re-check `uv --version`. Other options (pipx, GitHub releases) are listed at <https://docs.astral.sh/uv/getting-started/installation/>.

### Install Python 3.12 through `uv` (if missing or too old)

You do **not** need to upgrade the macOS-bundled Python or install one from python.org. `uv` ships its own downloader that places hermetic interpreters under `~/.local/share/uv/python/` — they coexist with the system Python:

```bash
uv python install 3.12
```

That's it. Later in this guide, `uv sync` (run from the repo's `python/` directory) reads `requires-python = ">=3.12"` from [`pyproject.toml`](../python/pyproject.toml) and automatically picks the interpreter `uv` just installed. You don't need to point at it manually, and your system `python3` keeps working as before — uv's interpreter is private to projects that ask for it.

## Step 2 — Install Homebrew (if you don't have it)

Homebrew is the standard macOS package manager. Check first — it's common to already have it from another project:

```bash
brew --version
```

If that prints a version, skip to Step 3. Otherwise install it:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

The installer asks for your password (it uses `sudo` to create `/opt/homebrew` on Apple Silicon or `/usr/local` on Intel) and prints two `eval` lines at the end that add `brew` to your `PATH`. Copy those into your shell profile as instructed — typically:

```bash
# Apple Silicon (M1/M2/M3/M4)
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
eval "$(/opt/homebrew/bin/brew shellenv)"

# Intel
echo 'eval "$(/usr/local/bin/brew shellenv)"' >> ~/.zprofile
eval "$(/usr/local/bin/brew shellenv)"
```

Close and reopen Terminal, then verify `brew --version` works in a fresh window.

## Step 3 — Install the C++ toolchain

```bash
xcode-select --install     # installs Apple Clang + the macOS SDK; skip if already done
brew install cmake ninja eigen
```

`xcode-select --install` pops up a GUI dialog. Click *Install* and wait — it's ~1 GB and 5–10 minutes. If it says "command line tools are already installed", you're set.

If you want GNU `time` to match the Linux chapter command verbatim (optional):

```bash
brew install gnu-time      # provides `gtime` with the `-v` flag
```

Verify:

```bash
clang++ --version          # Apple clang version 14 or newer
cmake --version
ninja --version
/usr/bin/time -l true 2>&1 | tail -n 5    # BSD time; look for "maximum resident set size"
gtime -v true 2>&1 | head -n 1            # only if you installed gnu-time
```

> **Prefer GCC over Apple Clang?** `brew install gcc` installs versioned binaries like `g++-14`. CMake's compiler detection picks Apple Clang by default; to force GCC, configure with `cmake -B build -S . -G Ninja -DCMAKE_CXX_COMPILER=g++-14`. For this course either compiler is fine — the `-O0 / -O2 / -O3` flags work the same on both.

## Step 4 — Install the VS Code extensions

Install VS Code from <https://code.visualstudio.com> (or `brew install --cask visual-studio-code`) and add these extensions (the Devcontainer used to install them automatically):

- `charliermarsh.ruff` — Python linter/formatter (this course uses uv + ruff, not the Microsoft Python extension)
- `ms-vscode.cpptools` — C/C++ IntelliSense and debugging
- `ms-vscode.cmake-tools` — CMake integration
- `twxs.cmake` — CMakeLists.txt syntax highlighting
- `tamasfe.even-better-toml` — for editing `pyproject.toml`

## Step 5 — Verify the whole stack

From the repo root:

```bash
# Python
cd python
uv sync
uv run pytest -v        # long list of FAILs is correct — TODOs aren't filled in yet

# C++
cd ../cpp
cmake -B build -S . -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

The first `cmake -B build` takes 1–2 minutes because it downloads Catch2 (and Eigen if you skipped `brew install eigen`). Subsequent configures are instant.

If both halves run, you're done. Open [`00-getting-started.md`](00-getting-started.md#how-to-work) and continue with chapter 1.

## Measuring peak memory on macOS

[Chapter 8](08-cpp-vs-eigen.md) measures the C++ benchmark's peak working set. macOS's BSD `time` uses a different flag than GNU's `-v`. You have two options:

**Option 1 — BSD `time` (no extra install):**

```bash
cd cpp
/usr/bin/time -l ./build-O3/benchmark > ../results/cpp-O3.csv
```

Look for `maximum resident set size` in the output. It's printed in bytes on macOS 13+ (older releases printed kilobytes — check the line just below the field name).

**Option 2 — GNU `gtime` (matches the Linux chapter command):**

```bash
brew install gnu-time     # one-time
cd cpp
gtime -v ./build-O3/benchmark > ../results/cpp-O3.csv
```

The output then matches the Linux example in chapter 8 line-for-line, including the `Maximum resident set size (kbytes)` label.

## Troubleshooting

<details>
<summary><code>brew: command not found</code> after running the installer.</summary>

The installer printed two `eval` lines at the end (it tells you the exact path: `/opt/homebrew/bin/brew shellenv` on Apple Silicon, `/usr/local/bin/brew shellenv` on Intel). You skipped them — they're what puts `brew` on your `PATH`. Re-run them from your shell, append the `eval` line to `~/.zprofile`, then open a new Terminal window.

</details>

<details>
<summary>CMake says <code>Could NOT find Eigen3</code> and then proceeds.</summary>

You skipped `brew install eigen`. That's fine — CMake's `FetchContent` clones Eigen 3.4 from GitLab on the first configure. The first configure takes ~30 s extra; subsequent ones reuse the clone in `cpp/build/_deps/`.

</details>

<details>
<summary>Apple Silicon: <code>clang++</code> rejects a flag the chapter command used.</summary>

Apple Clang is based on upstream LLVM/Clang but lags by a release or two. The `-O0 / -O2 / -O3` flags used in this course are stable across both. If a flag from a chapter is genuinely missing, either install `brew install llvm` (provides current `clang++` at `/opt/homebrew/opt/llvm/bin/clang++`) or use GCC via `brew install gcc` and `-DCMAKE_CXX_COMPILER=g++-14`.

</details>

<details>
<summary><code>/usr/bin/time -v</code> says "illegal option -- v".</summary>

That's the BSD `time` shipped with macOS — it uses `-l`, not `-v`. Either use `/usr/bin/time -l` (and read "maximum resident set size") or `brew install gnu-time` and call `gtime -v`. The shell builtin `time` is a third thing — it does neither and only reports wall/user/system time.

</details>

<details>
<summary>Pytest can't find <code>matrix</code> when I run it.</summary>

You ran `pytest` instead of `uv run pytest`. The first uses your system Python; the second uses the project's virtualenv where `numpy` and the `matrix` package are installed. Always go through `uv run`.

</details>

<details>
<summary>VS Code's C/C++ extension picks the wrong compiler.</summary>

<kbd>Cmd</kbd>+<kbd>Shift</kbd>+<kbd>P</kbd> → *CMake: Select a Kit* → pick "Clang … (apple-clang)" for the default Apple toolchain or "GCC … (homebrew)" if you installed GCC. The kit is remembered per workspace.

</details>

## Fallback: the Devcontainer

The repo still ships a [Devcontainer](../.devcontainer/devcontainer.json) with the same packages pre-baked. Use it only if the native install above doesn't work for you. It needs Docker Desktop:

```bash
brew install --cask docker
```

Then in VS Code, install the *Dev Containers* extension and <kbd>Cmd</kbd>+<kbd>Shift</kbd>+<kbd>P</kbd> → *Dev Containers: Reopen in Container*. First-time build is ~5 minutes.
