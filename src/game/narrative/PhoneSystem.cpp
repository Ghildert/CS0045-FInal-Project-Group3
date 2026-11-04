#include "game/narrative/PhoneSystem.h"
#include "engine/ui/TextRenderer.h"
#include <GL/freeglut.h>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace game {
namespace narrative {

PhoneSystem::PhoneSystem()
    : m_isOpen(false)
    , m_activeThreadIndex(0)
{}

void PhoneSystem::ensureDefaultPhoneDataOnDisk(const std::string& filePath) {
    if (std::filesystem::exists(filePath)) {
        return;
    }
    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
    std::ofstream out(filePath);
    out << "# Echoes of November - Smartphone Messaging Data\n";
    out << "THREAD|Sam (Band Club)|replied_phone_sam\n";
    out << "MSG|Sam|Yo Elijah! Are you and Maya bringing the acoustic guitar to the rooftop?\n";
    out << "MSG|Sam|Rain's starting soon, let me know if you're still in Room 304!\n";
    out << "CHOICE|On my way with the chords!|Grabbing my sketchbook in 304, see you in 5 mins!|Sweet! I'll unlock the stairwell door.|replied_phone_sam|1\n";
    out << "CHOICE|Running a bit late, start tuning.|Still talking with Maya, give me 10 minutes.|No rush bro, stay dry!|replied_phone_sam|2\n";
    out << "THREAD|Maya Lin|replied_phone_maya\n";
    out << "MSG|Maya|Don't forget the perspective drawing notebook on the front-left desk!\n";
    out << "CHOICE|Got it, thanks Maya!|Just picked it up, thanks for spotting it!|Anytime :) Let's head out when you're ready.|replied_phone_maya|1\n";
    out << "CHOICE|Wait, whose handwriting is in it?|Wait... did you write the rooftop note in the margin?|Haha maybe! Come talk to me by the window.|replied_phone_maya|2\n";
    out.close();
}

void PhoneSystem::loadFromExternalData(const std::string& filePath) {
    ensureDefaultPhoneDataOnDisk(filePath);
    m_threads.clear();

    std::ifstream in(filePath);
    if (!in.is_open()) {
        return;
    }

    std::string line;
    PhoneThread* current = nullptr;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::vector<std::string> parts;
        std::stringstream ss(line);
        std::string token;
        while (std::getline(ss, token, '|')) {
            parts.push_back(token);
        }
        if (parts.empty()) continue;

        if (parts[0] == "THREAD" && parts.size() >= 2) {
            m_threads.push_back(PhoneThread{ parts[1], {}, {}, false });
            current = &m_threads.back();
        } else if (parts[0] == "MSG" && parts.size() >= 3 && current) {
            current->messages.push_back(PhoneMessage{ parts[1], parts[2] });
        } else if (parts[0] == "CHOICE" && parts.size() >= 6 && current) {
            current->replyChoices.push_back(PhoneReplyChoice{
                parts[1],
                parts[2],
                parts[3],
                parts[4],
                std::stoi(parts[5])
            });
        }
    }
}

void PhoneSystem::toggleOpen() {
    m_isOpen = !m_isOpen;
}

void PhoneSystem::selectNextThread() {
    if (!m_threads.empty()) {
        m_activeThreadIndex = (m_activeThreadIndex + 1) % static_cast<int>(m_threads.size());
    }
}

void PhoneSystem::selectChoice(int choiceIndex, StoryState& storyState) {
    if (m_threads.empty()) {
        return;
    }
    auto& thread = m_threads[m_activeThreadIndex];
    if (thread.replied || choiceIndex < 0 || choiceIndex >= static_cast<int>(thread.replyChoices.size())) {
        return;
    }

    const auto& choice = thread.replyChoices[choiceIndex];
    thread.messages.push_back(PhoneMessage{ "Elijah", choice.sentMessageText });
    if (!choice.autoReplyText.empty()) {
        thread.messages.push_back(PhoneMessage{ thread.contactName, choice.autoReplyText });
    }
    thread.replied = true;
    storyState.setFlag(choice.setFlagKey, choice.setFlagValue);
    storyState.showToast("Sent message to " + thread.contactName, 2.8f);
}

int PhoneSystem::getUnreadCount(const StoryState& storyState) const {
    int unread = 0;
    if (!storyState.hasFlag("replied_phone_sam"))  ++unread;
    if (!storyState.hasFlag("replied_phone_maya")) ++unread;
    return unread;
}

