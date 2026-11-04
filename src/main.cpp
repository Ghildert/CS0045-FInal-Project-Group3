/**
 * CS0045 Computer Graphics and Visual Computing
 * "Echoes of November" - 2.5D Narrative Adventure Game (C++17 & OpenGL Compatibility Profile)
 *
 * PHASE 1 DELIVERABLE: Core Renderer
 * - Window & main loop + resize handling (glViewport)
 * - Input callbacks (InputManager: keyboard, special keys, mouse buttons, motion, entry)
 * - Basic drawing primitives & line stippling (PrimitiveRenderer: Modules 1 & 2)
 * - Vertex Arrays & VBOs + Mapped Buffer 4x4 Transformations (MeshVBO + Math4x4: Modules 4, 5, 6)
 * - Texture loading via stb_image with GL_NEAREST pixel-art filtering & mipmaps (Texture2D: Module 8)
 * - Camera & Projection systems: gluPerspective, glFrustum, glOrtho, gluOrtho2D, gluLookAt (Camera3D: Module 7)
 * - Phong Lighting + Spotlight + Materials + GLUT 3D Objects (LightingSystem: Module 8)
 * - Bitmap & Stroke Text HUD (TextRenderer: Module 3)
 */

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#include "engine/core/Logger.h"
#include "engine/core/Math4x4.h"
#include "engine/input/InputManager.h"
#include "engine/assets/Texture2D.h"
#include "engine/renderer/PrimitiveRenderer.h"
#include "engine/renderer/MeshVBO.h"
#include "engine/renderer/LightingSystem.h"
#include "engine/camera/Camera3D.h"
#include "engine/ui/TextRenderer.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace engine;

// ============================================================================
// Phase 1 Engine State
// ============================================================================
namespace {

camera::Camera3D         g_camera;
renderer::LightingSystem g_lighting;

// Textures (Module 8 Subtopic 2)
assets::Texture2D g_texWoodFloor;
assets::Texture2D g_texClassroomWall;
assets::Texture2D g_texDeskWood;
assets::Texture2D g_texChalkboard;
assets::Texture2D g_texWindowGlass;
assets::Texture2D g_texCheckerDebug;

// Reusable VBO Meshes (Modules 4, 5, 6)
renderer::MeshVBO g_meshFloor;
renderer::MeshVBO g_meshBackWall;
renderer::MeshVBO g_meshSideWall;
renderer::MeshVBO g_meshChalkboard;
renderer::MeshVBO g_meshWindowPane;
renderer::MeshVBO g_meshDeskTop;
renderer::MeshVBO g_meshDeskLeg;
renderer::MeshVBO g_meshDynamicPyramid; // Module 6 mapped-VBO animated pyramid

// Runtime flags & timing
bool  g_autoVerifyExit    = false;
int   g_framesRendered    = 0;
float g_timeSeconds       = 0.0f;
bool  g_showDebugOverlay  = true;
bool  g_useCheckerTexture = false;
bool  g_wireframeTeapot   = false;
bool  g_fullscreen        = false;

// ============================================================================
// Scene Initialization
// ============================================================================
void initPhase1Resources() {
    core::Logger::initOpenGLDebugOutput();
    core::Logger::printContextInfo();

    // Depth testing & alpha blending (Modules 7 & 8)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);

    // Warm twilight background clear color (Module 1)
    glClearColor(0.09f, 0.08f, 0.14f, 1.0f);

    // Ensure pixel-art PNG textures exist on disk and load them via stb_image (Module 8)
    assets::Texture2D::ensureDefaultTexturesOnDisk();
    g_texWoodFloor.loadFromFile("assets/textures/wood_floor.png", true, true);
    g_texClassroomWall.loadFromFile("assets/textures/classroom_wall.png", true, true);
    g_texDeskWood.loadFromFile("assets/textures/desk_wood.png", true, true);
    g_texChalkboard.loadFromFile("assets/textures/chalkboard.png", true, false);
    g_texWindowGlass.loadFromFile("assets/textures/window_glass.png", true, false);
    g_texCheckerDebug.loadFromFile("assets/textures/checker_debug.png", true, true);

    // Build reusable VBO geometry (Modules 4, 5, 6)
    g_meshFloor.buildQuadXZ(9.0f, 6.5f, 6.0f, 4.0f, GL_STATIC_DRAW);
    g_meshBackWall.buildTexturedBox(9.0f, 3.8f, 0.22f, 4.0f, 1.0f, GL_STATIC_DRAW);
    g_meshSideWall.buildTexturedBox(0.22f, 3.8f, 6.5f, 3.0f, 1.0f, GL_STATIC_DRAW);
    g_meshChalkboard.buildTexturedBox(4.2f, 1.45f, 0.08f, 1.0f, 1.0f, GL_STATIC_DRAW);
    g_meshWindowPane.buildTexturedBox(0.08f, 1.7f, 2.2f, 1.0f, 1.0f, GL_STATIC_DRAW);
    g_meshDeskTop.buildTexturedBox(1.35f, 0.10f, 0.75f, 1.0f, 1.0f, GL_STATIC_DRAW);
    g_meshDeskLeg.buildTexturedBox(0.09f, 0.72f, 0.09f, 1.0f, 1.0f, GL_STATIC_DRAW);

    // Build dynamic VBO Pyramid for Module 6 glMapBuffer 4x4 matrix transformation test
    g_meshDynamicPyramid.buildPyramid(0.45f, 0.55f, GL_DYNAMIC_DRAW);

    // Demonstrate Module 6 explicit 4x4 matrix translation on a mapped VBO vertex at startup
    core::Math4x4::translateVertexInMappedVBO(
        g_meshDynamicPyramid.getVBOId(),
        0.0f, 0.02f, 0.0f,
        0);

    // Initialize Classroom Phong + Spotlight Lighting (Module 8)
    g_lighting.initClassroomLighting();

    core::Logger::checkGLError("initPhase1Resources");
}

// ============================================================================
// Helper to draw a complete 3D Desk (using Matrix Stack glPushMatrix/glPopMatrix)
// ============================================================================
void drawDesk3D(float x, float z, float yawDegrees, const assets::Texture2D& woodTex) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(yawDegrees, 0.0f, 1.0f, 0.0f);

