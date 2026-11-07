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
            "Start Story (Room 304 - November 14)",
            "Continue / Load Save Slot 1",
            "Acoustic Guitar Rhythm Minigame",
            "OpenGL Coursework Demo Mode (Modules 1-8)",
            "Quit Game"
        };
    } else if (m_screen == MenuScreen::PauseMenu) {
        return {
            "Resume Story",
            "Quick Save Game (Slot 1)",
            "Quick Load Game (Slot 1)",
            "OpenGL Coursework Demo Mode (Modules 1-8)",
            "Return to Main Menu",
            "Quit Game"
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

    // HTML/CSS backdrop-filter style darkened overlay
    TextRenderer::drawAccentBar2D(0.0f, 0.0f, w, h, 0.03f, 0.05f, 0.09f, 0.56f);

    float panelW = (w - 48.0f < 590.0f) ? (w - 48.0f) : 590.0f;
    float panelH = (m_screen == MenuScreen::MainMenu) ? 438.0f : 482.0f;
    if (panelH > h - 28.0f) {
        panelH = h - 28.0f;
    }
    float panelX = (w - panelW) * 0.5f;
    float panelY = (h - panelH) * 0.5f;
    float centerX = panelX + panelW * 0.5f;

    // Main HTML-styled card container (border-radius: 16px, slate-900 glass fill, sleek 1.4px slate-400 border)
    TextRenderer::drawRoundedRect2D(
        panelX, panelY, panelW, panelH, 16.0f,
        0.06f, 0.09f, 0.16f, 0.95f,
        0.45f, 0.54f, 0.68f, 0.48f,
        1.4f, true);

    // Header surface card inside top of modal
    float headerH = 92.0f;
    TextRenderer::drawRoundedRect2D(
        panelX + 12.0f, panelY + panelH - headerH - 12.0f,
        panelW - 24.0f, headerH, 12.0f,
        0.09f, 0.13f, 0.23f, 0.92f,
        0.38f, 0.48f, 0.64f, 0.32f,
        1.0f, false);

    std::string header = (m_screen == MenuScreen::MainMenu)
        ? "ECHOES OF NOVEMBER"
        : "GAME PAUSED";

    TextRenderer::drawStrokeTextCentered2D(
        centerX, panelY + panelH - 54.0f,
        header,
        0.21f, 2.3f, GLUT_STROKE_ROMAN,
        0.98f, 0.86f, 0.46f);

    TextRenderer::drawBitmapTextCentered2D(
        centerX, panelY + panelH - 84.0f,
        "2.5D Pixel-Art Narrative Adventure   |   CS0045 Visual Computing",
        GLUT_BITMAP_HELVETICA_12,
        0.74f, 0.84f, 0.95f);

    auto items = getCurrentItems();
    float listTopY = panelY + panelH - 160.0f;
    for (size_t i = 0; i < items.size(); ++i) {
        bool selected = (static_cast<int>(i) == m_selectedIndex);
        float btnW = panelW - 56.0f;
        float btnH = 42.0f;
        float btnX = panelX + 28.0f;
        float btnY = listTopY - static_cast<float>(i) * 48.0f;

        // HTML Button styling (rounded 10px, smooth hover/focus ring without gold outline)
        TextRenderer::drawRoundedRect2D(
            btnX, btnY, btnW, btnH, 10.0f,
            selected ? 0.13f : 0.08f,
            selected ? 0.22f : 0.11f,
            selected ? 0.38f : 0.20f,
            0.96f,
            selected ? 0.48f : 0.36f,
            selected ? 0.74f : 0.44f,
            selected ? 0.98f : 0.56f,
            selected ? 0.85f : 0.38f,
            selected ? 1.6f  : 1.0f,
            selected);

        // Keycap pill badge on the left of each menu button
        std::string keyBadge = std::to_string(i + 1);
        TextRenderer::drawPillBadge2D(
            btnX + 12.0f, btnY + 9.0f, 28.0f, 24.0f,
            keyBadge,
            selected ? 0.22f : 0.12f,
            selected ? 0.46f : 0.17f,
            selected ? 0.78f : 0.28f,
            0.98f,
            selected ? 0.54f : 0.42f,
            selected ? 0.78f : 0.52f,
            selected ? 0.98f : 0.65f,
            selected ? 0.96f : 0.78f,
            selected ? 0.98f : 0.84f,
            selected ? 1.0f  : 0.92f,
            GLUT_BITMAP_HELVETICA_12);

        TextRenderer::drawBitmapText2D(
            btnX + 52.0f, btnY + 14.0f,
            items[i],
            GLUT_BITMAP_HELVETICA_18,
            selected ? 0.96f : 0.84f,
            selected ? 0.98f : 0.89f,
            selected ? 1.0f  : 0.95f);

        if (selected) {
            TextRenderer::drawPillBadge2D(
                btnX + btnW - 68.0f, btnY + 10.0f, 54.0f, 22.0f,
                "ENTER",
                0.18f, 0.28f, 0.45f, 0.92f,
                0.56f, 0.76f, 0.96f,
                0.92f, 0.96f, 1.0f,
                GLUT_BITMAP_HELVETICA_10);
        }
    }

    // Subtle footer pill bar
    TextRenderer::drawRoundedRect2D(
        panelX + 28.0f, panelY + 14.0f, panelW - 56.0f, 28.0f, 8.0f,
        0.05f, 0.07f, 0.13f, 0.85f,
        0.35f, 0.44f, 0.58f, 0.28f,
        1.0f, false);

    TextRenderer::drawBitmapTextCentered2D(
        centerX, panelY + 23.0f,
        "Navigate: [Up / Down] or [1..5]    |    Confirm: [Enter / E / Space]    |    Pause: [ESC]",
        GLUT_BITMAP_HELVETICA_12,
        0.68f, 0.80f, 0.92f);
}

} // namespace ui
} // namespace engine
