/**
 * CS0045 Computer Graphics and Visual Computing
 * "Echoes of November" - 2.5D Narrative Adventure Game (C++17 & OpenGL Compatibility Profile)
 *
 * COMPLETE FINAL BUILD (Phases 0-8):
 * 1. Interactive 2.5D environments (Classroom Room 304, 3rd-Floor Hallway, Rainy Street / Sundown Mart)
 * 2. 2D animated pixel-art characters inside 3D spaces (Elijah, Maya, Sam)
 * 3. Player movement & 3D AABB sliding collision detection
 * 4. Interactive objects & NPCs with 3D floating indicators
 * 5. Branching dialogue, character portraits, story flags, & smartphone messaging [Tab]
 * 6. Smooth cinematic scene transitions
 * 7. Pixel-art textured 3D objects & environmental details (stb_image + GL_NEAREST + mipmaps)
 * 8. Phong lighting, spotlights, materials, dust motes, rain streaks & puddle ripples
 * 9. 2.5D follow camera & free-orbit camera with Perspective/Frustum/Orthographic projections
 * 10. Interactive Acoustic Guitar Rhythm Minigame [G]
 * 11. Main Menu, Pause Menu [ESC], Audio System (WAV synthesis + WinMM), & Save/Load [F5/F9]
 * 12. Interactive 8-Station OpenGL Coursework Demonstration Mode [F2] covering Modules 1-8
 */

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#include "engine/core/Logger.h"
#include "engine/input/InputManager.h"
#include "engine/audio/AudioSystem.h"
#include "engine/camera/Camera3D.h"
#include "engine/ui/MenuSystem.h"
#include "game/scenes/ClassroomScene.h"
#include "debug/OpenGLDemoMode.h"

#include <cstring>
#include <iostream>

using namespace engine;

