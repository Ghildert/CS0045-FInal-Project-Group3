# Development Log

## Phase 0 — Requirements Audit and Environment Setup (2026-11-05)

### Completed Tasks
1. **Lecture Modules Audit:**
   - Read all 8 lecture PDFs (`Module 01.pdf` through `Module 08.pdf`).
   - Extracted every required OpenGL function, constant, and code pattern into `docs/opengl-module-coverage.md`.
   - Identified and documented lecture sample code caveats (such as `glutInitDisplayMode` ordering before `glutCreateWindow`, `glColor*` before `glRasterPos2f`, per-frame VBO/texture allocation vs. cached GPU resources, and `GL_ELEMENT_ARRAY_BUFFER` usage).
2. **Environment & Toolchain Verification:**
   - Verified OS: Windows with dual GPUs (`Intel(R) UHD Graphics` and `NVIDIA GeForce RTX 5050 Laptop GPU`).
   - Verified Compiler: `C:\msys64\ucrt64\bin\g++.exe` (MSYS2 UCRT64 GCC 16.2.0).
   - Verified Libraries in MSYS2 UCRT64:
     - FreeGLUT (`libfreeglut`, `<GL/glut.h>`, `<GL/freeglut_ext.h>`)
     - GLEW (`libglew32`, `<GL/glew.h>`)
     - GLM (`<glm/glm.hpp>`)
     - Windows OpenGL & GLU (`libopengl32`, `libglu32`)
   - Installed CMake 4.4.3, Ninja 1.13.2, Make 4.4.1, and GLM 1.0.3 into MSYS2 UCRT64 (`C:\msys64\ucrt64\bin`).
3. **Project Structure & Build System:**
   - Created modular directory hierarchy (`src/`, `docs/`, `assets/`, `data/`, `tests/`, `.vscode/`).
   - Created `CMakeLists.txt` targeting C++17 with MSYS2 UCRT64 GCC, GLEW (`GLEW_STATIC`), FreeGLUT, GLU, and OpenGL32.
   - Created Phase 0 verification program (`src/main.cpp`) demonstrating:
     - OpenGL Compatibility Profile initialization and startup diagnostics (`GL_VERSION`, `GL_VENDOR`, `GL_RENDERER`).
     - OpenGL Debug Output callback (`glDebugMessageCallback`, Module 6).
     - FreeGLUT callbacks (`glutDisplayFunc`, `glutReshapeFunc`, `glutKeyboardFunc`, `glutMouseFunc`, `glutMotionFunc`, `glutPassiveMotionFunc`, `glutEntryFunc`, `glutTimerFunc`).
     - Interactive verification view rendering a 3D perspective color-cube/pyramid via VBO (`glGenBuffers`, `glBindBuffer`, `glBufferData`, `glDrawArrays`) and a 2D HUD overlay with `glLineStipple` and `glutBitmapString`.


---

## Phase 1 — Core Renderer (2026-11-05)

### Completed Subsystems
1. **Debug Logging & Error Checking (`src/engine/core/Logger.h`, `Logger.cpp`):**
   - Prints OpenGL Version, Vendor, Renderer, and `GL_MAX_LIGHTS` on startup.
   - Enables `GL_DEBUG_OUTPUT` and registers `glDebugMessageCallback` (Module 6).
   - Tracks OpenGL errors per frame via `glGetError()`.
2. **Custom $4 \times 4$ Matrix & Homogeneous Coordinate Utilities (`src/engine/core/Math4x4.h`, `Math4x4.cpp`):**
   - Implements `Vec4(x, y, z, w)` ($w=1$ for position, $w=0$ for direction) and `Mat4` (addition, subtraction, scalar product, $4 \times 4$ matrix-vector product, $4 \times 4$ matrix-matrix product, translation, rotation, scaling).
   - Implements `Math4x4::translateVertexInMappedVBO` and `Math4x4::transformMeshVerticesInMappedVBO` using `glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE)` and `glUnmapBuffer(GL_ARRAY_BUFFER)` (Modules 5 & 6).
