#ifndef ENGINE_UI_MENUSYSTEM_H
#define ENGINE_UI_MENUSYSTEM_H

#include <string>
#include <vector>

namespace engine {
namespace ui {

enum class MenuScreen {
    None = 0,
    MainMenu = 1,
    PauseMenu = 2
};

enum class MenuAction {
    None = 0,
    StartNewGame,
    LoadSavedGame,
    LaunchRhythmMinigame,
    LaunchOpenGLDemoMode,
    ResumeGame,
    QuickSaveGame,
    ReturnToMainMenu,
    QuitApplication
};

class MenuSystem {
public:
    MenuSystem();

    void setScreen(MenuScreen screen);
    MenuScreen getScreen() const { return m_screen; }
    bool isMenuOpen() const { return m_screen != MenuScreen::None; }

    void moveCursor(int delta);
    int  getSelectedIndex() const { return m_selectedIndex; }

    MenuAction confirmSelection();
    MenuAction onKeyPress(unsigned char key);
    void onSpecialKeyPress(int key);

    void render2D(int screenWidth, int screenHeight) const;

private:
    std::vector<std::string> getCurrentItems() const;

    MenuScreen m_screen;
    int        m_selectedIndex;
};

} // namespace ui
} // namespace engine

#endif // ENGINE_UI_MENUSYSTEM_H
