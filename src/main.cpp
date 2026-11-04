/**
 * CS0045 Computer Graphics and Visual Computing
 * "Echoes of November" - 2.5D Narrative Adventure Game (C++17 & OpenGL Compatibility Profile)
 *
 * PHASE 6 DELIVERABLE: Interactive Rhythm Minigame ("Acoustic Rhythm Session - Echoes in the Rain")
 * - Connected to the story via Sam's Acoustic Guitar Case on the Rainy Street bench (or [G] shortcut)
 * - Uses FreeGLUT keyboard & timer callbacks, Module 2 primitives & line stippling, per-vertex colors,
 *   and Module 6/7 matrix transformations (glPushMatrix, glTranslatef, glRotatef, glScalef)
 * - Returns cleanly to the 2.5D story and triggers Maya's post-song celebration dialogue
 */

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#include "engine/core/Logger.h"
#include "engine/input/InputManager.h"
#include "engine/camera/Camera3D.h"
#include "game/scenes/ClassroomScene.h"

#include <cstring>
#include <iostream>

using namespace engine;

namespace {

camera::Camera3D             g_camera;
game::scenes::ClassroomScene g_worldScene;

bool g_autoVerifyExit   = false;
bool g_verifyPhase6Mode = false;
int  g_framesRendered   = 0;
bool g_fullscreen       = false;

void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    g_worldScene.render3D(g_camera);
    g_worldScene.render2D(g_camera);

    glutSwapBuffers();
    glFlush();

    core::Logger::checkGLError("displayCallback");
    ++g_framesRendered;

    if (g_autoVerifyExit && g_framesRendered >= 34) {
        bool sceneOk = g_worldScene.isValid();
        const auto& st = g_worldScene.getStoryState();
        const auto& mg = g_worldScene.getRhythmMinigame();

        std::cout << "[Phase 6 Self-Test] Rendered " << g_framesRendered
                  << " frames. MinigameNotesHit=" << mg.getNotesHit() << "/" << mg.getTotalNotes()
                  << ", MaxCombo=" << mg.getMaxCombo()
                  << ", Score=" << mg.getScore()
                  << ", Flag[completed_rhythm_minigame]=" << st.getFlag("completed_rhythm_minigame")
                  << ", Flag[story_act1_complete]=" << st.getFlag("story_act1_complete")
                  << ", Scene=" << (sceneOk ? "OK" : "FAIL")
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
    } else if (key == 'c' || key == 'C') {
        g_camera.setFreeCameraEnabled(!g_camera.isFreeCameraEnabled());
    } else if (key == 'p' || key == 'P') {
        g_camera.cycleProjectionMode();
    } else {
        g_worldScene.onKeyPress(key);
        if (key == 'r' || key == 'R') {
            g_camera.resetToDefault25D();
        }
    }
}

void keyboardUpCallback(unsigned char key, int x, int y) {
    input::InputManager::onKeyboardUp(key, x, y);
}

void specialDownCallback(int key, int x, int y) {
    input::InputManager::onSpecialDown(key, x, y);

    if (key == GLUT_KEY_F11) {
        g_fullscreen = !g_fullscreen;
        if (g_fullscreen) {
            glutFullScreen();
        } else {
            glutReshapeWindow(1280, 720);
        }
    } else {
        g_worldScene.onSpecialKeyPress(key);
    }
}

void specialUpCallback(int key, int x, int y) {
    input::InputManager::onSpecialUp(key, x, y);
}

void mouseButtonCallback(int button, int state, int x, int y) {
    input::InputManager::onMouseButton(button, state, x, y);
    g_worldScene.onMouseClick(button, state);
}

void timerCallback(int value) {
    (void)value;
    constexpr float dt = 0.016f;

    if (g_verifyPhase6Mode) {
        g_worldScene.runPhase6VerificationStep(dt, g_camera);
    } else {
        g_worldScene.update(dt, g_camera);
    }

    input::InputManager::endFrame();
    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, 0);
}

} // anonymous namespace

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--verify-phase0") == 0 ||
            std::strcmp(argv[i], "--verify-phase1") == 0 ||
            std::strcmp(argv[i], "--verify-phase2") == 0 ||
            std::strcmp(argv[i], "--verify-phase3") == 0 ||
            std::strcmp(argv[i], "--verify-phase4") == 0 ||
            std::strcmp(argv[i], "--verify-phase5") == 0 ||
            std::strcmp(argv[i], "--verify-phase6") == 0) {
            g_autoVerifyExit   = true;
            g_verifyPhase6Mode = true;
        }
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 60);
    glutCreateWindow("Echoes of November - 2.5D Narrative Adventure [Phase 6: Acoustic Rhythm Minigame]");

    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "[FATAL] glewInit() failed: "
                  << reinterpret_cast<const char*>(glewGetErrorString(glewStatus)) << std::endl;
        return 1;
    }

    core::Logger::initOpenGLDebugOutput();
    core::Logger::printContextInfo();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.07f, 0.07f, 0.12f, 1.0f);

    input::InputManager::init();
    g_worldScene.init();

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardDownCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialDownCallback);
    glutSpecialUpFunc(specialUpCallback);
    glutMouseFunc(mouseButtonCallback);
    glutMotionFunc(input::InputManager::onMouseMotion);
    glutPassiveMotionFunc(input::InputManager::onMousePassiveMotion);
    glutEntryFunc(input::InputManager::onMouseEntry);
    glutTimerFunc(16, timerCallback, 0);

    glutMainLoop();

    g_worldScene.destroy();
    return (core::Logger::getErrorCount() == 0) ? 0 : 2;
}
