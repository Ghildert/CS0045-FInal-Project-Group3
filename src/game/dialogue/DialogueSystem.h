#ifndef GAME_DIALOGUE_DIALOGUESYSTEM_H
#define GAME_DIALOGUE_DIALOGUESYSTEM_H

#include "engine/assets/Texture2D.h"
#include "game/narrative/StoryState.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace game {
namespace dialogue {

struct DialogueLine {
    std::string speaker;
    std::string text;
};

struct DialogueChoice {
    std::string label;
    std::string targetNodeId;
    std::string setFlagKey;
    int         setFlagValue;
};

struct DialogueSequence {
    std::string                 id;
    std::vector<DialogueLine>   lines;
    std::vector<DialogueChoice> choices;
    std::string                 completionFlagKey;
    int                         completionFlagValue;
};

class DialogueSystem {
public:
    DialogueSystem();
    ~DialogueSystem();

    void initPortraitsAndData(const std::string& filePath = "data/dialogue/story_dialogues.txt");
    void destroy();

    static void ensureDefaultStoryFileOnDisk(const std::string& filePath = "data/dialogue/story_dialogues.txt");
    void loadFromExternalData(const std::string& filePath = "data/dialogue/story_dialogues.txt");

    void registerSequence(const DialogueSequence& seq);
    bool startDialogue(const std::string& sequenceId);
    void advanceOrCompleteLine(narrative::StoryState* storyState = nullptr);
    bool selectChoice(int choiceIndex, narrative::StoryState* storyState = nullptr);
    void moveChoiceCursor(int delta);
    void update(float dt);

    void render2D(int screenWidth, int screenHeight) const;

    bool isActive() const { return m_active; }
    bool isWaitingForChoice() const;
    const std::string& getActiveSequenceId() const { return m_activeSequenceId; }
    int getCurrentLineIndex() const { return m_currentLineIndex; }
    int getCompletedDialoguesCount() const { return m_completedCount; }

private:
    const engine::assets::Texture2D* getPortraitForSpeaker(const std::string& speaker) const;

    std::unordered_map<std::string, DialogueSequence> m_sequences;
    engine::assets::Texture2D m_portraitElijah;
    engine::assets::Texture2D m_portraitMaya;
    engine::assets::Texture2D m_portraitSam;

    bool        m_active;
    std::string m_activeSequenceId;
    int         m_currentLineIndex;
    int         m_selectedChoiceIndex;
    float       m_charTimer;
    size_t      m_visibleChars;
    int         m_completedCount;
};

} // namespace dialogue
} // namespace game

#endif // GAME_DIALOGUE_DIALOGUESYSTEM_H
