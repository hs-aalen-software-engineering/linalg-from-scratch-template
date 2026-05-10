# Setup — Windows (native)

This is the recommended path on Windows. You will install a C++ toolchain natively on your machine. No Docker, no WSL required. The Python half of the assignment only needs Python and `uv`, which you already have from earlier in the course.

If you'd rather not touch your system at all, the [Devcontainer fallback](#fallback-the-devcontainer) at the bottom still works.

## What you need

| Tool | Used for | How you'll install it |
| --- | --- | --- |
| Python ≥ 3.11 + `uv` | the Python half | already done in the course |
| A C++17 compiler | the C++ half | MSYS2 (recommended) or Visual Studio Build Tools |
| CMake ≥ 3.20 | configures and builds | comes with MSYS2 or VS Build Tools |
| Ninja | fast parallel builds | comes with MSYS2 or VS Build Tools |
| VS Code + extensions | editor | already done in the course |

You do **not** need to install Eigen or Catch2 yourself. CMake's [`FetchContent`](../cpp/CMakeLists.txt) pulls them in on first configure.

## Step 1 — Confirm Python and `uv`

Open PowerShell and run:

```powershell
python --version    # expect 3.11 or newer
uv --version
```

Both should print a version. If they don't, follow the course's earlier Python setup notes.

## Step 2 — Install a C++ toolchain

Pick one of the three paths below.

| Path | What you get | Pick this when |
| --- | --- | --- |
| **A — MSYS2 + mingw-w64** *(recommended)* | Native Windows binaries, GCC toolchain (`g++`, `pacman`) | You want everything on Windows; the lecture's `-O0 / -O2 / -O3` flags and GCC error messages match verbatim |
| **B — Visual Studio Build Tools** | Native Windows binaries, Microsoft compiler (`cl.exe`) | You already have Visual Studio installed |
| **C — WSL2 + Ubuntu** | A real Linux shell on Windows | You want exactly the lecture's Linux experience and you're willing to keep the repo on the Linux filesystem (see warning in Path C) |

All three end up with `g++` (or `cl.exe`), `cmake`, and `ninja` available.

> **Recommendation: pick Path A.** It's the simplest end-to-end on Windows: you install once via `winget`, the repo lives where you cloned it, and rebuilds are full-speed. Path C looks attractive because the chapter docs match Linux verbatim, but it has a real foot-gun — see the disclaimer at the top of Path C below. Pick C only if you've already used WSL2 and know what you're getting into.

### Path A — MSYS2 + mingw-w64 (recommended)

**First, check whether you already have what you need.** It's common to have installed these in an earlier course or experiment and forgotten. Open a fresh PowerShell window and run:

```powershell
g++ --version
cmake --version
ninja --version
```