    woodTex.bind();

    // Tabletop (drawn with VBO + EBO glDrawElements)
    glPushMatrix();
    glTranslatef(0.0f, 0.74f, 0.0f);
    g_meshDeskTop.drawVBOElements();
    glPopMatrix();

    // 4 Desk legs (drawn with VBO glDrawArrays)
    const float lx = 0.56f;
    const float lz = 0.28f;
    const float legCoords[4][2] = {
        { -lx, -lz }, {  lx, -lz },
        { -lx,  lz }, {  lx,  lz }
    };
    for (int i = 0; i < 4; ++i) {
        glPushMatrix();
        glTranslatef(legCoords[i][0], 0.36f, legCoords[i][1]);
        g_meshDeskLeg.drawVBOArrays();
        glPopMatrix();
    }

    assets::Texture2D::unbind();
    glPopMatrix();
}

// ============================================================================
// Render 3D Classroom Test Scene
// ============================================================================
void renderScene3D() {
    g_camera.applyProjection();
    g_camera.applyView();

    g_lighting.applyLights(g_timeSeconds);
    renderer::LightingSystem::applyMaterial(renderer::LightingSystem::matteWoodMaterial());

    const assets::Texture2D& floorTex = g_useCheckerTexture ? g_texCheckerDebug : g_texWoodFloor;
    const assets::Texture2D& wallTex  = g_useCheckerTexture ? g_texCheckerDebug : g_texClassroomWall;
    const assets::Texture2D& deskTex  = g_useCheckerTexture ? g_texCheckerDebug : g_texDeskWood;
    const assets::Texture2D& boardTex = g_useCheckerTexture ? g_texCheckerDebug : g_texChalkboard;

    // 1. Textured Wood Floor (VBO + EBO glDrawElements)
    floorTex.bind();
    glPushMatrix();
    g_meshFloor.drawVBOElements();
    glPopMatrix();

    // 2. Back Classroom Wall (VBO glDrawArrays)
    wallTex.bind();
    glPushMatrix();
    glTranslatef(0.0f, 1.9f, -3.25f);
    g_meshBackWall.drawVBOArrays();
    glPopMatrix();

    // 3. Left Classroom Wall (Client-side Vertex Array demonstration - Module 4)
    glPushMatrix();
    glTranslatef(-4.5f, 1.9f, 0.0f);
    g_meshSideWall.drawClientSideElements();
    glPopMatrix();

    // 4. Front Chalkboard (Module 6 glArrayElement demonstration inside VBO)
    boardTex.bind();
    glPushMatrix();
    glTranslatef(0.0f, 1.95f, -3.08f);
    g_meshChalkboard.drawVBOArrayElement();
    glPopMatrix();
    assets::Texture2D::unbind();

    // 5. Teacher's Desk + Student Desks
    drawDesk3D(1.5f, -1.6f, -8.0f, deskTex);
    drawDesk3D(-1.6f,  0.2f,  0.0f, deskTex);
    drawDesk3D( 0.4f,  0.2f,  0.0f, deskTex);
    drawDesk3D(-1.6f,  1.7f,  0.0f, deskTex);
    drawDesk3D( 0.4f,  1.7f,  0.0f, deskTex);

    // 6. Module 8 GLUT 3D Objects on Teacher's Desk (Teapot & Sphere with polished specular material)
    renderer::LightingSystem::applyMaterial(renderer::LightingSystem::polishedCeramicMaterial());
    glPushMatrix();
    glTranslatef(1.35f, 0.92f, -1.55f);
    glRotatef(g_timeSeconds * 28.0f, 0.0f, 1.0f, 0.0f);
    glColor3f(0.95f, 0.82f, 0.48f);
    if (g_wireframeTeapot) {
        glutWireTeapot(0.16);
    } else {
        glutSolidTeapot(0.16);
    }
    glPopMatrix();

    // Globe sphere prop next to teapot
    glPushMatrix();
    glTranslatef(1.82f, 0.93f, -1.62f);
    glColor3f(0.45f, 0.76f, 0.88f);
    if (g_wireframeTeapot) {
        glutWireSphere(0.13, 16, 16);
    } else {
        glutSolidSphere(0.13, 16, 16);
    }
    glPopMatrix();

    // 7. Module 6 3D Pyramid with Mapped-Buffer Transformation on Front Student Desk
    renderer::LightingSystem::applyMaterial(renderer::LightingSystem::matteWoodMaterial());
    glPushMatrix();
    glTranslatef(-1.6f, 0.80f, 0.2f);
    glRotatef(g_timeSeconds * 40.0f, 0.0f, 1.0f, 0.0f);
    g_meshDynamicPyramid.drawVBOArrays();
    glPopMatrix();

    // 8. Ceiling Spotlight Fixture Prop (Glowing Emission Material)
    renderer::LightingSystem::applyMaterial(renderer::LightingSystem::glowingLampMaterial());
    glPushMatrix();
    glTranslatef(1.5f, 3.62f, -0.8f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 0.95f, 0.75f);
    glutSolidCone(0.22, 0.18, 16, 4);
    glPopMatrix();

    // 9. Atmospheric Dust Motes (Module 2 GL_POINTS + glPointSize)
    renderer::PrimitiveRenderer::drawDustParticles(g_timeSeconds, 32);

    // 10. Semi-Transparent Sunset Window Panes on Left Wall (Sorted back-to-front)
    g_texWindowGlass.bind();
    glPushMatrix();
    glTranslatef(-4.36f, 2.05f, -1.2f);
    g_meshWindowPane.drawVBOElements();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-4.36f, 2.05f, 1.3f);
    g_meshWindowPane.drawVBOElements();
    glPopMatrix();
    assets::Texture2D::unbind();

    // 11. Debug Grid, Axes, Stippled Wireframe Bounding Boxes & Module 2 Primitive Showcase (F3)
    if (g_showDebugOverlay) {
        renderer::PrimitiveRenderer::drawReferenceGrid(4.5f, 0.75f);
        renderer::PrimitiveRenderer::drawCoordinateAxes(1.5f);
        renderer::PrimitiveRenderer::drawStippledWireBox(
            1.5f, 0.42f, -1.6f, 1.42f, 0.85f, 0.82f,
            1.0f, 0.82f, 0.32f, 2, renderer::PrimitiveRenderer::kStippleDashed, 1.8f);
        renderer::PrimitiveRenderer::drawModule2PrimitiveShowcase(0.0f, 2.95f, -2.95f);
    }
}

