/**
 * CS0045 Computer Graphics and Visual Computing
 * "Echoes of November" - 2.5D Narrative Adventure Game (C++17 & OpenGL Compatibility Profile)
 *
 * PHASE 7 DELIVERABLE: OpenGL Demonstration Mode (Modules 1-8 Grading Showcase)
 * - Accessible anytime via [F2]
 * - Interactive 8-station coursework showcase covering Modules 1 through 8
 * - Displays which module each station represents, the exact OpenGL APIs demonstrated,
 *   and where the same concept is used in the game
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
#include "debug/OpenGLDemoMode.h"

#include <cstring>
#include <iostream>

using namespace engine;

namespace {

camera::Camera3D             g_camera;
game::scenes::ClassroomScene g_worldScene;
debug::OpenGLDemoMode        g_demoMode;

bool g_autoVerifyExit   = false;
bool g_verifyPhase7Mode = false;
int  g_framesRendered   = 0;
bool g_fullscreen       = false;

void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_demoMode.isActive()) {
        g_demoMode.render3D(g_camera);
        g_demoMode.render2D(g_camera);
    } else {
        g_worldScene.render3D(g_camera);
        g_worldScene.render2D(g_camera);
    }

    glutSwapBuffers();
    glFlush();

    core::Logger::checkGLError("displayCallback");
    ++g_framesRendered;

    if (g_autoVerifyExit && g_framesRendered >= 20) {
        std::cout << "[Phase 7 Self-Test] Rendered " << g_framesRendered
                  << " frames. DemoModulesVisitedMask=0x" << std::hex << g_demoMode.getModulesVisitedMask() << std::dec
                  << " (All8=" << (g_demoMode.getModulesVisitedMask() == 0xFF ? "YES" : "NO") << ")"
                  << ", ActiveStation=" << g_demoMode.getCurrentModuleNumber()
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
    } else if (g_demoMode.isActive()) {
        g_demoMode.onKeyPress(key, g_camera);
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

    if (key == GLUT_KEY_F2) {
        g_demoMode.toggleActive();
        if (g_demoMode.isActive()) {
            g_camera.resetToDefault25D();
        }
    } else if (key == GLUT_KEY_F11) {
        g_fullscreen = !g_fullscreen;
        if (g_fullscreen) {
            glutFullScreen();
        } else {
            glutReshapeWindow(1280, 720);
        }
    } else if (g_demoMode.isActive()) {
        g_demoMode.onSpecialKeyPress(key);
    } else {
        g_worldScene.onSpecialKeyPress(key);
    }
}

void specialUpCallback(int key, int x, int y) {
    input::InputManager::onSpecialUp(key, x, y);
}

void mouseButtonCallback(int button, int state, int x, int y) {
    input::InputManager::onMouseButton(button, state, x, y);
    if (!g_demoMode.isActive()) {
        g_worldScene.onMouseClick(button, state);
    }
}

void timerCallback(int value) {
    (void)value;
    constexpr float dt = 0.016f;

    if (g_verifyPhase7Mode) {
        if (!g_demoMode.isActive()) {
            g_demoMode.setActive(true);
            g_demoMode.selectModule(1);
        } else {
            // Step through all 8 stations across frames
            int station = ((g_framesRendered / 2) % 8) + 1;
            g_demoMode.selectModule(station);
        }
        g_demoMode.update(dt);
    } else if (g_demoMode.isActive()) {
        g_demoMode.update(dt);
        g_camera.updateFreeOrbit(dt);
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
            std::strcmp(argv[i], "--verify-phase6") == 0 ||
            std::strcmp(argv[i], "--verify-phase7") == 0) {
            g_autoVerifyExit   = true;
            g_verifyPhase7Mode = true;
        }
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 60);
    glutCreateWindow("Echoes of November - 2.5D Narrative Adventure [Phase 7: OpenGL Demo Mode]");

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
    g_demoMode.init();

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

    g_demoMode.destroy();
    g_worldScene.destroy();
    return (core::Logger::getErrorCount() == 0) ? 0 : 2;
}
