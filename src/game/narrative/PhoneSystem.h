#ifndef GAME_NARRATIVE_PHONESYSTEM_H
#define GAME_NARRATIVE_PHONESYSTEM_H

#include "game/narrative/StoryState.h"
#include <string>
#include <vector>

namespace game {
namespace narrative {

struct PhoneMessage {
    std::string sender; // "Elijah" (right-aligned) or contact name (left-aligned)
    std::string text;
};

struct PhoneReplyChoice {
    std::string label;
    std::string sentMessageText;
    std::string autoReplyText;
    std::string setFlagKey;
    int         setFlagValue;
};

struct PhoneThread {
    std::string                   contactName;
    std::vector<PhoneMessage>     messages;
    std::vector<PhoneReplyChoice> replyChoices;
    bool                          replied;
};

class PhoneSystem {
public:
    PhoneSystem();

    void loadFromExternalData(const std::string& filePath = "data/dialogue/phone_messages.txt");
    static void ensureDefaultPhoneDataOnDisk(const std::string& filePath = "data/dialogue/phone_messages.txt");

    void toggleOpen();
    void setOpen(bool open) { m_isOpen = open; }
    bool isOpen() const { return m_isOpen; }

    void selectNextThread();
    void selectChoice(int choiceIndex, StoryState& storyState);

    int getUnreadCount(const StoryState& storyState) const;
    void render2D(int screenWidth, int screenHeight, const StoryState& storyState) const;

    const std::vector<PhoneThread>& getThreads() const { return m_threads; }

private:
    bool                     m_isOpen;
    int                      m_activeThreadIndex;
    std::vector<PhoneThread> m_threads;
};

} // namespace narrative
} // namespace game

#endif // GAME_NARRATIVE_PHONESYSTEM_H
