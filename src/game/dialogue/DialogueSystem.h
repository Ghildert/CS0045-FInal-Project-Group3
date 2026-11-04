#ifndef GAME_DIALOGUE_DIALOGUESYSTEM_H
#define GAME_DIALOGUE_DIALOGUESYSTEM_H

#include <string>
#include <unordered_map>
#include <vector>

namespace game {
namespace dialogue {

struct DialogueLine {
    std::string speaker;
    std::string text;
};

struct DialogueSequence {
    std::string               id;
    std::vector<DialogueLine> lines;
};

class DialogueSystem {
public:
    DialogueSystem();

    void registerSequence(const DialogueSequence& seq);
    void loadDefaultClassroomDialogues();

    bool startDialogue(const std::string& sequenceId);
    void advanceOrCompleteLine();
    void update(float dt);

    void render2D(int screenWidth, int screenHeight) const;

    bool isActive() const { return m_active; }
    const std::string& getActiveSequenceId() const { return m_activeSequenceId; }
    int getCurrentLineIndex() const { return m_currentLineIndex; }
    int getCompletedDialoguesCount() const { return m_completedCount; }

private:
    std::unordered_map<std::string, DialogueSequence> m_sequences;
    bool        m_active;
    std::string m_activeSequenceId;
    int         m_currentLineIndex;
    float       m_charTimer;
    size_t      m_visibleChars;
    int         m_completedCount;
};

} // namespace dialogue
} // namespace game

#endif // GAME_DIALOGUE_DIALOGUESYSTEM_H