// ============================================================================
// Render 2D Orthographic HUD Overlay (Module 3 & Module 7 gluOrtho2D)
// ============================================================================
void renderHUD2D() {
    const int w = g_camera.getViewportWidth();
    const int h = g_camera.getViewportHeight();

    camera::Camera3D::begin2DOverlay(w, h);

    // Top-left status panel
    ui::TextRenderer::drawPanel2D(16.0f, static_cast<float>(h - 175), 510.0f, 158.0f);

    // Scalable Vector Stroke Title (Module 3 glutStrokeString)
    ui::TextRenderer::drawStrokeText2D(
        30.0f, static_cast<float>(h - 46),
        "ECHOES OF NOVEMBER - PHASE 1 CORE RENDERER",
        0.135f, 2.0f, GLUT_STROKE_ROMAN,
        1.0f, 0.86f, 0.48f);

    // Bitmap HUD status lines (Module 3 glutBitmapString)
    std::ostringstream camLine;
    camLine << std::fixed << std::setprecision(1)
            << "Camera Eye: (" << g_camera.getEyeX() << ", "
            << g_camera.getEyeY() << ", " << g_camera.getEyeZ()
            << ") | Proj [P]: " << g_camera.getProjectionModeName();
    ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 74),
        camLine.str(), GLUT_BITMAP_HELVETICA_12, 0.90f, 0.94f, 1.0f);

    std::ostringstream lightLine;
    lightLine << "Lighting [1]: " << (g_lighting.isLightingEnabled() ? "ON" : "OFF")
              << " | Sun [2]: " << (g_lighting.isSunLightEnabled() ? "ON" : "OFF")
              << " | Spot [3]: " << (g_lighting.isSpotLightEnabled() ? "ON" : "OFF")
              << " | Specular [4]: " << (g_lighting.isSpecularEnabled() ? "ON" : "OFF");
    ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 96),
        lightLine.str(), GLUT_BITMAP_HELVETICA_12, 0.98f, 0.85f, 0.62f);

    ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 118),
        "Controls: WASD Pan | Q/E Zoom | Arrows or Right-Drag Orbit | R Reset Camera",
        GLUT_BITMAP_HELVETICA_12, 0.78f, 0.86f, 0.78f);

    ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 140),
        "Toggles : [F3] Debug Grid/Primitives | [T] UV Checker | [M] Wire Teapot | [F11] Fullscreen",
        GLUT_BITMAP_HELVETICA_12, 0.78f, 0.86f, 0.78f);

    ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 162),
        std::string("Mouse Inside Window (glutEntryFunc): ") +
            (input::InputManager::isMouseInsideWindow() ? "YES" : "NO") +
            " | GL Errors: " + std::to_string(core::Logger::getErrorCount()),
        GLUT_BITMAP_9_BY_15, 0.68f, 0.92f, 0.85f);

    camera::Camera3D::end2DOverlay();
}

