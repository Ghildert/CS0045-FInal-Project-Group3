# OpenGL Coursework Requirements Traceability Matrix (Modules 1–8)

This document cross-references every concept, function, constant, and code pattern from the eight CS0045 lecture PDFs (`Module 01.pdf` – `Module 08.pdf`) against our 2.5D narrative adventure game (**"Echoes of November"**) and its built-in **OpenGL Coursework Demonstration Mode**.

---

## Technical Conflicts & Lecture Errata Resolved in Architecture

During the Phase 0 audit of `Module 01.pdf` through `Module 08.pdf`, several important technical considerations and slide errata were identified and resolved:

1. **`glutInitDisplayMode` Call Order (Module 7 Slides 29 & 38):**
   - *Issue in slides:* `glutCreateWindow("simple")` is called before `glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH)` and `glutInitWindowSize(800, 600)`. In FreeGLUT, display mode and window size must be configured **before** creating the window.
   - *Resolution:* Our initialization sequence strictly calls `glutInit()` -> `glutInitDisplayMode()` -> `glutInitWindowSize()` -> `glutInitWindowPosition()` -> `glutCreateWindow()`.
2. **Raster Position Color Latching (Module 3 Slide 8):**
   - *Rule in slides:* `glColor3f()` / `glColor4f()` **must** be called *before* `glRasterPos2f(x, y)` because OpenGL latches the current color into the raster position state at the moment `glRasterPos*` is invoked.
   - *Resolution:* All `glutBitmapString()` rendering helpers set `glColor*` prior to `glRasterPos2f()`.
3. **Per-Frame GPU Resource Allocation (Module 5 Slide 19 & Module 8 Slide 32):**
   - *Issue in slides:* Sample code calls `glGenBuffers()`, `glGenTextures()`, and `stbi_load()` inside the `display()` callback on every redraw.
   - *Resolution:* Game scenes allocate VBOs and textures once during initialization/loading and clean them up with `glDeleteBuffers()` / `glDeleteTextures()`.
4. **`GL_ELEMENT_ARRAY_BUFFER` vs. `GL_NORMAL_ARRAY` (Module 5 Slides 21–23):**
   - *Issue in slides:* Slide 22–23 enables `GL_NORMAL_ARRAY` and calls `glNormalPointer(GL_UNSIGNED_BYTE, 0, 0)` when demonstrating `GL_ELEMENT_ARRAY_BUFFER` indices.
   - *Resolution:* We implement both properly: `GL_ELEMENT_ARRAY_BUFFER` for indexed `glDrawElements()` calls, and `GL_NORMAL_ARRAY` / `glNormalPointer(GL_FLOAT, 0, ...)` for 3D surface normals used by Module 8 Phong lighting, plus an exact recreation of the Module 5 indexed VBO slide in the Developer Demo Mode.
5. **Single Buffering (`glFlush`) vs. Smooth Gameplay (`glutSwapBuffers`):**
   - *Resolution:* We initialize with `GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH` for tear-free 60 FPS animation while still calling `glFlush()` prior to `glutSwapBuffers()` to satisfy Module 1 pipeline flushing semantics.
6. **Compatibility Profile Context:**
   - *Requirement:* Modules 1–8 combine fixed-function APIs (`glBegin`/`glEnd`, `glLineStipple`, `glRasterPos2f`, `glMatrixMode`, `glLightfv`, `glMaterialfv`, `glArrayElement`), client-side vertex arrays (`glEnableClientState`, `glVertexPointer`), VBO APIs (`glGenBuffers`, `glBufferData`, `glBufferSubData`, `glMapBuffer`), and modern extensions (`glGenerateMipmap`, `glDebugMessageCallback`).
   - *Resolution:* We use a Compatibility Profile OpenGL context via FreeGLUT and GLEW (`GLEW_STATIC`), verifying support at startup and printing `GL_VERSION`, `GL_VENDOR`, and `GL_RENDERER`.

---

## Module-by-Module Traceability Table

### Module 1 — Introduction to Computer Graphics & Simple OpenGL Program

