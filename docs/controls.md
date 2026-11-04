# Controls & Debug Shortcuts — Echoes of November

## 1. General, Menu & Window Controls

| Key / Input | Action | OpenGL / GLUT Callback Used |
| :--- | :--- | :--- |
| `UP` / `DOWN` (`W` / `S`) | Navigate Main Menu & Pause Menu items | `glutSpecialFunc` / `glutKeyboardFunc` |
| `ENTER` / `SPACE` | Activate selected Menu item | `glutKeyboardFunc` |
| `ESC` | Open/Close Pause Menu (or close active overlay / exit from Main Menu) | `glutKeyboardFunc` (`key == 27`) |
| `F2` | Toggle Interactive OpenGL Coursework Demo Mode (Modules 1–8) | `glutSpecialFunc` (`GLUT_KEY_F2`) |
| `F3` | Toggle 3D AABB Collider Wireframe & Diagnostics Overlay | `glutSpecialFunc` (`GLUT_KEY_F3`) |
| `F5` / `F9` | Quick Save (`data/saves/save_slot_1.sav`) / Quick Load progress | `glutSpecialFunc` (`GLUT_KEY_F5` / `F9`) |
| `F6` / `F7` / `F8` | Debug Teleport: Classroom (`F6`), Hallway (`F7`), Rainy Street (`F8`) | `glutSpecialFunc` |
| `F11` | Toggle Fullscreen / 1280x720 Windowed Mode | `glutFullScreen` / `glutReshapeWindow` |

---

## 2. 2.5D Exploration, Dialogue & Smartphone Controls

| Key / Input | Action |
| :--- | :--- |
| `W` / `A` / `S` / `D` or `Arrow Keys` | Move Elijah in 2.5D space (Depth `Z` and Horizontal `X`) |
| `E` / `ENTER` | Interact with nearest highlighted object, door, guitar bench, or NPC |
| `SPACE` / `ENTER` / `E` / `Left Click` | Complete typewriter text reveal or advance to next dialogue line |
| `1` / `2` or `UP` / `DOWN` + `ENTER` | Select branching dialogue choice or smartphone reply |
| `TAB` | Open / Close in-game Smartphone Messenger UI |
| `G` | Launch Acoustic Guitar Rhythm Minigame directly |
| `T` | Toggle Procedural Environment Textures vs. UV Checkerboard Debug Texture |
| `M` | Toggle Wireframe vs. Filled polygon rendering on 3D furniture |
| `1` / `2` / `3` / `4` | Toggle `GL_LIGHTING` (`1`), Sun `GL_LIGHT0` (`2`), Spot `GL_LIGHT1` (`3`), Specular (`4`) |
| `C` | Toggle 2.5D Narrative Tracking Camera vs. Free Orbit Camera |
| `P` | Cycle Camera Projection Mode (`gluPerspective` / `glFrustum` / `glOrtho`) |

---

## 3. Acoustic Guitar Rhythm Minigame Controls

| Key / Input | Action |
| :--- | :--- |
| `A` or `1` | Strum Lane 1 (Amber String) |
| `S` or `2` | Strum Lane 2 (Cyan String) |
| `D` or `3` | Strum Lane 3 (Emerald String) |
| `F` or `4` | Strum Lane 4 (Coral String) |
| `ESC` | Exit Minigame early and return to exploration |

---

## 4. OpenGL Coursework Demo Mode Controls (`[F2]`)

| Key / Input | Action |
| :--- | :--- |
| `1` – `8` | Jump directly to **Module 1 – Module 8** Interactive Demonstration Station |
| `LEFT` / `RIGHT` (`[` / `]`) | Step to Previous / Next Module Station |
| `SPACE` | Trigger Module-specific action (e.g., Module 5 `glMapBuffer` CPU vertex translation) |
| `P` | Cycle Projection Matrix (`gluPerspective` -> `glFrustum` -> `glOrtho`) |
| `C` | Toggle Free Orbit Camera (`Right Mouse Drag` to orbit, `W`/`S` to zoom) |
| `W` | Toggle `glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)` Wireframe Inspection |
| `L` | Toggle Fixed-Function Phong Lighting (`GL_LIGHTING`) |
| `F` | Toggle Texture Magnification Filter (`GL_NEAREST` pixel-art vs. `GL_LINEAR` bilinear) |
| `R` | Reset Camera and Demo Station state |