3. **Input System (`src/engine/input/InputManager.h`, `InputManager.cpp`):**
   - Tracks keyboard press/release, special keys, mouse buttons (`GLUT_LEFT_BUTTON`, `GLUT_MIDDLE_BUTTON`, `GLUT_RIGHT_BUTTON`, `GLUT_DOWN`, `GLUT_UP`), mouse delta motion (`glutMotionFunc`, `glutPassiveMotionFunc`), and cursor window entry state (`glutEntryFunc` with `GLUT_ENTERED` and `GLUT_LEFT`, Module 3).
4. **Texture Loading & Pixel-Art Asset Pipeline (`src/engine/assets/Texture2D.h`, `Texture2D.cpp`):**
   - Uses `stb_image.h` (`stbi_load`, `stbi_image_free`) and `stb_image_write.h`.
   - Configures `glGenTextures`, `glBindTexture`, `glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE)`, `glTexParameteri` (`GL_NEAREST` for crisp pixel art, `GL_REPEAT`/`GL_CLAMP_TO_EDGE`), `glTexImage2D`, and `glGenerateMipmap(GL_TEXTURE_2D)` (Module 8).
   - Automatically generates original pixel-art PNG textures in `assets/textures/` (`wood_floor.png`, `classroom_wall.png`, `desk_wood.png`, `chalkboard.png`, `window_glass.png`, `checker_debug.png`).
5. **Primitive & Stipple Renderer (`src/engine/renderer/PrimitiveRenderer.h`, `PrimitiveRenderer.cpp`):**
   - Renders `GL_POINTS` (`glPointSize`), `GL_LINES`, `GL_LINE_STRIP`, `GL_LINE_LOOP` (`glLineWidth`), `glLineStipple` (`0x00FF`, `0x0C0F`, `0xAAAA`), `GL_TRIANGLES`, `GL_TRIANGLE_STRIP`, `GL_TRIANGLE_FAN`, `GL_QUADS`, `GL_QUAD_STRIP`, and `GL_POLYGON` with `glColor3f`/`glColor4f`/`glColor3ub` (Modules 1 & 2).
6. **Vertex Array & VBO Mesh Renderer (`src/engine/renderer/MeshVBO.h`, `MeshVBO.cpp`):**
   - Uses `BUFFER_OFFSET(i)` macro and interleaved `Vertex3D` (position, normal, color, UV).
   - Supports client-side vertex array drawing (`drawClientSideElements`, Module 4), VBO `glDrawArrays` (`drawVBOArrays`, Module 5), VBO+EBO `glDrawElements` (`drawVBOElements`, Modules 4 & 5), VBO `glArrayElement` (`drawVBOArrayElement`, Module 6), and `glBufferSubData`.
7. **Camera & Projection System (`src/engine/camera/Camera3D.h`, `Camera3D.cpp`):**
   - Implements `glViewport`, `gluPerspective`, `glFrustum`, `glOrtho`, `gluOrtho2D`, and `gluLookAt` with interactive orbit/pan/zoom and projection cycling (`P` key, Module 7).
8. **Lighting & Material System (`src/engine/renderer/LightingSystem.h`, `LightingSystem.cpp`):**
   - Implements `GL_LIGHT0` (warm golden sunset light) and `GL_LIGHT1` (ceiling spotlight with `GL_SPOT_DIRECTION` and `GL_SPOT_CUTOFF`), `glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ...)`, `glMaterialfv` presets, and GLUT 3D solid/wire objects (`glutSolidTeapot`/`glutWireTeapot`, `glutSolidSphere`/`glutWireSphere`, `glutSolidCone`) with runtime toggles (`1`–`4`, Module 8).
9. **Bitmap & Stroke Text HUD (`src/engine/ui/TextRenderer.h`, `TextRenderer.cpp`):**
   - Renders 2D HUD panels, `glRasterPos2f` + `glutBitmapString`, and `glutStrokeString` (Module 3).

