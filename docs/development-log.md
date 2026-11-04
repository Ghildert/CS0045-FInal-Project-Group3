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
