/**
 * CS0045 Computer Graphics and Visual Computing
 * "Echoes of November" - 2.5D Narrative Adventure Game (C++17 & OpenGL Compatibility Profile)
 *
 * PHASE 4 DELIVERABLE: Narrative and UI Systems
 * - Character Portraits in Dialogue Boxes (portrait_elijah.png, portrait_maya.png)
 * - Branching Dialogue Choices & Story Flags (StoryState)
 * - External Story & Smartphone Data Files (data/dialogue/story_dialogues.txt, phone_messages.txt)
 * - Interactive Smartphone Messaging Overlay [Tab] (PhoneSystem)
 * - Cinematic Scene Transitions (SceneTransition)
 * - Save/Load System [F5 / F9] in data/saves/save_slot_1.sav
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
game::scenes::ClassroomScene g_classroomScene;

bool g_autoVerifyExit   = false;
bool g_verifyPhase4Mode = false;
int  g_framesRendered   = 0;
bool g_fullscreen       = false;

void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    g_classroomScene.render3D(g_camera);
    g_classroomScene.render2D(g_camera);

    glutSwapBuffers();
    glFlush();

    core::Logger::checkGLError("displayCallback");
    ++g_framesRendered;

    if (g_autoVerifyExit && g_framesRendered >= 28) {
        bool sceneOk = g_classroomScene.isValid();
        const auto& st = g_classroomScene.getStoryState();

        std::cout << "[Phase 4 Self-Test] Rendered " << g_framesRendered
                  << " frames. Scene=" << (sceneOk ? "OK" : "FAIL")
                  << ", Flag[talked_to_maya]=" << st.getFlag("talked_to_maya")
                  << ", Flag[promised_rooftop_rehearsal]=" << st.getFlag("promised_rooftop_rehearsal")
                  << ", Flag[replied_phone_sam]=" << st.getFlag("replied_phone_sam")
                  << ", Flag[hallway_unlocked]=" << st.getFlag("hallway_unlocked")
                  << ", DialoguesCompleted=" << g_classroomScene.getDialogue().getCompletedDialoguesCount()
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
        g_classroomScene.onKeyPress(key);
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
        g_classroomScene.onSpecialKeyPress(key);
    }
}

void specialUpCallback(int key, int x, int y) {
    input::InputManager::onSpecialUp(key, x, y);
}

void mouseButtonCallback(int button, int state, int x, int y) {
    input::InputManager::onMouseButton(button, state, x, y);
    g_classroomScene.onMouseClick(button, state);
}

void timerCallback(int value) {
    (void)value;
    constexpr float dt = 0.016f;

    if (g_verifyPhase4Mode) {
        g_classroomScene.runPhase4VerificationStep(dt, g_camera);
    } else {
        g_classroomScene.update(dt, g_camera);
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
            std::strcmp(argv[i], "--verify-phase4") == 0) {
            g_autoVerifyExit   = true;
            g_verifyPhase4Mode = true;
        }
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 60);
    glutCreateWindow("Echoes of November - 2.5D Narrative Adventure [Phase 4: Narrative & UI Systems]");

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
    glClearColor(0.09f, 0.08f, 0.14f, 1.0f);

    input::InputManager::init();
    g_classroomScene.init();

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

    g_classroomScene.destroy();
    return (core::Logger::getErrorCount() == 0) ? 0 : 2;
}