| Subtopic | Required Concept | OpenGL / GLUT Functions & Constants | Source File & Function | Gameplay Feature | How to Demonstrate | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| M1.S1 | Rendering Pipeline & Diagnostics | `glGetString(GL_VERSION)`, `glGetString(GL_VENDOR)`, `glGetString(GL_RENDERER)` | `src/main.cpp` (`initOpenGLDiagnostics`) | Startup console diagnostics & F1 Debug Overlay | Launch game; inspect console & press `F1` | Verified (Phase 0) |
| M1.S2 | GLUT Window Initialization | `glutInit`, `glutInitWindowSize`, `glutInitWindowPosition`, `glutCreateWindow` | `src/main.cpp` (`main`) | Game window creation (1280x720 default) | Launch executable | Verified (Phase 0) |
| M1.S2 | Clear & Event Loop | `glClearColor`, `glClear(GL_COLOR_BUFFER_BIT)`, `glFlush`, `glutDisplayFunc`, `glutMainLoop` | `src/main.cpp` (`displayCallback`, `main`) | Main rendering loop & frame buffer clearing | Launch executable; observe rendered frame | Verified (Phase 0) |
| M1.S2 | First Polygon Square | `glBegin(GL_POLYGON)`, `glVertex2f`, `glEnd` | `src/main.cpp` (`renderPhase0VerificationScene`) & Demo Mode | UI panel backgrounds & Module 1 Demo | Press `F2` -> Select Module 1 Demo | Verified (Phase 0) |

### Module 2 — Geometric Specification, Primitives, Line Stippling, and Colors

| Subtopic | Required Concept | OpenGL / GLUT Functions & Constants | Source File & Function | Gameplay Feature | How to Demonstrate | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| M2.S1 | Points & Point Size | `GL_POINTS`, `glPointSize`, `glVertex2f` / `glVertex3f` | `src/main.cpp` / `src/engine/renderer/` | Dust motes in classroom sunbeams, fireflies/rain splashes | Classroom scene sunbeams; Demo Mode M2 | Verified (Phase 0) |
| M2.S1 | Line Primitives & Width | `GL_LINES`, `GL_LINE_STRIP`, `GL_LINE_LOOP`, `glLineWidth` | `src/main.cpp` / `src/engine/ui/` | Rain streaks, dialogue box borders, interactable object outlines | Rainy street scene; UI dialogue frames; Demo Mode M2 | Verified (Phase 0) |
| M2.S1 | Line Stippling Patterns | `glEnable(GL_LINE_STIPPLE)`, `glLineStipple(factor, pattern)`, `glDisable(GL_LINE_STIPPLE)` (`0x00FF`, `0x0C0F`, `0xAAAA`) | `src/main.cpp` / `src/game/minigames/` | Dashed rhythm-lane guides in minigame, interaction range indicator, debug bounds | Minigame & `F2` Demo Mode M2 (cycles patterns/factors) | Verified (Phase 0) |
| M2.S2 | Filled Primitives | `GL_TRIANGLES`, `GL_TRIANGLE_STRIP`, `GL_TRIANGLE_FAN`, `GL_QUADS`, `GL_QUAD_STRIP`, `GL_POLYGON` | `src/main.cpp` / `src/engine/renderer/` | Architectural trim, light cones (`GL_TRIANGLE_FAN`), floor/wall segments (`GL_QUADS`) | All 3D scenes & Demo Mode M2 primitive showcase | Verified (Phase 0) |
| M2.S2 | RGBA & Unsigned Byte Colors | `glColor3f`, `glColor4f`, `glColor3ub` (e.g., `glColor3ub(92,181,100)`), vertex color gradients | `src/main.cpp` / `src/engine/renderer/` | Sky gradients, atmospheric lighting tints, UI transparency | Sunset sky backdrop through classroom windows; Demo Mode M2 | Verified (Phase 0) |

### Module 3 — GLUT Extensions (Text & Raster Position) and GLUT Callbacks

