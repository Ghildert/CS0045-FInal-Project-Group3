/**
 * @file main.cpp
 * @brief Phase 0 — OpenGL Compatibility Context, FreeGLUT Window, VBO, and Diagnostics Verification
 *
 * Course: CS0045 — Introduction to Computer Graphics and Visual Computing
 * Project: Echoes of November (2.5D Narrative Adventure Game)
 *
 * This Phase 0 build verifies:
 *  - Module 1: FreeGLUT window initialization, display loop, glClear, glFlush, pipeline diagnostics
 *  - Module 2: Geometric primitives (GL_POINTS, GL_LINES, GL_LINE_LOOP, GL_QUADS, GL_POLYGON),
 *              glLineWidth, glPointSize, glLineStipple (0xAAAA, 0x00FF), glColor3f/4f/3ub
 *  - Module 3: glRasterPos2f + glutBitmapString (<GL/freeglut_ext.h>), GLUT_STROKE_ROMAN,
 *              keyboard, mouse, motion, passive motion, entry, and timer callbacks
 *  - Module 4 & 5: GLEW initialization, VBO creation (glGenBuffers, glBindBuffer, glBufferData),
 *              Vertex & Color arrays (glEnableClientState, glVertexPointer, glColorPointer, glDrawArrays)
 *  - Module 6: BUFFER_OFFSET macro, glBufferSubData, glArrayElement, glDebugMessageCallback
 *  - Module 7: glViewport, glMatrixMode(GL_PROJECTION / GL_MODELVIEW), glPushMatrix/glPopMatrix,
 *              glTranslatef/glRotatef/glScalef, gluPerspective, glOrtho, gluLookAt, GL_DEPTH_TEST
 */

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#define BUFFER_OFFSET(i) ((void*)(i))

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