- If **all three** print a version, skip ahead to [Step 3 — VS Code extensions](#step-3--install-the-vs-code-extensions). You're done with Step 2.
- If **only some** are missing, you can either install just the missing ones via MSYS2 below, or — if you already have a different g++ on PATH (e.g., from an old MinGW install) — keep it and only `pacman -S` the missing pieces.
- If **all three** say `command not found`, follow the install steps below.

> Even if `g++` works, glance at its version. Anything 11.0 or newer is fine for C++17. If `g++ --version` shows 7.x or 8.x (very old MinGW from years ago), prefer the fresh MSYS2 install below.

**Install steps** (only if the check above showed missing tools):

1. Install MSYS2. Pick whichever you prefer:

   **CLI (recommended)** — from any PowerShell window:

   ```powershell
   winget install --id MSYS2.MSYS2 -e
   ```

   `winget` ships with Windows 10/11 and pulls from the official package list. If you already use Chocolatey (`choco install msys2`) or Scoop (`scoop install msys2`), those work too.

   **GUI fallback** — download and run the installer from <https://www.msys2.org>. Accept the defaults.

   Either way, the install path is `C:\msys64`.

2. Open the **MSYS2 UCRT64** shell. Concretely:

   - Press <kbd>Win</kbd> to open the Start menu.
   - Type `UCRT64` (or `MSYS2`).
   - You will see several MSYS2 entries — `MSYS2 MSYS`, `MSYS2 MINGW64`, `MSYS2 UCRT64`, `MSYS2 CLANG64`, …. **Click `MSYS2 UCRT64`** (the orange "U" icon). The other entries are different toolchains; the rest of this guide assumes UCRT64.
   - A black terminal window opens with a magenta prompt that says `UCRT64`. That's the right shell.

   *Equivalent from the command line:* run `C:\msys64\ucrt64.exe` — it's a launcher that opens the same shell with the right environment variables. Useful if you ever script this.

   *Why this specific shell:* MSYS2 ships several environments (MSYS / MINGW64 / UCRT64 / CLANG64). They differ in which C runtime the toolchain links against. UCRT64 uses Microsoft's modern Universal C Runtime, which is what every Windows 10/11 machine ships with — so binaries you build will run on any other student's machine without redistributables. The `mingw-w64-ucrt-x86_64-*` package names below are tied to this shell.

   *Why not PowerShell:* the first `pacman -Syu` updates the package manager itself and asks you to close the shell mid-run. Driving that from PowerShell can leave the install half-updated.

   In the UCRT64 shell, run:

   ```bash
   pacman -Syu          # close the shell when prompted, then reopen UCRT64 and re-run
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc \
                      mingw-w64-ucrt-x86_64-cmake \
                      mingw-w64-ucrt-x86_64-ninja
   ```

   When `pacman` asks `[Y/n]`, press <kbd>Enter</kbd> to accept.

3. Add the toolchain to your **Windows** `PATH` so VS Code and PowerShell find it:
   - Press <kbd>Win</kbd> → type `env` → "Edit the system environment variables" → *Environment Variables…*
   - Under *User variables*, select `Path` → *Edit…* → *New* → type `C:\msys64\ucrt64\bin` → click *OK* on **all three** dialogs (the variable editor, the env-vars window, the system-properties window). If you skip any of the OKs, the change isn't saved.
   - **Close every PowerShell window and every VS Code window completely**, including the VS Code icon in the system tray. Environment-variable changes only reach *new* processes; sessions that were already open keep the old PATH forever.
   - Open a brand-new PowerShell window. Verify:

     ```powershell
     g++ --version
     cmake --version
     ninja --version
     ```

   If any of the three say `not recognized as a name of a cmdlet`, run this diagnostic:

   ```powershell
   # Did the persisted edit land?
   [Environment]::GetEnvironmentVariable("Path", "User") -split ';' | Select-String 'msys'
   # Does this shell see it?
   $env:Path -split ';' | Select-String 'msys'
   ```

   - First line prints `C:\msys64\ucrt64\bin` but the second is empty → you have a stale PowerShell. Close it (and any VS Code) fully and open a new one.
   - Both empty → the env-var edit didn't save. Redo it and watch for the three OK clicks.
   - Both show the path but `cmake` still fails → the binary isn't there. Re-run the `pacman -S --needed …` line in the UCRT64 shell and watch for errors.

   *One-shot patch for the current session* (handy when debugging): `$env:Path += ";C:\msys64\ucrt64\bin"`. Lasts until you close the window.

### Path B — Visual Studio 2022 Build Tools (MSVC)

Use this if you already have Visual Studio installed or your employer mandates it. The C++ compiler is `cl.exe` (MSVC), not `g++`. CMake handles the difference; `-O3` becomes `/O2` under the hood.

1. Download "Build Tools for Visual Studio 2022" from <https://visualstudio.microsoft.com/downloads/> (scroll to *Tools for Visual Studio*).
2. Run the installer. Select the **Desktop development with C++** workload. Make sure these components are checked: *MSVC v143*, *Windows 11 SDK*, *C++ CMake tools for Windows*.
3. After install, **always run the C++ commands from a "Developer PowerShell for VS 2022"** shell — it's in the Start menu. A normal PowerShell does not have `cl.exe` on the path.

   ```powershell
   cl                   # prints the MSVC banner
   cmake --version
   ninja --version
   ```

> Where the chapter docs say `-O3`, mentally translate to `/O2` if you took Path B. The CMake build types (`Release`, `RelWithDebInfo`, `Debug`) work identically on both compilers — that's the level you actually pass.

### Path C — WSL2 + Ubuntu (Linux on Windows)

WSL2 gives you a real Ubuntu shell on Windows. The chapter docs (which assume Linux) and the `/usr/bin/time -v` peak-memory recipe in [chapter 8](08-cpp-vs-eigen.md) work verbatim — no PowerShell `PeakWorkingSet64` workaround, no `-O3 → /O2` translation.

> ⚠️ **Read this before you commit to Path C.** WSL2 is a Linux VM, and its filesystem and your Windows filesystem are *separate*. If you clone the repo on the Windows side (e.g., to `C:\Users\you\workspace\...`) and access it from WSL via `/mnt/c/...`, every file syscall crosses the VM boundary. CMake configure, builds, and test runs slow down by **5–10×**. A clean C++ build that takes 30 seconds on Path A can take 3–5 minutes through `/mnt/c`.
>
> The fix is to clone the repo *inside* WSL onto the Linux filesystem (`~/workspace/...`). That works fine, but it means:
>
> - Two copies of the repo on disk if you also keep the Windows checkout (or you have to delete one).
> - You navigate to it via `\\wsl$\Ubuntu\home\<you>\...` from Windows tools, which is not where most students expect their files to live.
> - Editing must go through VS Code's WSL Remote extension; opening the same folder in plain Windows VS Code crosses the boundary again.
>
> **For this course, Path A is the safer default on Windows.** It avoids the dual-filesystem trap entirely. The cost is a small handful of Windows-specific footnotes elsewhere in the chapter docs (PowerShell instead of `/usr/bin/time -v`, etc.) — that's a fair trade for build speed and one-copy simplicity.
>
> Stick with Path C only if you've used WSL2 before and you're comfortable keeping a single working copy under `~/workspace/...` on the Linux side.

**First, check whether WSL is already there.** From PowerShell:

```powershell
wsl --status
```

- If it lists a default distribution (e.g., `Ubuntu`), open it from the Start menu and skip to "Inside the Ubuntu shell" below.
- If it says `Windows Subsystem for Linux has no installed distributions` or the command isn't recognised, install it:

  ```powershell
  wsl --install -d Ubuntu-22.04
  ```

  This needs admin (UAC will prompt). It enables the WSL feature, downloads the Ubuntu image, and reboots if necessary. After the reboot, an "Ubuntu" Start-menu entry appears; open it once to finish setup (it asks for a Linux username and password — pick anything; this is a per-distro account, unrelated to your Windows account).

**Inside the Ubuntu shell**, follow [`setup-ubuntu.md`](setup-ubuntu.md) from Step 2 onward. The whole thing — `apt install build-essential cmake ninja-build libeigen3-dev`, `uv sync`, `cmake -B build`, `ctest` — works as written. Python and `uv` need to be installed *inside* WSL too (they don't share with your Windows install); see the course's earlier setup notes or run `curl -LsSf https://astral.sh/uv/install.sh | sh` from the Ubuntu shell.

**Two gotchas worth knowing up front:**

1. **Keep the repo on the Linux filesystem.** Clone into `~/workspace/...` inside Ubuntu, *not* into `/mnt/c/Users/...`. The `/mnt/c` path is a bridge to Windows-side NTFS and is ~10× slower for `cmake` and `pytest` because every file syscall crosses the VM boundary.

2. **Open the project in VS Code via the WSL extension.** Install Microsoft's `ms-vscode-remote.remote-wsl` on the Windows side, then from the Ubuntu shell run `code .` in the repo directory. VS Code opens *attached to WSL* — the integrated terminal is bash, the Python interpreter is the Linux uv venv, and CMake/Ninja are the Linux ones. Don't open the same folder via `\\wsl$\…` in a regular Windows VS Code window; that'll mix Windows and Linux extensions and be slow.

That's it for Path C. Chapter docs match what you see on screen and you skip the Windows-specific footnotes elsewhere in this guide.

## Step 3 — Install the VS Code extensions

In VS Code, install these (the Devcontainer used to install them automatically):

- `charliermarsh.ruff` — Python linter/formatter (this course uses uv + ruff, not the Microsoft Python extension)
- `ms-vscode.cpptools` — C/C++ IntelliSense and debugging
- `ms-vscode.cmake-tools` — CMake integration
- `twxs.cmake` — CMakeLists.txt syntax highlighting
- `tamasfe.even-better-toml` — for editing `pyproject.toml`

You can install them one shot from the *Extensions* sidebar with these IDs.

## Step 4 — Verify the whole stack

From the repo root in PowerShell:

```powershell
# Python
cd python
uv sync
uv run pytest -v        # long list of FAILs is correct — TODOs aren't filled in yet

# C++
cd ..\cpp
cmake -B build -S . -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

The first `cmake -B build` takes 1–2 minutes because it downloads Catch2 and (if no system Eigen is found) Eigen. Subsequent configures are instant.

If both halves run, you're done. Skipping ahead: open [`00-getting-started.md`](00-getting-started.md#how-to-work) and continue with chapter 1.

## Measuring peak memory on Windows

[Chapter 8](08-cpp-vs-eigen.md) measures the C++ benchmark's peak working set. The Linux command `/usr/bin/time -v` does not exist on Windows. Use this PowerShell snippet instead:

```powershell
cd cpp
$p = Start-Process -FilePath ".\build-O3\benchmark.exe" `
                   -RedirectStandardOutput "..\results\cpp-O3.csv" `
                   -PassThru -NoNewWindow -Wait
"Peak working set: {0:N0} KB" -f ($p.PeakWorkingSet64 / 1KB)
```

`PeakWorkingSet64` is the Windows equivalent of "Maximum resident set size". The number is in bytes; we divide by `1KB` for kilobytes to match Linux's `/usr/bin/time` output.

## Troubleshooting

<details>
<summary><code>g++: command not found</code> in PowerShell after installing MSYS2.</summary>

You added `C:\msys64\ucrt64\bin` to PATH but didn't open a *new* PowerShell window. Environment variable changes do not propagate to already-open terminals. Close and reopen.

</details>

<details>
<summary>CMake says <code>Could NOT find Eigen3</code> and then proceeds.</summary>

That is expected on Windows. There is no Windows package for Eigen on `apt` because there is no `apt`. The [`cpp/CMakeLists.txt`](../cpp/CMakeLists.txt) detects the absence and falls back to `FetchContent` from GitLab. The first configure takes ~30 s extra.

</details>

<details>
<summary>VS Code's C/C++ extension picks the wrong compiler.</summary>

<kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>P</kbd> → *CMake: Select a Kit* → pick "GCC … (mingw-w64)" for Path A or "Visual Studio Build Tools 2022" for Path B. The kit is remembered per workspace.

</details>

<details>
<summary>Pytest can't find <code>matrix</code> when I run it.</summary>

You ran `pytest` instead of `uv run pytest`. The first uses your system Python; the second uses the project's virtualenv where `numpy` and the `matrix` package are installed. Always go through `uv run`.

</details>

## Fallback: the Devcontainer

If something above doesn't work and you don't want to debug it, the repo still has its [Devcontainer](../.devcontainer/devcontainer.json). It is **not the recommended path on Windows** — it requires Docker Desktop, which needs WSL2 and 6+ GB of RAM dedicated to a VM — but it is a known-good environment.

Use it only if:
- You already have Docker Desktop running, **or**
- You've burned 30+ minutes on a native setup issue and want to move on.

To use it: install Docker Desktop, install the *Dev Containers* VS Code extension, then <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>P</kbd> → *Dev Containers: Reopen in Container*.