// ============================================================================
// GLUT Callbacks (Modules 1, 3, 7)
// ============================================================================
void displayCallback() {
    // Module 7: Clear both Color and Depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderScene3D();
    renderHUD2D();

    glutSwapBuffers();
    glFlush();

    core::Logger::checkGLError("displayCallback");
    ++g_framesRendered;

    if (g_autoVerifyExit && g_framesRendered >= 6) {
        bool texturesOk = g_texWoodFloor.isValid() &&
                          g_texClassroomWall.isValid() &&
                          g_texDeskWood.isValid() &&
                          g_texChalkboard.isValid() &&
                          g_texWindowGlass.isValid() &&
                          g_texCheckerDebug.isValid();
        bool vbosOk = g_meshFloor.isValid() &&
                      g_meshBackWall.isValid() &&
                      g_meshDeskTop.isValid() &&
                      g_meshDynamicPyramid.isValid();

        std::cout << "[Phase 1 Self-Test] Rendered " << g_framesRendered
                  << " frames. Textures=" << (texturesOk ? "OK(6)" : "FAIL")
                  << ", VBOs=" << (vbosOk ? "OK" : "FAIL")
                  << ", GL Errors=" << core::Logger::getErrorCount() << std::endl;
        glutLeaveMainLoop();
    }
}

void reshapeCallback(int width, int height) {
    g_camera.setViewport(width, height);
}

