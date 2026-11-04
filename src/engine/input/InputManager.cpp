#include "engine/input/InputManager.h"
#include <cctype>
#include <cstring>

namespace engine {
namespace input {

bool InputManager::s_keyDown[256]            = { false };
bool InputManager::s_keyPressed[256]         = { false };
bool InputManager::s_specialDown[256]        = { false };
bool InputManager::s_specialPressed[256]     = { false };
bool InputManager::s_mouseButtonDown[8]      = { false };
bool InputManager::s_mouseButtonPressed[8]   = { false };
int  InputManager::s_mouseX                  = 0;
int  InputManager::s_mouseY                  = 0;
int  InputManager::s_mouseDeltaX             = 0;
int  InputManager::s_mouseDeltaY             = 0;
bool InputManager::s_mouseInitialized        = false;
bool InputManager::s_mouseInWindow           = true;

void InputManager::init() {
    std::memset(s_keyDown, 0, sizeof(s_keyDown));
    std::memset(s_keyPressed, 0, sizeof(s_keyPressed));
    std::memset(s_specialDown, 0, sizeof(s_specialDown));
    std::memset(s_specialPressed, 0, sizeof(s_specialPressed));
    std::memset(s_mouseButtonDown, 0, sizeof(s_mouseButtonDown));
    std::memset(s_mouseButtonPressed, 0, sizeof(s_mouseButtonPressed));
    s_mouseDeltaX = 0;
    s_mouseDeltaY = 0;
    s_mouseInitialized = false;
    s_mouseInWindow = true;
}

void InputManager::endFrame() {
    std::memset(s_keyPressed, 0, sizeof(s_keyPressed));
    std::memset(s_specialPressed, 0, sizeof(s_specialPressed));
    std::memset(s_mouseButtonPressed, 0, sizeof(s_mouseButtonPressed));
    s_mouseDeltaX = 0;
    s_mouseDeltaY = 0;
}

void InputManager::onKeyboardDown(unsigned char key, int x, int y) {
    (void)x;
    (void)y;
    unsigned char lower = static_cast<unsigned char>(std::tolower(key));
    if (!s_keyDown[key]) {
        s_keyPressed[key] = true;
    }
    if (!s_keyDown[lower]) {
        s_keyPressed[lower] = true;
    }
    s_keyDown[key]   = true;
    s_keyDown[lower] = true;
}

void InputManager::onKeyboardUp(unsigned char key, int x, int y) {
    (void)x;
    (void)y;
    unsigned char lower = static_cast<unsigned char>(std::tolower(key));
    unsigned char upper = static_cast<unsigned char>(std::toupper(key));
    s_keyDown[key]   = false;
    s_keyDown[lower] = false;
    s_keyDown[upper] = false;
}

void InputManager::onSpecialDown(int key, int x, int y) {
    (void)x;
    (void)y;
    if (key >= 0 && key < 256) {
        if (!s_specialDown[key]) {
            s_specialPressed[key] = true;
        }
        s_specialDown[key] = true;
    }
}

void InputManager::onSpecialUp(int key, int x, int y) {
    (void)x;
    (void)y;
    if (key >= 0 && key < 256) {
        s_specialDown[key] = false;
    }
}

void InputManager::onMouseButton(int button, int state, int x, int y) {
    s_mouseX = x;
    s_mouseY = y;
    if (button >= 0 && button < 8) {
        if (state == GLUT_DOWN) {
            if (!s_mouseButtonDown[button]) {
                s_mouseButtonPressed[button] = true;
            }
            s_mouseButtonDown[button] = true;
        } else if (state == GLUT_UP) {
            s_mouseButtonDown[button] = false;
        }
    }
}

void InputManager::onMouseMotion(int x, int y) {
    if (!s_mouseInitialized) {
        s_mouseX = x;
        s_mouseY = y;
        s_mouseInitialized = true;
    }
    s_mouseDeltaX += (x - s_mouseX);
    s_mouseDeltaY += (y - s_mouseY);
    s_mouseX = x;
    s_mouseY = y;
}

void InputManager::onMousePassiveMotion(int x, int y) {
    onMouseMotion(x, y);
}

void InputManager::onMouseEntry(int state) {
    if (state == GLUT_ENTERED) {
        s_mouseInWindow = true;
    } else if (state == GLUT_LEFT) {
        s_mouseInWindow = false;
    }
}

bool InputManager::isKeyDown(unsigned char key) {
    return s_keyDown[key];
}

bool InputManager::isKeyPressed(unsigned char key) {
    return s_keyPressed[key];
}

bool InputManager::isSpecialDown(int key) {
    return (key >= 0 && key < 256) ? s_specialDown[key] : false;
}

bool InputManager::isSpecialPressed(int key) {
    return (key >= 0 && key < 256) ? s_specialPressed[key] : false;
}

bool InputManager::isMouseButtonDown(int button) {
    return (button >= 0 && button < 8) ? s_mouseButtonDown[button] : false;
}

bool InputManager::isMouseButtonPressed(int button) {
    return (button >= 0 && button < 8) ? s_mouseButtonPressed[button] : false;
}

int InputManager::getMouseX() {
    return s_mouseX;
}

int InputManager::getMouseY() {
    return s_mouseY;
}

int InputManager::getMouseDeltaX() {
    return s_mouseDeltaX;
}

int InputManager::getMouseDeltaY() {
    return s_mouseDeltaY;
}

bool InputManager::isMouseInsideWindow() {
    return s_mouseInWindow;
}

} // namespace input
} // namespace engine