| Subtopic | Required Concept | OpenGL / GLUT Functions & Constants | Source File & Function | Gameplay Feature | How to Demonstrate | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| M3.S1 | Raster Position & Bitmap Text | `glRasterPos2f` (called *after* `glColor*`), `glutBitmapString` (`<GL/freeglut_ext.h>`), `GLUT_BITMAP_HELVETICA_18`, `GLUT_BITMAP_HELVETICA_12`, `GLUT_BITMAP_TIMES_ROMAN_24`, `GLUT_BITMAP_9_BY_15`, `GLUT_BITMAP_8_BY_13` | `src/main.cpp` / `src/engine/ui/` | Dialogue text, speaker names, phone UI messages, HUD prompts, debug overlay | Talk to NPCs, open phone UI, or view HUD overlay | Verified (Phase 0) |
| M3.S1 | Stroke Font Text | `GLUT_STROKE_ROMAN`, `GLUT_STROKE_MONO_ROMAN`, `glutStrokeString` / `glutStrokeCharacter` | `src/engine/ui/` & Demo Mode M3 | 3D world-space signage (classroom number plaque, store sign) & title header | Look at classroom door sign / convenience store sign; Demo Mode M3 | Verified (Phase 0) |
| M3.S2 | Window & Redisplay Control | `glutFullScreen`, `glutReshapeWindow`, `glutPostRedisplay` | `src/main.cpp` | `F11` Fullscreen toggle & continuous event-driven redraws | Press `F11` to toggle fullscreen | Verified (Phase 0) |
| M3.S2 | Keyboard Callbacks | `glutKeyboardFunc`, `glutKeyboardUpFunc`, `glutSpecialFunc`, `glutSpecialUpFunc` | `src/main.cpp` / `src/engine/input/` | `WASD`/Arrow movement, `E` interact, `1-3` dialogue choices, `ESC` pause | Move player with `WASD`, interact with `E` | Verified (Phase 0) |
| M3.S2 | Mouse Callbacks | `glutMouseFunc` (`GLUT_LEFT_BUTTON`, `GLUT_MIDDLE_BUTTON`, `GLUT_RIGHT_BUTTON`, `GLUT_DOWN`, `GLUT_UP`) | `src/main.cpp` / `src/engine/input/` | Clicking menu buttons, advancing dialogue, clicking phone UI | Click menu items or dialogue choices with mouse | Verified (Phase 0) |
| M3.S2 | Mouse Motion & Entry Callbacks | `glutMotionFunc`, `glutPassiveMotionFunc`, `glutEntryFunc` (`GLUT_LEFT`, `GLUT_ENTERED`) | `src/main.cpp` / `src/engine/input/` | UI button hover highlights, free-camera orbit drag in debug/demo mode, auto-pause notice when mouse leaves window | Move/drag mouse in menus or Demo Mode; move cursor out of window | Verified (Phase 0) |
| M3.S2 | Timer & Idle Callbacks | `glutTimerFunc`, `glutIdleFunc` | `src/main.cpp` | 60 FPS game update tick, sprite animation timers, typewriter text effect | Observe animated sprites and timer-driven animations | Verified (Phase 0) |

### Module 4 — Vertex Arrays (`glDrawArrays`, `glDrawElements`, Color Arrays)

| Subtopic | Required Concept | OpenGL / GLUT Functions & Constants | Source File & Function | Gameplay Feature | How to Demonstrate | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| M4.S1 | Client-Side Vertex Arrays | `glEnableClientState(GL_VERTEX_ARRAY)`, `glVertexPointer`, `glDrawArrays`, `glDisableClientState(GL_VERTEX_ARRAY)` | `src/engine/renderer/` & Demo Mode M4 | Procedural room structural geometry (walls, blackboard, window frames) | Classroom scene & Demo Mode M4 | Verified (Phase 0) |
| M4.S1 | Indexed Vertex Arrays | `glDrawElements(mode, count, GL_UNSIGNED_BYTE / GL_UNSIGNED_SHORT, indices)` | `src/engine/renderer/` & Demo Mode M4 | Reusable desk/chair/locker meshes sharing indexed vertices | Classroom furniture & Demo Mode M4 | Verified (Phase 0) |
| M4.S2 | Color Arrays | `glEnableClientState(GL_COLOR_ARRAY)`, `glColorPointer`, `glDisableClientState(GL_COLOR_ARRAY)` | `src/engine/renderer/` & Demo Mode M4 | Multi-colored pinwheel / stained glass / vertex-colored props | Demo Mode M4 (exact 4-triangle pinwheel + green bar from slides 17–22) | Verified (Phase 0) |

### Module 5 — Vertex Buffer Objects (VBOs, Indexed VBOs, `glMapBuffer`)

