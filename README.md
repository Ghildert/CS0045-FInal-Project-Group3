# Echoes of November — 2.5D Narrative Adventure Game (CS0045 Finals)

An original 2.5D pixel-art narrative adventure game inspired by the visual presentation of *Until Then*, built from scratch in **C++17** and **OpenGL (Compatibility Profile)** using **FreeGLUT** and **GLEW** for Computer Graphics and Visual Computing (CS0045).

---

## 1. Prerequisites & Toolchain

This project strictly uses the course-mandated **MSYS2 UCRT64** toolchain on Windows:

- **Compiler:** MSYS2 UCRT64 GCC (`C:\msys64\ucrt64\bin\g++.exe`, C++17)
- **Build System:** CMake (`C:\msys64\ucrt64\bin\cmake.exe`) + Ninja (`C:\msys64\ucrt64\bin\ninja.exe`) or direct VS Code Build Task (`Ctrl+Shift+B`)
- **Libraries (MSYS2 UCRT64):**
  - `mingw-w64-ucrt-x86_64-freeglut`
  - `mingw-w64-ucrt-x86_64-glew`
  - `mingw-w64-ucrt-x86_64-glm`
  - Windows `opengl32` and `glu32`

To install or verify packages in an MSYS2 UCRT64 terminal:
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-freeglut mingw-w64-ucrt-x86_64-glew mingw-w64-ucrt-x86_64-glm mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
```

---

## 2. Building the Project

### Option A — CMake + Ninja (Recommended)
Run in PowerShell from the project root:
```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Option B — VS Code Build Task (`Ctrl+Shift+B`)
Press `Ctrl+Shift+B` in Visual Studio Code to run the configured MSYS2 UCRT64 build task.

---

## 3. Running the Game

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
.\build\NarrativeGame.exe
```

For testing/verification, you can also pass `--verify-phase0` to run an automated self-test that initializes OpenGL, renders multiple frames, verifies zero OpenGL errors, and exits cleanly:
```powershell
.\build\NarrativeGame.exe --verify-phase0
```

---

## 4. Documentation Index

- [Architecture Overview](docs/architecture.md)
- [OpenGL Modules 1–8 Coverage Matrix](docs/opengl-module-coverage.md)
- [Controls & Debug Shortcuts](docs/controls.md)
- [Development Log](docs/development-log.md)
