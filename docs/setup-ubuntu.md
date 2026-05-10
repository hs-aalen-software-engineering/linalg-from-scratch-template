# Setup — Ubuntu / Linux (native)

This guide covers native Ubuntu (22.04 or newer), Debian, and Ubuntu running under **WSL2** on Windows. The steps are identical — WSL2's Ubuntu shell is a real Ubuntu shell. Where something differs for WSL, it's called out inline.

> **Coming from Windows / Path C?** If [`setup-windows.md`](setup-windows.md) sent you here for Path C (WSL2 + Ubuntu), the WSL feature and the Ubuntu distro are already installed. Open the *Ubuntu* Start-menu entry and start at [Step 1](#step-1--confirm-python-and-uv) below — every command runs *inside* the Ubuntu shell, not in PowerShell.

If you'd rather not touch your system at all, the [Devcontainer fallback](#fallback-the-devcontainer) at the bottom still works.

## What you need

| Tool | Used for | Apt package |
| --- | --- | --- |
| Python ≥ 3.11 + `uv` | the Python half | already done in the course |
| g++ ≥ 11 | the C++ half | `build-essential` |
| CMake ≥ 3.20 | configures and builds | `cmake` |
| Ninja | fast parallel builds | `ninja-build` |
| Eigen 3 | the Eigen reference matmul | `libeigen3-dev` (optional — see below) |
| GNU time | peak-memory measurement (chapter 8) | usually pre-installed at `/usr/bin/time` |

`libeigen3-dev` is optional. If apt can't find it (older distros, WSL minimal install), CMake's [`FetchContent`](../cpp/CMakeLists.txt) downloads Eigen on first configure. Catch2 is always fetched.

## Step 1 — Confirm Python and `uv`

```bash
python3 --version    # expect 3.11 or newer
uv --version
```

Both should print a version. If they don't, follow the course's earlier Python setup notes.

## Step 2 — Install the C++ toolchain

```bash
sudo apt update
sudo apt install -y --no-install-recommends \
    build-essential cmake ninja-build libeigen3-dev time
```

Verify:

```bash
g++ --version
cmake --version
ninja --version
/usr/bin/time -v true 2>&1 | head -n 1   # must say "Command being timed: \"true\""
```

If `/usr/bin/time` is missing on your distro, install it: `sudo apt install -y time`. The shell builtin `time` is **not** the same — it does not have the `-v` flag and does not report peak memory.

## Step 3 — Install the VS Code extensions

If you're on a Linux desktop, install VS Code from <https://code.visualstudio.com> and add these extensions (the Devcontainer used to install them automatically):

- `charliermarsh.ruff` — Python linter/formatter (this course uses uv + ruff, not the Microsoft Python extension)
- `ms-vscode.cpptools` — C/C++ IntelliSense and debugging
- `ms-vscode.cmake-tools` — CMake integration
- `twxs.cmake` — CMakeLists.txt syntax highlighting
- `tamasfe.even-better-toml` — for editing `pyproject.toml`

**WSL note**: install VS Code on the *Windows* side, then add Microsoft's `ms-vscode-remote.remote-wsl` extension. From the Ubuntu shell, run `code .` in the repo directory — VS Code opens, attached to WSL, and prompts to install the extensions above on the Linux side. Follow the prompt.

## Step 4 — Verify the whole stack

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

The first `cmake -B build` takes 1–2 minutes because it downloads Catch2 (and Eigen if you skipped `libeigen3-dev`). Subsequent configures are instant.

If both halves run, you're done. Open [`00-getting-started.md`](00-getting-started.md#how-to-work) and continue with chapter 1.

## Troubleshooting

<details>
<summary>WSL: <code>cmake</code> or <code>g++</code> from a fresh Ubuntu install isn't found.</summary>

A fresh `wsl --install -d Ubuntu-22.04` ships without compilers. Run the `apt install` command from Step 2 inside the Ubuntu shell. If `apt update` itself fails ("Temporary failure resolving …"), restart the WSL distro: in a Windows PowerShell, `wsl --shutdown`, then re-open the Ubuntu shell.

</details>

<details>
<summary>CMake says <code>Could NOT find Eigen3</code> and then proceeds.</summary>

You skipped `libeigen3-dev`. That's fine — CMake's `FetchContent` clones Eigen 3.4 from GitLab on the first configure. The first configure takes ~30 s extra; subsequent ones reuse the clone in `cpp/build/_deps/`.

</details>

<details>
<summary>WSL: I want to edit files from Windows Explorer.</summary>

Don't. Edit through VS Code (with the WSL extension), or `cd` into `\\wsl$\Ubuntu-22.04\home\<you>\…` from Explorer. Cloning the repo into `/mnt/c/...` works but is **slow** for `cmake` and `pytest` because of the Windows ↔ Linux filesystem boundary. Keep the repo on the Linux filesystem (e.g., `~/workspace/linalg-from-scratch`).

</details>

<details>
<summary>Pytest can't find <code>matrix</code> when I run it.</summary>

You ran `pytest` instead of `uv run pytest`. The first uses your system Python; the second uses the project's virtualenv where `numpy` and the `matrix` package are installed. Always go through `uv run`.

</details>

<details>
<summary><code>/usr/bin/time -v</code> says "command not found" or doesn't accept <code>-v</code>.</summary>

Your shell is intercepting `time` as a builtin. Use the **full path** `/usr/bin/time`, not the bare word `time`. If `/usr/bin/time` itself doesn't exist, `sudo apt install time`.

</details>

## Fallback: the Devcontainer

The repo still ships a [Devcontainer](../.devcontainer/devcontainer.json) with the same packages pre-baked. Use it only if the native install above doesn't work for you. It needs Docker:

```bash
sudo apt install -y docker.io
sudo usermod -aG docker $USER   # log out and back in
```

Then in VS Code, install the *Dev Containers* extension and <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>P</kbd> → *Dev Containers: Reopen in Container*. On WSL, the Devcontainer runs *inside* WSL — that's a container inside a VM. It works but it's slow on the first build (~5 min).