| Subtopic | Required Concept | OpenGL / GLUT Functions & Constants | Source File & Function | Gameplay Feature | How to Demonstrate | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| M5.S1 | GLEW Initialization | `#define GLEW_STATIC`, `<GL/glew.h>`, `glewInit()`, `GLEW_OK` | `src/main.cpp` (`main`) | Loads OpenGL 1.5–4.5 VBO and debug extensions | Verified on startup | Verified (Phase 0) |
| M5.S1 | Static & Dynamic VBOs | `glGenBuffers`, `glIsBuffer`, `glBindBuffer(GL_ARRAY_BUFFER, id)`, `glBufferData` (`GL_STATIC_DRAW`, `GL_DYNAMIC_DRAW`, `GL_STREAM_DRAW`), `glDeleteBuffers` | `src/engine/renderer/` & Demo Mode M5 | GPU-buffered 3D room meshes, props, and character sprite quads | All 3D scenes & Demo Mode M5 | Verified (Phase 0) |
| M5.S2 | Indexed VBOs | `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id)`, `glBufferData(GL_ELEMENT_ARRAY_BUFFER, ...)`, `glDrawElements(..., 0)` | `src/engine/renderer/` & Demo Mode M5 | Indexed 3D environment props and tiled floors | Classroom/Hallway scenes & Demo Mode M5 | Verified (Phase 0) |
| M5.S2 | Mapped Buffer Updates | `glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE)`, `glUnmapBuffer(GL_ARRAY_BUFFER)` | `src/engine/renderer/` & Demo Mode M5 | Dynamic rippling puddle vertices / swaying curtain / animated polygon | Rainy street puddle ripples & Demo Mode M5/M6 | Verified (Phase 0) |

### Module 6 — Matrix Operations, 3D Objects, `glBufferSubData`, `glArrayElement`, Error Callback

| Subtopic | Required Concept | OpenGL / GLUT Functions & Constants | Source File & Function | Gameplay Feature | How to Demonstrate | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| M6.S1 | 4x4 Matrix Math & Homogeneous Coords | Custom $4 \times 4$ translation/scaling/rotation matrix multiplication on `(x, y, z, w)` with `glMapBuffer` | `src/engine/core/Math4x4.h` & Demo Mode M6 | CPU-side matrix transformation of dynamic VBO vertices (Slide 22 `translate()` implementation) | Demo Mode M6 (animated translating hexagon via 4x4 matrix + `glMapBuffer`) | Planned (Phase 1 / Demo) |
| M6.S2 | 3D Cube & Pyramid Primitives | 3D Cube (`GL_QUADS`, 24 verts) and 3D Pyramid (`GL_TRIANGLES`, 12 verts) | `src/engine/renderer/` & Demo Mode M6 | 3D desks, lockers, convenience store counters, street lamps | All 3D scenes & Demo Mode M6 | Verified (Phase 0) |
| M6.S2 | Sub-Buffer Updates & `BUFFER_OFFSET` | `#define BUFFER_OFFSET(i) ((void*)(i))`, `glBufferSubData`, `glArrayElement(i)` | `src/engine/renderer/` & Demo Mode M6/M8 | Combined vertex + color + UV VBOs rendered via `glArrayElement` & `glDrawArrays` | Demo Mode M6 & M8 | Verified (Phase 0) |
| M6.S2 | OpenGL Debug Output Callback | `glEnable(GL_DEBUG_OUTPUT)`, `glDebugMessageCallback(MessageCallback, 0)`, `GL_DEBUG_TYPE_ERROR` | `src/main.cpp` (`MessageCallback`) | Real-time OpenGL error interception and logging | Active throughout runtime; logs to console | Verified (Phase 0) |

### Module 7 — Transformations, Viewing, Projections, and Depth Testing

| Subtopic | Required Concept | OpenGL / GLUT Functions & Constants | Source File & Function | Gameplay Feature | How to Demonstrate | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| M7.S1 | Matrix Modes, Stack & Viewport | `glMatrixMode(GL_MODELVIEW / GL_PROJECTION)`, `glLoadIdentity`, `glPushMatrix`, `glPopMatrix`, `glViewport` | `src/main.cpp` / `src/engine/camera/` | Window resize handler, hierarchical scene-graph rendering (desk + props, swinging door, rotating fan) | Resize window; observe hierarchical props in classroom | Verified (Phase 0) |
| M7.S1 | Modeling Transformations | `glTranslatef`, `glRotatef`, `glScalef` | `src/main.cpp` / `src/engine/renderer/` | Placing, orienting, and scaling all 3D furniture, characters, and animated ceiling fans/doors | All 3D scenes & Demo Mode M7 | Verified (Phase 0) |
| M7.S2 | Perspective & Orthographic Projections | `gluPerspective`, `glFrustum`, `glOrtho`, `gluOrtho2D` | `src/engine/camera/` & `src/engine/ui/` | `gluPerspective` for 2.5D 3D world; `glOrtho`/`gluOrtho2D` for 2D UI/dialogue overlays; `glFrustum` in Demo Mode | Toggle Perspective vs. Orthographic in Demo Mode M7 (`P` key) | Verified (Phase 0) |
| M7.S2 | Depth Buffer & Camera Viewing | `GLUT_DEPTH`, `glEnable(GL_DEPTH_TEST)`, `glClear(GL_DEPTH_BUFFER_BIT)`, `gluLookAt` | `src/main.cpp` / `src/engine/camera/` | 2.5D cinematic follow camera with `gluLookAt` and proper Z-buffer occlusion of 2D characters behind 3D desks | Walk behind/in front of desks in classroom; orbit camera in Demo Mode | Verified (Phase 0) |