namespace {

// Window dimensions
int g_windowWidth = 1280;
int g_windowHeight = 720;
bool g_isFullscreen = false;

// Verification & diagnostic states
bool g_autoVerifyMode = false;
int g_renderedFrames = 0;
int g_glErrorCount = 0;
bool g_showDiagnosticsOverlay = true;
bool g_useOrthographic = false;

// Animation & camera state (Module 3 & Module 7)
float g_rotationAngle = 25.0f;
float g_cameraYaw = 0.0f;
float g_cameraPitch = 18.0f;
int g_lastMouseX = 0;
int g_lastMouseY = 0;
bool g_mouseLeftDown = false;
std::string g_lastInputEvent = "None (Press WASD, P, F1, or click/drag mouse)";

// GPU Vertex Buffer Object ID (Module 5 & Module 6)
GLuint g_vboId = 0;
bool g_vboInitialized = false;

// 3D Pyramid (12 vertices = 36 floats) + 3D Cube (24 vertices = 72 floats) from Module 6 Slide 36
const GLfloat kGeometryVertices[] = {
    // --- PYRAMID (4 triangles, 12 vertices) ---
    // Front face
     0.0f,  1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    // Right face
     0.0f,  1.0f,  0.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
    // Back face
     0.0f,  1.0f,  0.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    // Left face
     0.0f,  1.0f,  0.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,

    // --- CUBE (6 quads, 24 vertices) ---
    // Top face (y = 1.0f)
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    // Bottom face (y = -1.0f)
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    // Front face (z = 1.0f)
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    // Back face (z = -1.0f)
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    // Left face (x = -1.0f)
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    // Right face (x = 1.0f)
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f
};

// Vertex colors for Pyramid (36 floats) + Cube (72 floats) from Module 6 Slides 42 & 45
const GLfloat kGeometryColors[] = {
    // Pyramid colors (smooth RGB per-vertex interpolation)
    1.0f, 0.25f, 0.25f,   0.25f, 0.9f, 0.4f,   0.25f, 0.5f, 1.0f,
    1.0f, 0.25f, 0.25f,   0.25f, 0.5f, 1.0f,   0.25f, 0.9f, 0.4f,
    1.0f, 0.25f, 0.25f,   0.25f, 0.9f, 0.4f,   0.25f, 0.5f, 1.0f,
    1.0f, 0.25f, 0.25f,   0.25f, 0.5f, 1.0f,   0.25f, 0.9f, 0.4f,

    // Cube colors (6 solid face colors)
    0.20f, 0.80f, 0.35f,  0.20f, 0.80f, 0.35f,  0.20f, 0.80f, 0.35f,  0.20f, 0.80f, 0.35f, // Top: Green
    0.95f, 0.55f, 0.15f,  0.95f, 0.55f, 0.15f,  0.95f, 0.55f, 0.15f,  0.95f, 0.55f, 0.15f, // Bottom: Orange
    0.90f, 0.25f, 0.25f,  0.90f, 0.25f, 0.25f,  0.90f, 0.25f, 0.25f,  0.90f, 0.25f, 0.25f, // Front: Red
    0.95f, 0.85f, 0.20f,  0.95f, 0.85f, 0.20f,  0.95f, 0.85f, 0.20f,  0.95f, 0.85f, 0.20f, // Back: Yellow
    0.25f, 0.45f, 0.95f,  0.25f, 0.45f, 0.95f,  0.25f, 0.45f, 0.95f,  0.25f, 0.45f, 0.95f, // Left: Blue
    0.85f, 0.30f, 0.85f,  0.85f, 0.30f, 0.85f,  0.85f, 0.30f, 0.85f,  0.85f, 0.30f, 0.85f  // Right: Magenta
};

/**
 * @brief Module 6 Slide 38: OpenGL Debug Output callback for catching driver/API errors.
 */
void GLAPIENTRY MessageCallback(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar* message,
                                const void* userParam) {
    (void)source;
    (void)id;
    (void)length;
    (void)userParam;
    if (type == GL_DEBUG_TYPE_ERROR) {
        ++g_glErrorCount;
        std::fprintf(stderr, "GL CALLBACK: ** GL ERROR ** type = 0x%x, severity = 0x%x, message = %s\n",
                     type, severity, message);
    }
}

/**
 * @brief Module 3 Slide 8: Renders a bitmap string at 2D screen coordinates.
 * Note: Color MUST be set before glRasterPos2f() because OpenGL latches the current color into the raster state.
 */
void drawBitmapText2D(float x, float y, void* font, const char* text, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    glutBitmapString(font, reinterpret_cast<const unsigned char*>(text));
}

/**
 * @brief Module 5 & Module 6: Allocates and populates the VBO using glBufferData and glBufferSubData.
 */
void initPhase0VBO() {
    if (g_vboInitialized) {
        return;
    }
    glGenBuffers(1, &g_vboId);
    glBindBuffer(GL_ARRAY_BUFFER, g_vboId);

    const GLsizeiptr vertexBytes = sizeof(kGeometryVertices);
    const GLsizeiptr colorBytes = sizeof(kGeometryColors);

    // Allocate combined VBO and upload vertices, then upload colors via glBufferSubData (Module 6 Slide 43)
    glBufferData(GL_ARRAY_BUFFER, vertexBytes + colorBytes, kGeometryVertices, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, vertexBytes, colorBytes, kGeometryColors);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    g_vboInitialized = (glIsBuffer(g_vboId) == GL_TRUE);
}

void cleanupResources() {
    if (g_vboInitialized && g_vboId != 0) {
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &g_vboId);
        g_vboId = 0;
        g_vboInitialized = false;
    }
}

/**
 * @brief Configures the 3D projection matrix (Module 7 Subtopic 2).
 */
void apply3DProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const double aspect = (g_windowHeight > 0)
                              ? static_cast<double>(g_windowWidth) / static_cast<double>(g_windowHeight)
                              : 1.0;
    if (g_useOrthographic) {
        // Module 7 Slide 25: Orthographic parallel projection
        glOrtho(-4.0 * aspect, 4.0 * aspect, -4.0, 4.0, 0.1, 50.0);
    } else {
        // Module 7 Slide 20: Perspective projection
        gluPerspective(50.0, aspect, 0.1, 50.0);
    }
}

/**
 * @brief Renders the 3D verification scene (Floor grid with stippled lines, VBO Pyramid, VBO Cube).
 */