### Verification
- Built with Ninja & MSYS2 UCRT64 GCC 16.2.0 (`0 warnings, 0 errors`).
- Executed `.\build\NarrativeGame.exe --verify-phase1`:
  - Loaded all 6 PNG textures via `stbi_load`.
  - Created and rendered VBOs, EBOs, client-side vertex arrays, and mapped-buffer $4 \times 4$ transformations.
  - Verified `GL Errors = 0`.


---

## Phase 2 — 2.5D Character Prototype (2026-11-05)

### Completed Subsystems
1. **Pixel-Art Character Sprite-Sheet Generator (`src/engine/assets/Texture2D.cpp`):**
   - Generates original 4x2 (128x96, eight 32x48 frames) pixel-art character sprite sheets in `assets/sprites/player_elijah_sheet.png` and `assets/sprites/npc_maya_sheet.png` (Row 0 = 4-frame Idle breathing/blinking animation; Row 1 = 4-frame Walk cycle with swinging arms and stepping legs).
2. **Sprite-Sheet Animator (`src/engine/animation/SpriteAnimator.h`, `SpriteAnimator.cpp`):**
   - Tracks `AnimState::Idle` and `AnimState::Walk`, advances frame timers, handles horizontal UV flipping for Left/Right orientation, and computes normalized `UVRect` sub-rectangles.
3. **2.5D Character Quad & Grounded Contact Shadow (`src/engine/renderer/SpriteQuad3D.h`, `SpriteQuad3D.cpp`):**
   - Renders a dynamic-VBO character quad (`GL_DYNAMIC_DRAW`) in 3D space with per-frame UV updates via `glBufferSubData` (Module 6).
   - Uses `GL_ALPHA_TEST` (`glAlphaFunc(GL_GREATER, 0.1f)`) and `GL_BLEND` (`GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`) with `GL_NEAREST` pixel-art filtering so sprites have crisp edges and participate accurately in 3D depth sorting and Phong lighting.
   - Renders a grounded contact shadow ellipse underneath the character using `GL_TRIANGLE_FAN` (Module 2).
4. **3D AABB Collision & Sliding Resolution (`src/engine/collision/CollisionSystem.h`, `CollisionSystem.cpp`):**
   - Implements `AABB3D` and `CollisionWorld::resolveMovementXZ` with axis-separated X and Z collision checks so the player slides smoothly along room walls and classroom desks.
   - Renders stippled wireframe collider boxes (`glLineStipple`, Module 2) when `F3` Debug Mode is active, highlighting contacted colliders in bright red.
5. **Player Controller & 2.5D Camera Follow (`src/game/player/PlayerController.h`, `PlayerController.cpp`, `src/engine/camera/Camera3D.cpp`):**
   - Supports `WASD` and Arrow-key movement in 3D space, directional sprite flipping, walk/idle state transitions, and smooth 2.5D camera tracking (`Camera3D::setNarrativeTarget`), with `[C]` toggling between 2.5D Narrative Follow Camera and Free-Orbit Camera.

### Verification
- Built with Ninja & MSYS2 UCRT64 GCC 16.2.0 (`0 warnings, 0 errors`).
- Executed `.\build\NarrativeGame.exe --verify-phase2`:
  - Verified loading `assets/sprites/player_elijah_sheet.png` (`128x96`, `id=7`).
  - Verified walk cycle UV updates, camera follow, and sliding collision resolution against `StudentDesk_FrontLeft` with `GL Errors = 0`.


---

## Phase 3 — First Playable Scene (2026-11-05)

### Completed Subsystems
1. **NPC Character System (`src/game/characters/NPCCharacter.h`, `NPCCharacter.cpp`):**
   - Implements animated 2.5D NPCs (`Maya`) in 3D space with automatic player-facing orientation when nearby, grounded contact shadows, and AABB collider registration.
2. **Proximity Interaction System (`src/game/interactions/InteractionSystem.h`, `InteractionSystem.cpp`):**
   - Tracks 3D interactable objects and NPCs (`InteractionManager`), detects the closest target within trigger radius, renders a rotating 3D amber diamond marker (`GL_TRIANGLES`) and stippled ground ring (`GL_LINE_LOOP` + `glLineStipple`, Module 2), and displays a 2D interaction prompt banner (`[E] Talk to Maya`, `[E] Inspect Sketchbook & Notes`, etc.).
