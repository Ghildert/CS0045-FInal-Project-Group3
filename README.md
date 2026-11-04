# Echoes of November — 2.5D Narrative Adventure Game (CS0045 Finals)

An original 2.5D pixel-art narrative adventure game inspired by the visual presentation and gameplay style of *Until Then*, built from scratch in **C++17** and **OpenGL (Compatibility Profile)** using **FreeGLUT** and **GLEW** for **Computer Graphics and Visual Computing (CS0045)**.

Every OpenGL and FreeGLUT concept from **Modules 1–8** of the course lectures is visibly and accurately implemented both inside the playable 2.5D narrative adventure and in an interactive **8-Station OpenGL Coursework Demonstration Mode (`[F2]`)**.

---

## 1. Key Features & Coursework Highlights

1. **Three Playable 2.5D Locations:**
   - **Room 304 — Late Afternoon Classroom:** Golden sunset light streaming through windows, floating dust motes (`GL_POINTS`), 6 student desks + teacher's desk (`glDrawElements` VBO+EBO), chalkboard, sketchbook prop, GLUT 3D objects (`glutSolidTeapot`, `glutSolidSphere`), and classmate **Maya Lin**.
   - **3rd-Floor School Corridor:** Terrazzo tiled hallway, steel-blue locker banks, campus noticeboard, corridor bench, and Band Club member **Sam**.
   - **Rainy Street — 24/7 Sundown Mart Exterior:** Rain-slicked asphalt & crosswalk, illuminated convenience store neon canopy, cast-iron streetlamp with point light, bus-stop bench with Sam's acoustic guitar case, falling stippled rain streaks (`GL_LINES` + `glLineStipple`), and expanding puddle splash rings (`GL_LINE_LOOP`).
2. **2.5D Pixel-Art Characters in 3D Space:**
   - Animated 4x2 sprite sheets (`player_elijah_sheet.png`, `npc_maya_sheet.png`, `npc_sam_sheet.png`) rendered on dynamic 3D quads (`glBufferSubData`) with `GL_NEAREST` pixel-art filtering, alpha testing/blending, and `GL_TRIANGLE_FAN` ground contact shadows.
3. **3D AABB Sliding Collision & Proximity Interactions:**
   - Axis-separated X/Z sliding collision detection (`CollisionWorld::resolveMovementXZ`) with optional `[F3]` stippled wireframe collider debug view.
   - Rotating 3D amber diamond beacons (`GL_TRIANGLES`) and stippled ground rings (`glLineStipple`) over interactive objects and NPCs.
4. **External Branching Dialogue, Portraits, Smartphone UI & Save/Load:**
   - External story script (`data/dialogue/story_dialogues.txt`) with $48 \times 48$ pixel-art portraits (`portrait_elijah.png`, `portrait_maya.png`, `portrait_sam.png`), typewriter reveal, and branching player choices.
   - Interactive **Smartphone Messenger (`[TAB]`)** loaded from `data/dialogue/phone_messages.txt` with unread notification badges and reply choices.
   - Persistent story flags and **Quick Save (`[F5]`) / Quick Load (`[F9]`)** serialized to `data/saves/save_slot_1.sav`.
5. **Interactive Acoustic Guitar Rhythm Minigame (`[G]` or Guitar Bench):**
   - 4-lane (`[A/S/D/F]` or `[1/2/3/4]`) 16-note rhythm sequence featuring hierarchical matrix-transformed note gems (`glPushMatrix`, `glTranslatef`, `glRotatef`, `glScalef`), stippled fret wires (`glLineStipple`), and hit-burst particle stars (`GL_POINTS`).
6. **Interactive 8-Station OpenGL Coursework Demo Mode (`[F2]`):**
   - Dedicated interactive showcase for **Modules 1–8** (`[1]`–`[8]`), displaying each module's live 3D/2D demonstration, the exact OpenGL APIs invoked, and the corresponding source file & function in the game.
7. **Main Menu, Pause Menu (`[ESC]`), and Synthesized WAV Audio:**
   - Title screen & pause overlay (`MenuSystem`) and asynchronous WinMM PCM `.wav` sound effects (`AudioSystem`).

---

## 2. Prerequisites & MSYS2 UCRT64 Toolchain

This project strictly uses the course-mandated **MSYS2 UCRT64** toolchain on Windows:

- **Compiler:** MSYS2 UCRT64 GCC (`C:\msys64\ucrt64\bin\g++.exe`, C++17)
- **Build System:** CMake + Ninja (or VS Code Build Task `Ctrl+Shift+B`)
- **Libraries (MSYS2 UCRT64):**
  - `mingw-w64-ucrt-x86_64-freeglut`
  - `mingw-w64-ucrt-x86_64-glew`
  - `mingw-w64-ucrt-x86_64-glm`
  - `mingw-w64-ucrt-x86_64-stb`
  - Windows `opengl32`, `glu32`, and `winmm`

---

## 3. Building and Running the Project

### Configure & Build (PowerShell)
```powershell
$env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;$env:PATH"
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Run the Game
```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
.\build\NarrativeGame.exe
```

### Run Automated Unit Tests & Full Runtime Self-Test
```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
ctest --test-dir build --output-on-failure
.\build\NarrativeGame.exe --verify-phase8
```

---

## 4. Quick Walkthrough Guide

1. **Main Menu:** Press `ENTER` on **Start / Continue Story** (or select **OpenGL Coursework Demo Mode [F2]** to inspect Modules 1–8 directly).
2. **Room 304 (Classroom):**
   - Walk (`WASD`) to **Elijah's Sketchbook** on the front-left desk and press `E` to pick it up.
   - Walk to **Maya** near the window and press `E` to talk and choose a dialogue response.
   - Press `TAB` to open your **Smartphone** and reply to Sam's text message.
   - Walk to the **Hallway Sliding Door** on the right wall and press `E` to enter the 3rd-Floor Hallway.
3. **3rd-Floor School Hallway:**
   - Talk to **Sam** by the lockers (`E`) to get the guitar case code and unlock the stairwell exit.
   - Walk to the **Stairwell Exit** on the far right (`E`) to head down to the Rainy Street.
4. **Rainy Street (24/7 Sundown Mart):**
   - Talk to **Maya** under the neon store awning (`E`).
   - Inspect **Sam's Acoustic Guitar** on the bus-stop bench (`E`, or press `G`) to play the **Acoustic Guitar Rhythm Minigame** (`A`, `S`, `D`, `F` or `1`, `2`, `3`, `4` as notes cross the target line).
5. **Coursework Defense (`[F2]`):**
   - Press `F2` at any time to enter the **OpenGL Coursework Demo Mode**, then press keys `1` through `8` (or `Left`/`Right` arrows) to demonstrate **Modules 1–8** live.

---

## 5. Documentation Index

- [Architecture Overview](docs/architecture.md)
- [OpenGL Modules 1–8 Coverage Matrix](docs/opengl-module-coverage.md)
- [Controls & Debug Shortcuts](docs/controls.md)
- [Development Log](docs/development-log.md)