void render3DVerificationScene() {
    glEnable(GL_DEPTH_TEST);
    apply3DProjection();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Module 7 Slide 35: Camera transformation via gluLookAt
    gluLookAt(0.0, 2.6, 7.5,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);

    glRotatef(g_cameraPitch, 1.0f, 0.0f, 0.0f);
    glRotatef(g_cameraYaw, 0.0f, 1.0f, 0.0f);

    // 1. Render 2.5D stage floor grid using GL_LINES and GL_LINE_STIPPLE (Module 2 Subtopic 1)
    glLineWidth(1.5f);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(2, 0xAAAA);
    glBegin(GL_LINES);
    for (int i = -5; i <= 5; ++i) {
        glColor3ub(92, 181, 100); // Module 2 Slide 35: glColor3ub
        glVertex3f(static_cast<float>(i), -1.5f, -5.0f);
        glVertex3f(static_cast<float>(i), -1.5f,  5.0f);

        glColor3f(0.30f, 0.45f, 0.65f);
        glVertex3f(-5.0f, -1.5f, static_cast<float>(i));
        glVertex3f( 5.0f, -1.5f, static_cast<float>(i));
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    // 2. Render atmospheric floating particles using GL_POINTS (Module 2 Slide 6)
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.95f, 0.6f);
    glVertex3f(-2.5f, 1.8f, -1.0f);
    glVertex3f( 0.0f, 2.2f,  0.5f);
    glVertex3f( 2.5f, 1.6f, -0.5f);
    glEnd();

    // 3. Render VBO Pyramid and VBO Cube using BUFFER_OFFSET, glDrawArrays, and glArrayElement (Modules 4, 5, 6, 7)
    initPhase0VBO();
    glBindBuffer(GL_ARRAY_BUFFER, g_vboId);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    const GLsizeiptr vertexBytes = sizeof(kGeometryVertices);

    // Left Object: 3D Pyramid rendered with glArrayElement inside glBegin/glEnd (Module 6 Slide 43)
    glPushMatrix();
    glTranslatef(-2.0f, 0.0f, 0.0f);
    glRotatef(g_rotationAngle, 0.0f, 1.0f, 0.0f);
    glScalef(0.9f, 0.9f, 0.9f);

    glVertexPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
    glColorPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(vertexBytes));
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 12; ++i) {
        glArrayElement(i);
    }
    glEnd();
    glPopMatrix();

    // Right Object: 3D Color Cube rendered with glDrawArrays and BUFFER_OFFSET (Module 6 Slide 37 & 46)
    glPushMatrix();
    glTranslatef(2.0f, 0.0f, 0.0f);
    glRotatef(-g_rotationAngle * 0.8f, 0.3f, 1.0f, 0.2f);
    glScalef(0.85f, 0.85f, 0.85f);

    glVertexPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(12 * 3 * sizeof(GLfloat)));
    glColorPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(vertexBytes + (12 * 3 * sizeof(GLfloat))));
    glDrawArrays(GL_QUADS, 0, 24);
    glPopMatrix();

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/**
 * @brief Renders the 2D HUD overlay using gluOrtho2D, GL_POLYGON, GL_LINE_LOOP, and glutBitmapString (Modules 1, 2, 3, 7).
 */