3. **Working Dialogue System (`src/game/dialogue/DialogueSystem.h`, `DialogueSystem.cpp`):**
   - Implements multi-line dialogue sequences with typewriter character reveal, speaker nameplate, line counter, and progression via `[E]`, `[Space]`, `[Enter]`, or Left Mouse Click (`GLUT_LEFT_BUTTON` / `GLUT_DOWN`, Module 3).
4. **Playable Classroom Scene (`src/game/scenes/ClassroomScene.h`, `ClassroomScene.cpp`):**
   - Encapsulates Room 304 (Late Afternoon, Nov 14) with textured walls, wood floor, chalkboard, sliding hallway door, 6 student desks + chairs, teacher's desk + chair, sketchbook prop, GLUT teapot & globe props, glowing ceiling lamp, sunset windows, dust motes, controllable player (`Elijah`), NPC (`Maya`), and 5 interactive targets.

### Verification
- Built with Ninja & MSYS2 UCRT64 GCC 16.2.0 (`0 warnings, 0 errors`).
- Executed `.\build\NarrativeGame.exe --verify-phase3`:
  - Verified loading all 8 textures/sprite-sheets, approaching an interactable target, triggering an interaction (`InteractionsTriggered=1`), and completing a full multi-line dialogue sequence (`DialoguesCompleted=1`) with `GL Errors = 0`.


---

## Phase 4 — Narrative and UI Systems (2026-11-05)

### Completed Subsystems
1. **Pixel-Art Character Portraits (`src/engine/assets/Texture2D.cpp`, `src/engine/ui/TextRenderer.cpp`):**
   - Generates original $48 \times 48$ pixel-art dialogue portraits (`assets/sprites/portrait_elijah.png`, `assets/sprites/portrait_maya.png`), loads them via `stb_image`, and renders them inside the dialogue box using `TextRenderer::drawTexturedQuad2D`.
2. **External Branching Story Data & Dialogue Choices (`src/game/dialogue/DialogueSystem.h`, `DialogueSystem.cpp`, `data/dialogue/story_dialogues.txt`):**
   - Separates narrative content into external data file `data/dialogue/story_dialogues.txt`.
   - Supports multi-option branching dialogue choices (`[1]`, `[2]`, or Up/Down arrows + `[E]`/`[Enter]`) that jump to target dialogue nodes and mutate story flags.
3. **Story Flags & Save/Load System (`src/game/narrative/StoryState.h`, `StoryState.cpp`, `data/saves/save_slot_1.sav`):**
   - Tracks story progression flags (`has_sketchbook`, `talked_to_maya`, `promised_rooftop_rehearsal`, `replied_phone_sam`, `replied_phone_maya`, `hallway_unlocked`), dynamic objective prompts, and HUD status toasts.
   - Implements Quick Save (`[F5]`) and Quick Load (`[F9]`) reading/writing `data/saves/save_slot_1.sav`.
4. **Interactive Smartphone Messaging Overlay (`src/game/narrative/PhoneSystem.h`, `PhoneSystem.cpp`, `data/dialogue/phone_messages.txt`):**
   - Loads external chat threads (`Sam (Band Club)`, `Maya Lin`) from `data/dialogue/phone_messages.txt`.
   - Renders an unread badge pill on the HUD and a full smartphone handset UI (`[Tab]` / `[B]`) with interactive reply choices (`[1]`, `[2]`) and thread switching (`[N]`).
5. **Cinematic Scene Transitions (`src/engine/scene/SceneTransition.h`, `SceneTransition.cpp`):**
   - Renders smooth fade-to-black + animated letterbox bars with midpoint callbacks and vector stroke title banners.

