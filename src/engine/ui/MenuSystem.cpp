#include "engine/ui/MenuSystem.h"
#include "engine/ui/TextRenderer.h"
#include "engine/audio/AudioSystem.h"
#include <GL/freeglut.h>

namespace engine {
namespace ui {

MenuSystem::MenuSystem()
    : m_screen(MenuScreen::MainMenu)
    , m_selectedIndex(0)
{}

void MenuSystem::setScreen(MenuScreen screen) {
    m_screen        = screen;
    m_selectedIndex = 0;
}

std::vector<std::string> MenuSystem::getCurrentItems() const {
    if (m_screen == MenuScreen::MainMenu) {
        return {
            "[1] Start Story (Room 304 — November 14)",
            "[2] Continue / Load Save Slot 1",
            "[3] Acoustic Guitar Rhythm Minigame",
            "[4] OpenGL Coursework Demo Mode (Modules 1-8)",
            "[5] Quit Game"
        };
    } else if (m_screen == MenuScreen::PauseMenu) {
        return {
            "[1] Resume Story",
            "[2] Quick Save Game (Slot 1)",
            "[3] Quick Load Game (Slot 1)",
            "[4] OpenGL Coursework Demo Mode (Modules 1-8)",
            "[5] Return to Main Menu",
            "[6] Quit Game"
        };
    }
    return {};
}

void MenuSystem::moveCursor(int delta) {
    auto items = getCurrentItems();
    if (items.empty()) {
        return;
    }
    int n = static_cast<int>(items.size());
    m_selectedIndex = (m_selectedIndex + delta + n) % n;
    audio::AudioSystem::play(audio::SoundEvent::UIBlip);
}

MenuAction MenuSystem::confirmSelection() {
    audio::AudioSystem::play(audio::SoundEvent::DialogueSelect);

    if (m_screen == MenuScreen::MainMenu) {
        switch (m_selectedIndex) {
            case 0: return MenuAction::StartNewGame;
            case 1: return MenuAction::LoadSavedGame;
            case 2: return MenuAction::LaunchRhythmMinigame;
            case 3: return MenuAction::LaunchOpenGLDemoMode;
            case 4: return MenuAction::QuitApplication;
        }
    } else if (m_screen == MenuScreen::PauseMenu) {
        switch (m_selectedIndex) {
            case 0: return MenuAction::ResumeGame;
            case 1: return MenuAction::QuickSaveGame;
            case 2: return MenuAction::LoadSavedGame;
            case 3: return MenuAction::LaunchOpenGLDemoMode;
            case 4: return MenuAction::ReturnToMainMenu;
            case 5: return MenuAction::QuitApplication;
        }
    }
    return MenuAction::None;
}

MenuAction MenuSystem::onKeyPress(unsigned char key) {
    if (m_screen == MenuScreen::None) {
        return MenuAction::None;
    }

    auto items = getCurrentItems();
    if (key >= '1' && key <= '6') {
        int idx = key - '1';
        if (idx >= 0 && idx < static_cast<int>(items.size())) {
            m_selectedIndex = idx;
            return confirmSelection();
        }
    }

    if (key == 13 || key == 'e' || key == 'E' || key == ' ') {
        return confirmSelection();
    }
    return MenuAction::None;
}

void MenuSystem::onSpecialKeyPress(int key) {
    if (m_screen == MenuScreen::None) {
        return;
    }
    if (key == GLUT_KEY_UP) {
        moveCursor(-1);
    } else if (key == GLUT_KEY_DOWN) {
        moveCursor(1);
    }
}

void MenuSystem::render2D(int screenWidth, int screenHeight) const {
    if (m_screen == MenuScreen::None) {
        return;
    }

    float w = static_cast<float>(screenWidth);
    float h = static_cast<float>(screenHeight);

    float panelW = 580.0f;
    float panelH = (m_screen == MenuScreen::MainMenu) ? 410.0f : 445.0f;
    float panelX = (w - panelW) * 0.5f;
    float panelY = (h - panelH) * 0.5f;

    TextRenderer::drawPanel2D(
        panelX, panelY, panelW, panelH,
        0.04f, 0.05f, 0.11f, 0.94f,
        0.96f, 0.78f, 0.36f);

    std::string header = (m_screen == MenuScreen::MainMenu)
        ? "ECHOES OF NOVEMBER"
        : "GAME PAUSED";

    TextRenderer::drawStrokeText2D(
        panelX + 38.0f, panelY + panelH - 56.0f,
        header,
        0.22f, 2.4f, GLUT_STROKE_ROMAN,
        1.0f, 0.88f, 0.48f);

    TextRenderer::drawBitmapText2D(
        panelX + 40.0f, panelY + panelH - 86.0f,
        "2.5D Pixel-Art Narrative Adventure  |  CS0045 Computer Graphics & Visual Computing",
        GLUT_BITMAP_HELVETICA_12,
        0.82f, 0.90f, 0.98f);

    auto items = getCurrentItems();
    for (size_t i = 0; i < items.size(); ++i) {
        bool selected = (static_cast<int>(i) == m_selectedIndex);
        float btnW = panelW - 76.0f;
        float btnH = 42.0f;
        float btnX = panelX + 38.0f;
        float btnY = panelY + panelH - 150.0f - static_cast<float>(i) * 50.0f;

        TextRenderer::drawPanel2D(
            btnX, btnY, btnW, btnH,
            selected ? 0.20f : 0.09f,
            selected ? 0.16f : 0.11f,
            selected ? 0.32f : 0.19f,
            0.96f,
            selected ? 1.0f  : 0.45f,
            selected ? 0.84f : 0.58f,
            selected ? 0.36f : 0.72f);

        std::string label = (selected ? ">  " : "   ") + items[i];
        TextRenderer::drawBitmapText2D(
            btnX + 18.0f, btnY + 14.0f,
            label,
            GLUT_BITMAP_HELVETICA_18,
            selected ? 1.0f : 0.85f,
            selected ? 0.92f : 0.88f,
            selected ? 0.48f : 0.94f);
    }

    TextRenderer::drawBitmapText2D(
        panelX + 40.0f, panelY + 20.0f,
        "Navigate: [Up/Down Arrows] or [1..5]  |  Confirm: [Enter / E / Space]  |  Pause: [ESC]",
        GLUT_BITMAP_HELVETICA_12,
        0.72f, 0.84f, 0.92f);
}

} // namespace ui
} // namespace engine