void render2DOverlay() {
    if (!g_showDiagnosticsOverlay) {
        return;
    }

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, static_cast<double>(g_windowWidth), 0.0, static_cast<double>(g_windowHeight));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Semi-transparent HUD panel background (Module 2 Slide 33: glColor4f)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float panelLeft = 20.0f;
    const float panelBottom = static_cast<float>(g_windowHeight) - 230.0f;
    const float panelRight = 620.0f;
    const float panelTop = static_cast<float>(g_windowHeight) - 20.0f;

    glColor4f(0.08f, 0.10f, 0.16f, 0.85f);
    glBegin(GL_POLYGON);
    glVertex2f(panelLeft, panelBottom);
    glVertex2f(panelRight, panelBottom);
    glVertex2f(panelRight, panelTop);
    glVertex2f(panelLeft, panelTop);
    glEnd();

    // Dashed border using GL_LINE_LOOP and glLineStipple (Module 2 Slides 11 & 13)
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x00FF);
    glLineWidth(2.0f);
    glColor3f(0.45f, 0.80f, 0.95f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(panelLeft, panelBottom);
    glVertex2f(panelRight, panelBottom);
    glVertex2f(panelRight, panelTop);
    glVertex2f(panelLeft, panelTop);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    glDisable(GL_BLEND);

    // Bitmap text lines (Module 3 Subtopic 1)
    float textY = panelTop - 28.0f;
    drawBitmapText2D(panelLeft + 16.0f, textY, GLUT_BITMAP_HELVETICA_18,
                     "ECHOES OF NOVEMBER - Phase 0 OpenGL Verification", 0.98f, 0.90f, 0.55f);

    textY -= 26.0f;
    std::string glVer = std::string("GL_VERSION:  ") +
                        reinterpret_cast<const char*>(glGetString(GL_VERSION));
    drawBitmapText2D(panelLeft + 16.0f, textY, GLUT_BITMAP_HELVETICA_12, glVer.c_str(), 0.85f, 0.92f, 1.0f);

    textY -= 20.0f;
    std::string glRen = std::string("GL_RENDERER: ") +
                        reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    drawBitmapText2D(panelLeft + 16.0f, textY, GLUT_BITMAP_HELVETICA_12, glRen.c_str(), 0.85f, 0.92f, 1.0f);

    textY -= 20.0f;
    std::string projStr = std::string("Projection [P]: ") +
                          (g_useOrthographic ? "Orthographic (glOrtho)" : "Perspective (gluPerspective)") +
                          " | VBO ID: " + std::to_string(g_vboId);
    drawBitmapText2D(panelLeft + 16.0f, textY, GLUT_BITMAP_HELVETICA_12, projStr.c_str(), 0.65f, 0.95f, 0.70f);

    textY -= 20.0f;
    std::string inputStr = "Last Callback Event: " + g_lastInputEvent;
    drawBitmapText2D(panelLeft + 16.0f, textY, GLUT_BITMAP_HELVETICA_12, inputStr.c_str(), 0.95f, 0.80f, 0.65f);

    textY -= 24.0f;
    drawBitmapText2D(panelLeft + 16.0f, textY, GLUT_BITMAP_HELVETICA_12,
                     "Controls: [WASD] Orbit Camera | [P] Ortho/Persp | [F1] Toggle HUD | [F11] Fullscreen | [ESC] Exit",
                     0.75f, 0.78f, 0.85f);

    // Module 3 Slide 10: Stroke font vector text sample in bottom-left corner
    glPushMatrix();
    glTranslatef(24.0f, 24.0f, 0.0f);
    glScalef(0.14f, 0.14f, 1.0f);
    glLineWidth(1.5f);
    glColor3f(0.95f, 0.75f, 0.35f);
    glutStrokeString(GLUT_STROKE_ROMAN,
                     reinterpret_cast<const unsigned char*>("CS0045 OpenGL Compatibility Profile Active"));
    glPopMatrix();
}

void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render3DVerificationScene();
    render2DOverlay();

    glFlush();
    glutSwapBuffers();

    // Check for any OpenGL state errors
    GLenum err = glGetError();
    while (err != GL_NO_ERROR) {
        ++g_glErrorCount;
        std::fprintf(stderr, "glGetError() reported: 0x%x\n", err);
        err = glGetError();
    }

    ++g_renderedFrames;
    if (g_autoVerifyMode && g_renderedFrames >= 5) {
        std::printf("[Phase 0 Self-Test] Rendered %d frames successfully. VBO=%u, GL Errors=%d\n",
                    g_renderedFrames, g_vboId, g_glErrorCount);
        cleanupResources();
        std::exit(g_glErrorCount == 0 ? 0 : 1);
    }
}

void reshapeCallback(int width, int height) {
    if (height <= 0) {
        height = 1;
    }
    g_windowWidth = width;
    g_windowHeight = height;
    glViewport(0, 0, g_windowWidth, g_windowHeight);
    glutPostRedisplay();
}