### Verification
- Built with Ninja & MSYS2 UCRT64 GCC 16.2.0 (`0 warnings, 0 errors`).
- Executed `.\build\NarrativeGame.exe --verify-phase4`:
  - Verified loading portraits (`id=9`, `id=10`), external story & phone data files, branching choice selection (`Flag[promised_rooftop_rehearsal]=1`), smartphone reply (`Flag[replied_phone_sam]=1`), automatic hallway unlock (`Flag[hallway_unlocked]=1`), save/load round-trip to `data/saves/save_slot_1.sav`, and scene transition with `GL Errors = 0`.


---

## Phase 5 — Additional Locations (2026-11-05)

### Completed Subsystems
1. **New Environment & Character Assets (`src/engine/assets/Texture2D.cpp`):**
   - Generated pixel-art environment textures: `assets/textures/hallway_tile.png` (terrazzo corridor tiles), `assets/textures/school_lockers.png` (steel-blue lockers), `assets/textures/wet_asphalt.png` (rain-slicked street & crosswalk), and `assets/textures/storefront_neon.png` (24/7 Sundown Mart neon canopy & storefront).
   - Generated sprite sheet `assets/sprites/npc_sam_sheet.png` ($128 \times 96$) and portrait `assets/sprites/portrait_sam.png` ($48 \times 48$) for **Sam (Band Club)**.
2. **Module 2 Rain Particle & Ground Puddle Ripple System (`src/engine/renderer/PrimitiveRenderer.cpp`):**
   - Implemented `PrimitiveRenderer::drawRainAndRipples` using `GL_LINES` + `glLineStipple` (`0x00FF` dashed rain streaks) and expanding `GL_LINE_LOOP` ground puddle splash rings.
3. **3rd-Floor School Hallway & Rainy Convenience-Store Street (`src/game/scenes/ClassroomScene.h`, `ClassroomScene.cpp`):**
   - Added `LocationId::Hallway` (terrazzo floor, corridor walls, 2 steel-blue locker banks, campus noticeboard, waiting bench, doors to Room 304 and street stairwell, and NPC **Sam**).
   - Added `LocationId::RainyStreet` (wet asphalt road, 24/7 Sundown Mart storefront & neon awning, bus-stop bench + acoustic guitar case prop, cast-iron streetlight with glowing sphere lamp head, rain & puddle ripples, and NPC **Maya** under the awning).
   - Connected all three locations via interactive doors/stairs with `SceneTransition` and added quick-travel keys `[F6]`, `[F7]`, and `[F8]`.

### Verification
- Built with Ninja & MSYS2 UCRT64 GCC 16.2.0 (`0 warnings, 0 errors`).
- Executed `.\build\NarrativeGame.exe --verify-phase5`:
  - Verified loading all 16 textures, sprite sheets, and portraits.
  - Verified narrative sequence across all 3 locations (`LocationsVisitedMask=0x7`: `Classroom` -> `Hallway` -> `RainyStreet`), completing Sam's and Maya's branching dialogues (`Flag[talked_to_sam]=1`, `Flag[ready_for_minigame]=1`) with `GL Errors = 0`.


---

## Phase 6 — Minigame (2026-11-05)

### Completed Subsystems
1. **Acoustic Guitar Rhythm Minigame (`src/game/minigames/RhythmMinigame.h`, `RhythmMinigame.cpp`):**
   - Connected directly to the narrative when inspecting **Sam's Acoustic Guitar Case** on the bus-stop bench outside 24/7 Sundown Mart (`LocationId::RainyStreet`), or triggered anytime via `[G]`.
   - Uses FreeGLUT keyboard (`[D/F/J/K]` or `[1/2/3/4]`) and timer callbacks (`update(dt)`).
   - Renders fretboard string lanes, stippled fret wires and acoustic soundhole rosette (`GL_LINES`, `GL_LINE_LOOP`, `glLineStipple` with `0x00FF` and `0x0C0F`), target receptor glow bursts (`GL_TRIANGLE_FAN`), scrolling diamond note gems (`GL_POLYGON` with smooth vertex color interpolation), and hit burst particle stars (`GL_POINTS` + `glPointSize`).
   - Uses OpenGL matrix stack transformations (`glPushMatrix`, `glTranslatef`, `glRotatef`, `glScalef`, `glPopMatrix`) to animate scrolling, rotating, and pulsing note gems.
   - Returns cleanly to the 2.5D story on completion, setting `completed_rhythm_minigame = 1` and `rhythm_high_score` in `StoryState` and launching Maya's post-song celebration dialogue (`post_minigame_celebration`).

