/**
 * CS0045 Computer Graphics and Visual Computing
 * "Echoes of November" - 2.5D Narrative Adventure Game (C++17 & OpenGL Compatibility Profile)
 *
 * PHASE 2 DELIVERABLE: 2.5D Character Prototype
 * - Textured 2D pixel-art character quad positioned in 3D space (SpriteQuad3D)
 * - Sprite-sheet animation with Idle and 4-frame Walk cycles (SpriteAnimator)
 * - Player movement (WASD / Arrow keys) & directional orientation (PlayerController)
 * - Smooth 2.5D narrative follow camera + toggleable Free-Orbit camera [C] (Camera3D)
 * - 3D AABB collision detection & sliding resolution with walls and desks (CollisionSystem)
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
#include "engine/animation/SpriteAnimator.h"
#include "engine/renderer/PrimitiveRenderer.h"
#include "engine/renderer/MeshVBO.h"
#include "engine/renderer/SpriteQuad3D.h"
#include "engine/renderer/LightingSystem.h"
#include "engine/collision/CollisionSystem.h"
#include "engine/camera/Camera3D.h"
#include "engine/ui/TextRenderer.h"
#include "game/player/PlayerController.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace engine;

// ============================================================================
// Phase 2 Engine & Gameplay State
// ============================================================================
namespace {

camera::Camera3D           g_camera;
renderer::LightingSystem   g_lighting;
collision::CollisionWorld  g_collisionWorld;
game::player::PlayerController g_player;

// Environment Textures (Module 8 Subtopic 2)
assets::Texture2D g_texWoodFloor;
assets::Texture2D g_texClassroomWall;
assets::Texture2D g_texDeskWood;
assets::Texture2D g_texChalkboard;
assets::Texture2D g_texWindowGlass;
assets::Texture2D g_texCheckerDebug;

//Reusable VBO Environment Meshes (Modules 4, 5, 6)
renderer::MeshVBO g_meshFloor;
renderer::MeshVBO g_meshBackWall;
renderer::MeshVBO g_meshSideWall;
renderer::MeshVBO g_meshChalkboard;
renderer::MeshVBO g_meshWindowPane;
renderer::MeshVBO g_meshDeskTop;
renderer::MeshVBO g_meshDeskLeg;
renderer::MeshVBO g_meshDynamicPyramid;

// Runtime flags & verification state
bool  g_autoVerifyExit    = false;
bool  g_verifyPhase2Mode  = false;
bool  g_verifiedCollision = false;
int   g_framesRendered    = 0;
float g_timeSeconds       = 0.0f;
bool  g_showDebugOverlay  = true;
bool  g_useCheckerTexture = false;
bool  g_wireframeTeapot   = false;
bool  g_fullscreen        = false;

// ============================================================================
// Setup Room Colliders (Walls + Desks)
// ============================================================================
void buildClassroomColliders() {
    g_collisionWorld.clear();

    // Room boundary walls (floor is 9.0 wide x 6.5 deep: X in [-4.4, 4.4], Z in [-3.1, 3.1])
    g_collisionWorld.addBoxCollider( 0.0f, 1.9f, -3.20f, 9.2f, 3.8f, 0.35f, "BackWall");
    g_collisionWorld.addBoxCollider( 0.0f, 1.9f,  3.25f, 9.2f, 3.8f, 0.25f, "FrontBoundary");
    g_collisionWorld.addBoxCollider(-4.45f, 1.9f, 0.0f,  0.35f, 3.8f, 6.6f, "LeftWindowWall");
    g_collisionWorld.addBoxCollider( 4.45f, 1.9f, 0.0f,  0.35f, 3.8f, 6.6f, "RightWall");

    // Teacher's Desk & 4 Student Desks
    g_collisionWorld.addBoxCollider( 1.5f,  0.42f, -1.6f, 1.40f, 0.85f, 0.80f, "TeacherDesk");
    g_collisionWorld.addBoxCollider(-1.6f,  0.42f,  0.2f, 1.35f, 0.85f, 0.75f, "StudentDesk_FrontLeft");
    g_collisionWorld.addBoxCollider( 0.4f,  0.42f,  0.2f, 1.35f, 0.85f, 0.75f, "StudentDesk_FrontRight");
    g_collisionWorld.addBoxCollider(-1.6f,  0.42f,  1.7f, 1.35f, 0.85f, 0.75f, "StudentDesk_BackLeft");
    g_collisionWorld.addBoxCollider( 0.4f,  0.42f,  1.7f, 1.35f, 0.85f, 0.75f, "StudentDesk_BackRight");
}

// ============================================================================
// Scene Initialization
// ============================================================================
void initPhase2Resources() {
    core::Logger::initOpenGLDebugOutput();
    core::Logger::printContextInfo();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);

    glClearColor(0.09f, 0.08f, 0.14f, 1.0f);

    // Ensure pixel-art PNG textures & character sprite sheets exist on disk
    assets::Texture2D::ensureDefaultTexturesOnDisk();
    g_texWoodFloor.loadFromFile("assets/textures/wood_floor.png", true, true);
    g_texClassroomWall.loadFromFile("assets/textures/classroom_wall.png", true, true);
    g_texDeskWood.loadFromFile("assets/textures/desk_wood.png", true, true);
    g_texChalkboard.loadFromFile("assets/textures/chalkboard.png", true, false);
    g_texWindowGlass.loadFromFile("assets/textures/window_glass.png", true, false);
    g_texCheckerDebug.loadFromFile("assets/textures/checker_debug.png", true, true);

    // Build reusable VBO geometry
    g_meshFloor.buildQuadXZ(9.0f, 6.5f, 6.0f, 4.0f, GL_STATIC_DRAW);
    g_meshBackWall.buildTexturedBox(9.0f, 3.8f, 0.22f, 4.0f, 1.0f, GL_STATIC_DRAW);
    g_meshSideWall.buildTexturedBox(0.22f, 3.8f, 6.5f, 3.0f, 1.0f, GL_STATIC_DRAW);
    g_meshChalkboard.buildTexturedBox(4.2f, 1.45f, 0.08f, 1.0f, 1.0f, GL_STATIC_DRAW);
    g_meshWindowPane.buildTexturedBox(0.08f, 1.7f, 2.2f, 1.0f, 1.0f, GL_STATIC_DRAW);
    g_meshDeskTop.buildTexturedBox(1.35f, 0.10f, 0.75f, 1.0f, 1.0f, GL_STATIC_DRAW);
    g_meshDeskLeg.buildTexturedBox(0.09f, 0.72f, 0.09f, 1.0f, 1.0f, GL_STATIC_DRAW);
    g_meshDynamicPyramid.buildPyramid(0.45f, 0.55f, GL_DYNAMIC_DRAW);

    core::Math4x4::translateVertexInMappedVBO(
        g_meshDynamicPyramid.getVBOId(),
        0.0f, 0.02f, 0.0f,
        0);

    // Initialize 2.5D Player Character Prototype & Classroom Colliders
    buildClassroomColliders();
    g_player.init("assets/sprites/player_elijah_sheet.png");
    g_player.setPosition(-0.6f, 0.0f, 0.95f);

    g_lighting.initClassroomLighting();
    core::Logger::checkGLError("initPhase2Resources");
}

// ============================================================================
// Helper to draw a complete 3D Desk
// ============================================================================
void drawDesk3D(float x, float z, float yawDegrees, const assets::Texture2D& woodTex) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(yawDegrees, 0.0f, 1.0f, 0.0f);

    woodTex.bind();

    glPushMatrix();
    glTranslatef(0.0f, 0.74f, 0.0f);
    g_meshDeskTop.drawVBOElements();
    glPopMatrix();

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
// Render 3D Classroom & 2.5D Character
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

    // 1. Opaque Environment Geometry
    floorTex.bind();
    glPushMatrix();
    g_meshFloor.drawVBOElements();
    glPopMatrix();

    wallTex.bind();
    glPushMatrix();
    glTranslatef(0.0f, 1.9f, -3.25f);
    g_meshBackWall.drawVBOArrays();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-4.5f, 1.9f, 0.0f);
    g_meshSideWall.drawClientSideElements();
    glPopMatrix();

    boardTex.bind();
    glPushMatrix();
    glTranslatef(0.0f, 1.95f, -3.08f);
    g_meshChalkboard.drawVBOArrayElement();
    glPopMatrix();
    assets::Texture2D::unbind();

    // Desks
    drawDesk3D( 1.5f, -1.6f, -8.0f, deskTex);
    drawDesk3D(-1.6f,  0.2f,  0.0f, deskTex);
    drawDesk3D( 0.4f,  0.2f,  0.0f, deskTex);
    drawDesk3D(-1.6f,  1.7f,  0.0f, deskTex);
    drawDesk3D( 0.4f,  1.7f,  0.0f, deskTex);

    // GLUT 3D Props on Teacher's Desk
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

    glPushMatrix();
    glTranslatef(1.82f, 0.93f, -1.62f);
    glColor3f(0.45f, 0.76f, 0.88f);
    if (g_wireframeTeapot) {
        glutWireSphere(0.13, 16, 16);
    } else {
        glutSolidSphere(0.13, 16, 16);
    }
    glPopMatrix();

    // Module 6 Mapped-VBO Pyramid on Front Student Desk
    renderer::LightingSystem::applyMaterial(renderer::LightingSystem::matteWoodMaterial());
    glPushMatrix();
    glTranslatef(-1.6f, 0.80f, 0.2f);
    glRotatef(g_timeSeconds * 40.0f, 0.0f, 1.0f, 0.0f);
    g_meshDynamicPyramid.drawVBOArrays();
    glPopMatrix();

    // Ceiling Lamp
    renderer::LightingSystem::applyMaterial(renderer::LightingSystem::glowingLampMaterial());
    glPushMatrix();
    glTranslatef(1.5f, 3.62f, -0.8f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 0.95f, 0.75f);
    glutSolidCone(0.22, 0.18, 16, 4);
    glPopMatrix();

    // 2. Render 2.5D Player Character Sprite Quad in 3D Space
    renderer::LightingSystem::applyMaterial(renderer::LightingSystem::matteWoodMaterial());
    g_player.render(g_camera.getYawDegrees(), g_showDebugOverlay);

    // 3. Atmospheric Dust Particles & Semi-Transparent Sunset Windows
    renderer::PrimitiveRenderer::drawDustParticles(g_timeSeconds, 32);

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

    // 4. Debug Colliders, Reference Grid & Module 2 Primitive Showcase (F3)
    if (g_showDebugOverlay) {
        renderer::PrimitiveRenderer::drawReferenceGrid(4.5f, 0.75f);
        renderer::PrimitiveRenderer::drawCoordinateAxes(1.5f);
        g_collisionWorld.drawDebugColliders(g_player.getLastHitColliderTag());
        renderer::PrimitiveRenderer::drawModule2PrimitiveShowcase(0.0f, 2.95f, -2.95f);
    }
}

// ============================================================================
// Render 2D Orthographic HUD Overlay
// ============================================================================
void renderHUD2D() {
    const int w = g_camera.getViewportWidth();
    const int h = g_camera.getViewportHeight();

    camera::Camera3D::begin2DOverlay(w, h);

    ui::TextRenderer::drawPanel2D(16.0f, static_cast<float>(h - 195), 555.0f, 178.0f);

    ui::TextRenderer::drawStrokeText2D(
        30.0f, static_cast<float>(h - 46),
        "ECHOES OF NOVEMBER - PHASE 2 CHARACTER PROTOTYPE",
        0.125f, 2.0f, GLUT_STROKE_ROMAN,
        1.0f, 0.86f, 0.48f);

    std::ostringstream playerLine;
    playerLine << std::fixed << std::setprecision(2)
               << "Player Pos: (" << g_player.getX() << ", " << g_player.getZ() << ")"
               << " | State: " << (g_player.isMoving() ? "WALK" : "IDLE")
               << " (Frame " << g_player.getCurrentFrame() << ")"
               << " | Dir: " << (g_player.isFacingLeft() ? "LEFT" : "RIGHT");
    ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 72),
        playerLine.str(), GLUT_BITMAP_HELVETICA_12, 0.92f, 0.98f, 0.88f);

    std::ostringstream colLine;
    colLine << "Collision Contact: "
            << (g_player.getLastHitColliderTag().empty() ? "None" : g_player.getLastHitColliderTag())
            << " | Cam Mode [C]: " << (g_camera.isFreeCameraEnabled() ? "Free Orbit" : "2.5D Follow");
    ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 94),
        colLine.str(), GLUT_BITMAP_HELVETICA_12, 1.0f, 0.84f, 0.60f);

    std::ostringstream projLine;
    projLine << "Projection [P]: " << g_camera.getProjectionModeName()
             << " | Lights [1-4]: " << (g_lighting.isLightingEnabled() ? "ON" : "OFF");
    ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 116),
        projLine.str(), GLUT_BITMAP_HELVETICA_12, 0.86f, 0.92f, 1.0f);

    ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 138),
        "Move: WASD / Arrows | Zoom: Q/E | Orbit: Right-Drag | [C] Cam Mode | [R] Reset",
        GLUT_BITMAP_HELVETICA_12, 0.78f, 0.86f, 0.78f);

    ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 160),
        "Debug: [F3] AABB Colliders & Grid | [T] UV Checker | [M] Wire Teapot | [F11] Fullscreen",
        GLUT_BITMAP_HELVETICA_12, 0.78f, 0.86f, 0.78f);

    ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 182),
        std::string("Cursor Inside Window: ") +
            (input::InputManager::isMouseInsideWindow() ? "YES" : "NO") +
            " | GL Errors: " + std::to_string(core::Logger::getErrorCount()),
        GLUT_BITMAP_9_BY_15, 0.68f, 0.92f, 0.85f);

    camera::Camera3D::end2DOverlay();
}

// ============================================================================
// GLUT Callbacks
// ============================================================================
void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderScene3D();
    renderHUD2D();

    glutSwapBuffers();
    glFlush();

    core::Logger::checkGLError("displayCallback");
    ++g_framesRendered;

    if (g_autoVerifyExit && g_framesRendered >= 10) {
        bool texturesOk = g_texWoodFloor.isValid() &&
                          g_texClassroomWall.isValid() &&
                          g_texDeskWood.isValid() &&
                          g_texChalkboard.isValid() &&
                          g_texWindowGlass.isValid() &&
                          g_texCheckerDebug.isValid();
        bool playerOk = g_player.isValid();

        std::cout << "[Phase 2 Self-Test] Rendered " << g_framesRendered
                  << " frames. PlayerSprite=" << (playerOk ? "OK" : "FAIL")
                  << ", CollisionResolved=" << (g_verifiedCollision ? "OK(" + g_player.getLastHitColliderTag() + ")" : "NONE")
                  << ", PlayerPos=(" << g_player.getX() << "," << g_player.getZ() << ")"
                  << ", Textures=" << (texturesOk ? "OK" : "FAIL")
                  << ", GL Errors=" << core::Logger::getErrorCount() << std::endl;
        glutLeaveMainLoop();
    }
}

void reshapeCallback(int width, int height) {
    g_camera.setViewport(width, height);
}

void keyboardDownCallback(unsigned char key, int x, int y) {
    input::InputManager::onKeyboardDown(key, x, y);

    if (key == 27) {
        glutLeaveMainLoop();
    } else if (key == 'c' || key == 'C') {
        g_camera.setFreeCameraEnabled(!g_camera.isFreeCameraEnabled());
    } else if (key == 'p' || key == 'P') {
        g_camera.cycleProjectionMode();
    } else if (key == 'r' || key == 'R') {
        g_camera.resetToDefault25D();
        g_player.setPosition(-0.6f, 0.0f, 0.95f);
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

    if (g_verifyPhase2Mode) {
        // Automated Phase 2 verification: walk diagonally left/forward into StudentDesk_FrontLeft
        g_player.simulateMoveStep(-0.85f, -0.75f, 0.08f, g_collisionWorld);
        if (!g_player.getLastHitColliderTag().empty()) {
            g_verifiedCollision = true;
        }
    } else if (!g_camera.isFreeCameraEnabled()) {
        g_player.update(dt, g_collisionWorld, true);
    }

    g_camera.updateFreeOrbit(dt);
    if (!g_camera.isFreeCameraEnabled()) {
        // Smoothly follow the player in 2.5D narrative space
        g_camera.setNarrativeTarget(
            g_player.getX() * 0.55f,
            1.18f,
            g_player.getZ() * 0.45f,
            dt);
    }

    // Module 6 Subtopic 1 & 2: Subtle vertical bobbing on pyramid apex vertex via glMapBuffer
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
            std::strcmp(argv[i], "--verify-phase1") == 0 ||
            std::strcmp(argv[i], "--verify-phase2") == 0) {
            g_autoVerifyExit   = true;
            g_verifyPhase2Mode = true;
        }
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 60);
    glutCreateWindow("Echoes of November - 2.5D Narrative Adventure [Phase 2: Character Prototype]");

    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "[FATAL] glewInit() failed: "
                  << reinterpret_cast<const char*>(glewGetErrorString(glewStatus)) << std::endl;
        return 1;
    }

    input::InputManager::init();
    initPhase2Resources();

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

    // Clean up GPU buffers and textures
    g_player.destroy();
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