void PhoneSystem::render2D(int screenWidth, int screenHeight, const StoryState& storyState) const {
    // 1. Always render small Smartphone Notification Pill in top-right corner
    int unread = getUnreadCount(storyState);
    float badgeW = 248.0f;
    float badgeH = 38.0f;
    float badgeX = static_cast<float>(screenWidth) - badgeW - 18.0f;
    float badgeY = static_cast<float>(screenHeight) - badgeH - 16.0f;

    engine::ui::TextRenderer::drawPanel2D(
        badgeX, badgeY, badgeW, badgeH,
        0.07f, 0.09f, 0.16f, 0.90f,
        unread > 0 ? 1.0f : 0.45f,
        unread > 0 ? 0.78f : 0.65f,
        unread > 0 ? 0.28f : 0.75f);

    std::string badgeText = "[Tab] Phone (" + std::to_string(unread) + " Unread)";
    engine::ui::TextRenderer::drawBitmapText2D(
        badgeX + 16.0f, badgeY + 12.0f,
        badgeText,
        GLUT_BITMAP_HELVETICA_18,
        unread > 0 ? 1.0f : 0.78f,
        unread > 0 ? 0.88f : 0.86f,
        unread > 0 ? 0.42f : 0.92f);

    if (!m_isOpen || m_threads.empty()) {
        return;
    }

    // 2. Render Full Smartphone Handset Overlay on the right side of the screen
    float phoneW = 390.0f;
    float phoneH = 470.0f;
    float phoneX = static_cast<float>(screenWidth) - phoneW - 22.0f;
    float phoneY = static_cast<float>(screenHeight) - phoneH - 64.0f;

    // Outer phone bezel
    engine::ui::TextRenderer::drawPanel2D(
        phoneX, phoneY, phoneW, phoneH,
        0.04f, 0.05f, 0.09f, 0.96f,
        0.88f, 0.72f, 0.40f);

    // Header bar
    const auto& thread = m_threads[m_activeThreadIndex];
    engine::ui::TextRenderer::drawPanel2D(
        phoneX + 10.0f, phoneY + phoneH - 52.0f, phoneW - 20.0f, 42.0f,
        0.12f, 0.16f, 0.28f, 0.98f,
        0.55f, 0.72f, 0.92f);

    engine::ui::TextRenderer::drawBitmapText2D(
        phoneX + 20.0f, phoneY + phoneH - 36.0f,
        "Chat: " + thread.contactName + "  [N: Switch Thread]",
        GLUT_BITMAP_HELVETICA_12,
        1.0f, 0.92f, 0.62f);

    // Render message bubbles
    float cursorY = phoneY + phoneH - 96.0f;
    for (const auto& msg : thread.messages) {
        bool isSelf = (msg.sender == "Elijah");
        float bubbleX = isSelf ? (phoneX + 36.0f) : (phoneX + 14.0f);
        float bubbleW = phoneW - 50.0f;
        float bubbleH = 44.0f;

        engine::ui::TextRenderer::drawPanel2D(
            bubbleX, cursorY, bubbleW, bubbleH,
            isSelf ? 0.16f : 0.11f,
            isSelf ? 0.28f : 0.14f,
            isSelf ? 0.42f : 0.22f,
            0.94f,
            isSelf ? 0.48f : 0.68f,
            isSelf ? 0.78f : 0.68f,
            isSelf ? 0.95f : 0.78f);

        engine::ui::TextRenderer::drawBitmapText2D(
            bubbleX + 10.0f, cursorY + 26.0f,
            msg.sender + ":",
            GLUT_BITMAP_HELVETICA_10,
            0.98f, 0.84f, 0.52f);

        std::string clipped = (msg.text.size() > 50) ? (msg.text.substr(0, 47) + "...") : msg.text;
        engine::ui::TextRenderer::drawBitmapText2D(
            bubbleX + 10.0f, cursorY + 10.0f,
            clipped,
            GLUT_BITMAP_HELVETICA_12,
            0.94f, 0.96f, 1.0f);

        cursorY -= 54.0f;
    }

    // Render Reply Choices at the bottom of the phone screen
    if (!thread.replied && !thread.replyChoices.empty()) {
        engine::ui::TextRenderer::drawBitmapText2D(
            phoneX + 18.0f, phoneY + 96.0f,
            "Choose Reply (Press [1] or [2]):",
            GLUT_BITMAP_HELVETICA_12,
            1.0f, 0.86f, 0.46f);

        for (size_t i = 0; i < thread.replyChoices.size(); ++i) {
            float btnY = phoneY + 54.0f - static_cast<float>(i) * 38.0f;
            engine::ui::TextRenderer::drawPanel2D(
                phoneX + 14.0f, btnY, phoneW - 28.0f, 32.0f,
                0.15f, 0.20f, 0.32f, 0.95f,
                0.95f, 0.78f, 0.35f);

            engine::ui::TextRenderer::drawBitmapText2D(
                phoneX + 24.0f, btnY + 10.0f,
                "[" + std::to_string(i + 1) + "] " + thread.replyChoices[i].label,
                GLUT_BITMAP_HELVETICA_12,
                0.98f, 0.95f, 0.78f);
        }
    } else {
        engine::ui::TextRenderer::drawBitmapText2D(
            phoneX + 22.0f, phoneY + 28.0f,
            "[Replied] Press [N] for next chat or [Tab] to close.",
            GLUT_BITMAP_HELVETICA_12,
            0.68f, 0.92f, 0.78f);
    }
}

} // namespace narrative
} // namespace game