void keyboardDownCallback(unsigned char key, int x, int y) {
    input::InputManager::onKeyboardDown(key, x, y);

    if (key == 27) { // ESC
        glutLeaveMainLoop();
    } else if (key == 'p' || key == 'P') {
        g_camera.cycleProjectionMode();
    } else if (key == 'r' || key == 'R') {
        g_camera.resetToDefault25D();
    } else if (key == 't' || key == 'T') {
        g_useCheckerTexture = !g_useCheckerTexture;
    } else if (key == 'm' || key == 'M') {
        g_wireframeTeapot = !g_wireframeTeapot;
    } else if (key == '1') {
        g_lighting.toggleLightingEnabled();
    } else if (key == '2') {
        g_lighting.toggleSunLight();
    } else if (key == '3') {
        g_lighting.toggleSpotLight();
    } else if (key == '4') {
        g_lighting.toggleSpecular();
    }
}

void keyboardUpCallback(unsigned char key, int x, int y) {
    input::InputManager::onKeyboardUp(key, x, y);
}

void specialDownCallback(int key, int x, int y) {
    input::InputManager::onSpecialDown(key, x, y);

    if (key == GLUT_KEY_F3) {
        g_showDebugOverlay = !g_showDebugOverlay;
    } else if (key == GLUT_KEY_F11) {
        g_fullscreen = !g_fullscreen;
        if (g_fullscreen) {
            glutFullScreen();
        } else {
            glutReshapeWindow(1280, 720);
        }
    }
}

void specialUpCallback(int key, int x, int y) {
    input::InputManager::onSpecialUp(key, x, y);
}

void timerCallback(int value) {
    (void)value;
    constexpr float dt = 0.016f;
    g_timeSeconds += dt;

    g_camera.updateFreeOrbit(dt);

    // Module 6 Subtopic 1 & 2: Subtle vertical bobbing on pyramid apex vertex (index 0) via glMapBuffer
    float bobVel = std::cos(g_timeSeconds * 3.0f) * 0.0015f;
    core::Math4x4::translateVertexInMappedVBO(
        g_meshDynamicPyramid.getVBOId(),
        0.0f, bobVel, 0.0f,
        0);

    input::InputManager::endFrame();
    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, 0);
}

} // anonymous namespace

// ============================================================================
// Main Entry Point
// ============================================================================
int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--verify-phase0") == 0 ||
            std::strcmp(argv[i], "--verify-phase1") == 0) {
            g_autoVerifyExit = true;
        }
    }

    // Module 1 & Module 7: Initialize FreeGLUT and request RGBA + Double Buffer + Depth Buffer
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 60);
    glutCreateWindow("Echoes of November - 2.5D Narrative Adventure [Phase 1: Core Renderer]");

    // Module 5: Initialize GLEW
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "[FATAL] glewInit() failed: "
                  << reinterpret_cast<const char*>(glewGetErrorString(glewStatus)) << std::endl;
        return 1;
    }

    input::InputManager::init();
    initPhase1Resources();

    // Register FreeGLUT Callbacks (Modules 1 & 3)
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardDownCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialDownCallback);
    glutSpecialUpFunc(specialUpCallback);
    glutMouseFunc(input::InputManager::onMouseButton);
    glutMotionFunc(input::InputManager::onMouseMotion);
    glutPassiveMotionFunc(input::InputManager::onMousePassiveMotion);
    glutEntryFunc(input::InputManager::onMouseEntry);
    glutTimerFunc(16, timerCallback, 0);

    glutMainLoop();

    // Clean up GPU buffers and textures (Modules 5 & 8)
    g_meshFloor.destroy();
    g_meshBackWall.destroy();
    g_meshSideWall.destroy();
    g_meshChalkboard.destroy();
    g_meshWindowPane.destroy();
    g_meshDeskTop.destroy();
    g_meshDeskLeg.destroy();
    g_meshDynamicPyramid.destroy();

    g_texWoodFloor.destroy();
    g_texClassroomWall.destroy();
    g_texDeskWood.destroy();
    g_texChalkboard.destroy();
    g_texWindowGlass.destroy();
    g_texCheckerDebug.destroy();

    return (core::Logger::getErrorCount() == 0) ? 0 : 2;
}