namespace {

camera::Camera3D             g_camera;
ui::MenuSystem               g_menuSystem;
game::scenes::ClassroomScene g_worldScene;
debug::OpenGLDemoMode        g_demoMode;

bool g_autoVerifyExit   = false;
bool g_verifyFullSuite  = false;
int  g_framesRendered   = 0;
bool g_fullscreen       = false;

void handleMenuAction(ui::MenuAction action) {
    switch (action) {
        case ui::MenuAction::StartNewGame:
            g_menuSystem.setScreen(ui::MenuScreen::None);
            g_demoMode.setActive(false);
            g_worldScene.startNewGame();
            break;
        case ui::MenuAction::LoadSavedGame:
            g_menuSystem.setScreen(ui::MenuScreen::None);
            g_demoMode.setActive(false);
            g_worldScene.quickLoad();
            break;
        case ui::MenuAction::LaunchRhythmMinigame:
            g_menuSystem.setScreen(ui::MenuScreen::None);
            g_demoMode.setActive(false);
            g_worldScene.launchMinigameDirectly();
            break;
        case ui::MenuAction::LaunchOpenGLDemoMode:
            g_menuSystem.setScreen(ui::MenuScreen::None);
            g_demoMode.setActive(true);
            g_demoMode.selectModule(1);
            g_camera.resetToDefault25D();
            break;
        case ui::MenuAction::ResumeGame:
            g_menuSystem.setScreen(ui::MenuScreen::None);
            break;
        case ui::MenuAction::QuickSaveGame:
            g_worldScene.quickSave();
            g_menuSystem.setScreen(ui::MenuScreen::None);
            break;
        case ui::MenuAction::ReturnToMainMenu:
            g_demoMode.setActive(false);
            g_menuSystem.setScreen(ui::MenuScreen::MainMenu);
            break;
        case ui::MenuAction::QuitApplication:
            glutLeaveMainLoop();
            break;
        case ui::MenuAction::None:
        default:
            break;
    }
}

constexpr const char* kWindowTitle = "Echoes of November - 2.5D Narrative Adventure (CS0045 OpenGL)";

void ensureWindowVisibleAndFocused() {
    if (g_autoVerifyExit) {
        return;
    }
    glutShowWindow();
    glutPopWindow();
#ifdef _WIN32
    HWND hwnd = FindWindowA("FREEGLUT", kWindowTitle);
    if (!hwnd) {
        hwnd = FindWindowA(nullptr, kWindowTitle);
    }
    if (!hwnd) {
        hwnd = GetActiveWindow();
    }
    if (hwnd) {
        ShowWindow(hwnd, SW_RESTORE);
        ShowWindow(hwnd, SW_SHOWNORMAL);
        UpdateWindow(hwnd);
        SetForegroundWindow(hwnd);
        BringWindowToTop(hwnd);
    }
#endif
}

void displayCallback() {
    if (g_framesRendered <= 2) {
        ensureWindowVisibleAndFocused();
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_demoMode.isActive()) {
        g_demoMode.render3D(g_camera);
        g_demoMode.render2D(g_camera);
    } else {
        g_worldScene.render3D(g_camera);
        if (!g_menuSystem.isMenuOpen()) {
            g_worldScene.render2D(g_camera);
        } else {
            camera::Camera3D::begin2DOverlay(g_camera.getViewportWidth(), g_camera.getViewportHeight());
            g_menuSystem.render2D(g_camera.getViewportWidth(), g_camera.getViewportHeight());
            camera::Camera3D::end2DOverlay();
        }
    }

    glutSwapBuffers();
    glFlush();

    core::Logger::checkGLError("displayCallback");
    ++g_framesRendered;

    if (g_autoVerifyExit && g_framesRendered >= 40) {
        bool sceneOk = g_worldScene.isValid();
        const auto& st = g_worldScene.getStoryState();
        const auto& mg = g_worldScene.getRhythmMinigame();

        std::cout << "[Phase 8 Final Self-Test] Rendered " << g_framesRendered
                  << " frames. AllAssets=" << (sceneOk ? "OK" : "FAIL")
                  << ", LocationsMask=0x" << std::hex << g_worldScene.getLocationsVisitedCount()
                  << ", DemoModulesMask=0x" << g_demoMode.getModulesVisitedMask() << std::dec
                  << ", MinigameHits=" << mg.getNotesHit() << "/" << mg.getTotalNotes()
                  << ", StoryComplete=" << st.getFlag("story_act1_complete")
                  << ", GL Errors=" << core::Logger::getErrorCount() << std::endl;
        glutLeaveMainLoop();
    }
}

void reshapeCallback(int width, int height) {
    g_camera.setViewport(width, height);
}

void keyboardDownCallback(unsigned char key, int x, int y) {
    input::InputManager::onKeyboardDown(key, x, y);

    if (key == 27) { // ESC toggles Pause Menu / exits Demo Mode
        if (g_demoMode.isActive()) {
            g_demoMode.setActive(false);
        } else if (g_menuSystem.getScreen() == ui::MenuScreen::None) {
            g_menuSystem.setScreen(ui::MenuScreen::PauseMenu);
        } else if (g_menuSystem.getScreen() == ui::MenuScreen::PauseMenu) {
            g_menuSystem.setScreen(ui::MenuScreen::None);
        } else {
            glutLeaveMainLoop();
        }
        return;
    }

    if (g_menuSystem.isMenuOpen()) {
        handleMenuAction(g_menuSystem.onKeyPress(key));
        return;
    }

    if (g_demoMode.isActive()) {
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
        g_menuSystem.setScreen(ui::MenuScreen::None);
        g_demoMode.toggleActive();
        if (g_demoMode.isActive()) {
            g_camera.resetToDefault25D();
        }
        return;
    } else if (key == GLUT_KEY_F11) {
        g_fullscreen = !g_fullscreen;
        if (g_fullscreen) {
            glutFullScreen();
        } else {
            glutReshapeWindow(1280, 720);
        }
        return;
    }

    if (g_menuSystem.isMenuOpen()) {
        g_menuSystem.onSpecialKeyPress(key);
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
    if (!g_menuSystem.isMenuOpen() && !g_demoMode.isActive()) {
        g_worldScene.onMouseClick(button, state);
    }
}

void timerCallback(int value) {
    (void)value;
    constexpr float dt = 0.016f;

    if (g_verifyFullSuite) {
        // Frames 0..2: Render Main Menu & Pause Menu
        if (g_framesRendered == 1) {
            g_menuSystem.setScreen(ui::MenuScreen::PauseMenu);
        } else if (g_framesRendered == 2) {
            g_menuSystem.setScreen(ui::MenuScreen::None);
            g_worldScene.switchLocation(game::scenes::LocationId::Hallway, -2.5f, 0.3f, false);
        } else if (g_framesRendered >= 3 && g_framesRendered < 22) {
            g_worldScene.runPhase6VerificationStep(dt, g_camera);
        } else if (g_framesRendered >= 22) {
            g_demoMode.setActive(true);
            int station = ((g_framesRendered - 22) % 8) + 1;
            g_demoMode.selectModule(station);
            g_demoMode.update(dt);
        }
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
        if (std::strncmp(argv[i], "--verify", 8) == 0) {
            g_autoVerifyExit  = true;
            g_verifyFullSuite = true;
        }
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 60);
    glutCreateWindow(kWindowTitle);
    ensureWindowVisibleAndFocused();

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

    // Initialize Audio (muted during automated CLI verification so tests run silently)
    audio::AudioSystem::init(g_autoVerifyExit);
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

    std::cout << "[INFO] OpenGL window active! Use UP/DOWN + ENTER on the Main Menu (or press F2 for Coursework Demo Mode)." << std::endl;

    glutMainLoop();

    g_demoMode.destroy();
    g_worldScene.destroy();
    return (core::Logger::getErrorCount() == 0) ? 0 : 2;
}