### Verification
- Built with Ninja & MSYS2 UCRT64 GCC 16.2.0 (`0 warnings, 0 errors`).
- Executed `.\build\NarrativeGame.exe --verify-phase6`:
  - Verified full 16-note rhythm chart execution (`MinigameNotesHit=16/16`, `MaxCombo=16`, `Score=4440`), completion flag persistence (`Flag[completed_rhythm_minigame]=1`), and post-minigame story dialogue completion (`Flag[story_act1_complete]=1`) with `GL Errors = 0`.


---

## Phase 7 — OpenGL Demonstration Mode (2026-11-05)

### Completed Subsystems
1. **Interactive 8-Station OpenGL Coursework Showcase (`src/debug/OpenGLDemoMode.h`, `OpenGLDemoMode.cpp`):**
   - Toggled anytime via `[F2]` (or from the Main Menu) and navigated with `[1..8]` or `[Left / Right Arrow]` keys.
   - Displays on screen:
     1. Which CS0045 Module (1–8) is currently active.
     2. The exact OpenGL & FreeGLUT API functions demonstrated at that station.
     3. Where the exact same concept is used in the narrative game (`src/...` file and function names).
   - **Stations 1–8 Coverage:**
     - **Station 1 (Module 1):** Rendering pipeline stage diagram + immediate-mode `GL_POLYGON` (`glBegin`, `glVertex2f`, `glEnd`, `glClear`, `glFlush`).
     - **Station 2 (Module 2):** All 10 primitive types (`GL_POINTS`..`GL_POLYGON`), all 3 `glLineStipple` patterns (`0x00FF`, `0x0C0F`, `0xAAAA`), rain streaks, puddle ripples, and per-vertex colors (`glColor3ub`).
     - **Station 3 (Module 3):** Live GLUT keyboard/mouse/motion/entry callback inspector + 3D `glutStrokeString` and 2D `glutBitmapString` font comparison.
     - **Station 4 (Module 4):** Client-side vertex arrays (`glEnableClientState`, `glVertexPointer`, `glColorPointer`, `glNormalPointer`, `glTexCoordPointer`, `glDrawElements`).
     - **Station 5 (Module 5):** Static VBO (`GL_STATIC_DRAW`) vs. Dynamic Mapped VBO (`GL_DYNAMIC_DRAW`, `glGenBuffers`, `glIsBuffer`, `glBindBuffer`, `glBufferData`, `glMapBuffer`).
     - **Station 6 (Module 6):** Live homogeneous coordinate math (`Vec4` $w=1$ vs. $w=0$, `Mat4` $4 \times 4$ matrix-vector product), `BUFFER_OFFSET(i)`, `glArrayElement`, and mapped-VBO vertex translation.
     - **Station 7 (Module 7):** 3-level nested matrix stack planetary hierarchy (`glPushMatrix`, `glRotatef`, `glTranslatef`, `glScalef`, `glPopMatrix`) + live `gluPerspective` / `glFrustum` / `glOrtho` projection switching (`[P]`).
     - **Station 8 (Module 8):** Phong lighting + spotlight (`GL_LIGHT0`, `GL_LIGHT1`), materials (`glMaterialfv`), GLUT 3D objects (`glutSolidTeapot`/`glutWireTeapot`, `glutSolidTorus`), and mipmapped `stb_image` textures.

### Verification
- Built with Ninja & MSYS2 UCRT64 GCC 16.2.0 (`0 warnings, 0 errors`).
- Executed `.\build\NarrativeGame.exe --verify-phase7`:
  - Stepped through and rendered all 8 Module stations (`DemoModulesVisitedMask=0xff (All8=YES)`) with `GL Errors = 0`.