void keyboardCallback(unsigned char key, int x, int y) {
    (void)x;
    (void)y;
    g_lastInputEvent = std::string("Key '") + static_cast<char>(key) + "' pressed";
    switch (key) {
        case 27: // ESC
            cleanupResources();
            std::exit(0);
            break;
        case 'w':
        case 'W':
            g_cameraPitch += 4.0f;
            break;
        case 's':
        case 'S':
            g_cameraPitch -= 4.0f;
            break;
        case 'a':
        case 'A':
            g_cameraYaw -= 5.0f;
            break;
        case 'd':
        case 'D':
            g_cameraYaw += 5.0f;
            break;
        case 'p':
        case 'P':
            g_useOrthographic = !g_useOrthographic;
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void specialCallback(int key, int x, int y) {
    (void)x;
    (void)y;
    if (key == GLUT_KEY_F1) {
        g_showDiagnosticsOverlay = !g_showDiagnosticsOverlay;
        g_lastInputEvent = "F1 (Toggle Diagnostics Overlay)";
    } else if (key == GLUT_KEY_F11) {
        g_isFullscreen = !g_isFullscreen;
        if (g_isFullscreen) {
            glutFullScreen();
        } else {
            glutReshapeWindow(1280, 720);
        }
        g_lastInputEvent = "F11 (Toggle Fullscreen)";
    }
    glutPostRedisplay();
}

void mouseCallback(int button, int state, int x, int y) {
    g_lastMouseX = x;
    g_lastMouseY = y;
    if (button == GLUT_LEFT_BUTTON) {
        g_mouseLeftDown = (state == GLUT_DOWN);
        g_lastInputEvent = std::string("Mouse Left ") + (state == GLUT_DOWN ? "DOWN" : "UP") +
                           " at (" + std::to_string(x) + "," + std::to_string(y) + ")";
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        g_lastInputEvent = "Mouse Right DOWN";
    } else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
        g_lastInputEvent = "Mouse Middle DOWN";
    }
    glutPostRedisplay();
}

void motionCallback(int x, int y) {
    if (g_mouseLeftDown) {
        g_cameraYaw += static_cast<float>(x - g_lastMouseX) * 0.4f;
        g_cameraPitch += static_cast<float>(y - g_lastMouseY) * 0.4f;
        g_lastMouseX = x;
        g_lastMouseY = y;
        g_lastInputEvent = "Mouse Drag Orbit (" + std::to_string(x) + "," + std::to_string(y) + ")";
        glutPostRedisplay();
    }
}

void passiveMotionCallback(int x, int y) {
    g_lastMouseX = x;
    g_lastMouseY = y;
}

void entryCallback(int state) {
    if (state == GLUT_LEFT) {
        g_lastInputEvent = "Mouse LEFT window (GLUT_LEFT)";
    } else if (state == GLUT_ENTERED) {
        g_lastInputEvent = "Mouse ENTERED window (GLUT_ENTERED)";
    }
    glutPostRedisplay();
}

void timerCallback(int value) {
    g_rotationAngle += 1.2f;
    if (g_rotationAngle >= 360.0f) {
        g_rotationAngle -= 360.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, value);
}

} // namespace

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--verify-phase0") == 0) {
            g_autoVerifyMode = true;
        }
    }

    // Module 1 & Module 7: Initialize FreeGLUT and request RGBA + Double Buffer + Depth Buffer
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(g_windowWidth, g_windowHeight);
    glutInitWindowPosition(100, 60);
    glutCreateWindow("Echoes of November - CS0045 2.5D Narrative Game (Phase 0)");

    // Module 5: Initialize GLEW before calling OpenGL buffer/debug extensions
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::fprintf(stderr, "GLEW initialization failed: %s\n",
                     reinterpret_cast<const char*>(glewGetErrorString(err)));
        return 1;
    }

    // Module 6 Slide 39: Print OpenGL version, vendor, and renderer on startup
    std::printf("=== OpenGL Context Diagnostics ===\n");
    std::printf("OpenGL Version : %s\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    std::printf("OpenGL Vendor  : %s\n", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    std::printf("OpenGL Renderer: %s\n", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    GLint maxLights = 0;
    glGetIntegerv(GL_MAX_LIGHTS, &maxLights);
    std::printf("GL_MAX_LIGHTS  : %d\n", maxLights);
    std::printf("==================================\n");
    std::fflush(stdout);

    // Enable OpenGL Debug Output if supported by the driver (Module 6 Slide 39)
    if (GLEW_KHR_debug || glDebugMessageCallback != nullptr) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, nullptr);
    }

    glClearColor(0.09f, 0.11f, 0.16f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // Register FreeGLUT callbacks (Module 3 Subtopic 2)
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutSpecialFunc(specialCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback);
    glutPassiveMotionFunc(passiveMotionCallback);
    glutEntryFunc(entryCallback);
    glutTimerFunc(16, timerCallback, 0);

    glutMainLoop();
    return 0;
}
