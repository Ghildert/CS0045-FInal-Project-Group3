#ifndef ENGINE_INPUT_INPUTMANAGER_H
#define ENGINE_INPUT_INPUTMANAGER_H

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <GL/freeglut.h>

namespace engine {
namespace input {

class InputManager {
public:
    static void init();
    static void endFrame();

    // GLUT callback handlers (Module 3 Subtopic 2)
    static void onKeyboardDown(unsigned char key, int x, int y);
    static void onKeyboardUp(unsigned char key, int x, int y);
    static void onSpecialDown(int key, int x, int y);
    static void onSpecialUp(int key, int x, int y);
    static void onMouseButton(int button, int state, int x, int y);
    static void onMouseMotion(int x, int y);
    static void onMousePassiveMotion(int x, int y);
    static void onMouseEntry(int state);

    // State queries
    static bool isKeyDown(unsigned char key);
    static bool isKeyPressed(unsigned char key);
    static bool isSpecialDown(int key);
    static bool isSpecialPressed(int key);

    static bool isMouseButtonDown(int button);
    static bool isMouseButtonPressed(int button);
    static int getMouseX();
    static int getMouseY();
    static int getMouseDeltaX();
    static int getMouseDeltaY();
    static bool isMouseInsideWindow();

private:
    static bool s_keyDown[256];
    static bool s_keyPressed[256];
    static bool s_specialDown[256];
    static bool s_specialPressed[256];

    static bool s_mouseButtonDown[8];
    static bool s_mouseButtonPressed[8];
    static int  s_mouseX;
    static int  s_mouseY;
    static int  s_mouseDeltaX;
    static int  s_mouseDeltaY;
    static bool s_mouseInitialized;
    static bool s_mouseInWindow;
};

} // namespace input
} // namespace engine

#endif // ENGINE_INPUT_INPUTMANAGER_H