### Module 8 — Lighting and Texture Mapping

| Subtopic | Required Concept | OpenGL / GLUT Functions & Constants | Source File & Function | Gameplay Feature | How to Demonstrate | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| M8.S1 | Phong Lighting & Global Ambient | `glEnable(GL_LIGHTING)`, `glEnable(GL_LIGHT0..7)`, `glGetIntegerv(GL_MAX_LIGHTS, &n)`, `glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ...)` | `src/engine/renderer/` & Demo Mode M8 | Warm afternoon classroom sunlight, cool hallway fluorescent lights, night street lamps | All 3D scenes & Demo Mode M8 lighting toggles | Verified (Phase 0) |
| M8.S1 | Light Sources & Spotlights | `glLightfv` / `glLightf` (`GL_AMBIENT`, `GL_DIFFUSE`, `GL_SPECULAR`, `GL_POSITION`, `GL_SPOT_DIRECTION`, `GL_SPOT_CUTOFF`) | `src/engine/renderer/` & Demo Mode M8 | Directional sunlight (`w=0`), point lamps (`w=1`), overhead spotlight cones (`GL_SPOT_CUTOFF`) | Streetlight/desk spotlight & Demo Mode M8 (Teapot with `GL_LIGHT0` + `GL_LIGHT1` spotlight) | Verified (Phase 0) |
| M8.S1 | Material Properties | `glMaterialfv` / `glMaterialf` (`GL_FRONT`, `GL_BACK`, `GL_FRONT_AND_BACK`, `GL_DIFFUSE`, `GL_SPECULAR`, `GL_AMBIENT`, `GL_EMISSION`, `GL_SHININESS`) | `src/engine/renderer/` & Demo Mode M8 | Shiny polished floors, matte wooden desks, glowing vending machine/phone screen (`GL_EMISSION`) | Classroom/Street scenes & Demo Mode M8 | Verified (Phase 0) |
| M8.S1 | GLUT 3D Geometric Objects | `glutSolidTeapot`/`glutWireTeapot`, `glutSolidSphere`/`glutWireSphere`, `glutSolidCube`/`glutWireCube`, `glutSolidCone`/`glutWireCone`, `glutSolidTorus`/`glutWireTorus`, etc. | `src/engine/renderer/` & Demo Mode M8 | Classroom globe (`glutSolidSphere`), art-desk teapot (`glutSolidTeapot`), lamp shades (`glutSolidCone`), Demo Mode showcase | Classroom props & Demo Mode M8 | Verified (Phase 0) |
| M8.S2 | Texture Loading & Filtering | `stb_image.h` (`stbi_load`, `stbi_image_free`), `glEnable(GL_TEXTURE_2D)`, `glTexEnvf(..., GL_MODULATE)`, `glGenTextures`, `glBindTexture`, `glTexParameteri` (`GL_REPEAT`, `GL_NEAREST`, `GL_LINEAR`), `glTexImage2D`, `glGenerateMipmap` | `src/engine/assets/` & Demo Mode M8 | Pixel-art character sprites (`GL_NEAREST` + alpha blending) and 3D room/wood/brick textures | Character animations, textured walls/floors, & Demo Mode M8 textured cube | Verified (Phase 0) |
| M8.S2 | Buffered Texture Coordinates | `glEnableClientState(GL_TEXTURE_COORD_ARRAY)`, `glTexCoordPointer`, `glBufferSubData` | `src/engine/renderer/` & Demo Mode M8 | UV-mapped 3D environment cubes and animated sprite-sheet UV quads | All textured 3D objects & Demo Mode M8 | Verified (Phase 0) |
