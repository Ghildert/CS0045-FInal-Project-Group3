#ifndef GAME_NARRATIVE_STORYSTATE_H
#define GAME_NARRATIVE_STORYSTATE_H

#include <string>
#include <unordered_map>

namespace game {
namespace narrative {

class StoryState {
public:
    StoryState();

    void resetNewGame();

    void setFlag(const std::string& key, int value = 1);
    int  getFlag(const std::string& key) const;
    bool hasFlag(const std::string& key) const;

    std::string getCurrentObjectiveText() const;

    // Status toast notification (e.g. "Saved to data/saves/save_slot_1.sav")
    void showToast(const std::string& message, float durationSeconds = 3.5f);
    void update(float dt);
    const std::string& getToastMessage() const { return m_toastMessage; }
    bool hasActiveToast() const { return m_toastTimer > 0.0f && !m_toastMessage.empty(); }

    // External Save/Load support in data/saves/
    bool saveToFile(
        const std::string& filePath,
        const std::string& currentSceneId,
        float playerX,
        float playerZ);

    bool loadFromFile(
        const std::string& filePath,
        std::string& outSceneId,
        float& outPlayerX,
        float& outPlayerZ);

    const std::unordered_map<std::string, int>& getAllFlags() const { return m_flags; }

private:
    std::unordered_map<std::string, int> m_flags;
    std::string                          m_toastMessage;
    float                                m_toastTimer;
};

} // namespace narrative
} // namespace game

#endif // GAME_NARRATIVE_STORYSTATE_H
