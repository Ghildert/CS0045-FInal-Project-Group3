# Controls & Debug Shortcuts

## 1. General & Window Controls

| Key / Input | Action | OpenGL / GLUT Callback Used |
| :--- | :--- | :--- |
| `F1` | Toggle OpenGL Diagnostics & Pipeline Overlay | `glutKeyboardFunc` / `glutSpecialFunc` |
| `F2` | Toggle OpenGL Coursework Demo Mode (Modules 1–8) | `glutSpecialFunc` |
| `F11` | Toggle Fullscreen / Windowed Mode | `glutFullScreen` / `glutReshapeWindow` |
| `ESC` | Pause Menu / Return to Main Menu / Exit (in Phase 0) | `glutKeyboardFunc` (`key == 27`) |

## 2. Gameplay Controls (Phases 2–8)

| Key / Input | Action |
| :--- | :--- |
| `W` / `A` / `S` / `D` | Move character in 2.5D space (Depth `Z` and Horizontal `X`) |
| `E` / `SPACE` / `ENTER` | Interact with highlighted object/NPC or advance dialogue |
| `1` / `2` / `3` | Select dialogue choice |
| `TAB` / `P` | Open/Close in-game Phone UI |
| `F5` / `F9` | Quick Save / Quick Load progress |
| `Left Mouse Click` | Click UI buttons, dialogue options, or inspect objects |
| `Mouse Move` | Hover UI elements (`glutPassiveMotionFunc`) |

## 3. Coursework Demo Mode Controls (Phase 0 & Phase 7)

| Key / Input | Action |
| :--- | :--- |
| `1` – `8` | Jump directly to Module 1 – Module 8 Interactive Demo |
| `[` / `]` | Cycle sub-demonstrations within the active Module |
| `P` | Toggle Perspective (`gluPerspective`) vs. Orthographic (`glOrtho`) projection |
| `L` | Toggle OpenGL Lighting (`GL_LIGHTING`, `GL_LIGHT0`, `GL_LIGHT1`) |
| `T` | Toggle Texture Filtering (`GL_NEAREST` vs. `GL_LINEAR`) |
| `M` | Trigger Dynamic VBO `glMapBuffer` / 4x4 Matrix Translation step |
| `Left Mouse Drag` | Orbit 3D inspection camera (`glutMotionFunc`) |
