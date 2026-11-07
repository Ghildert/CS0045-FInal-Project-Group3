#include "game/narrative/PhoneSystem.h"
#include "engine/ui/TextRenderer.h"
#include "engine/audio/AudioSystem.h"
#include <GL/freeglut.h>
#include <cmath>
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
    engine::audio::AudioSystem::play(engine::audio::SoundEvent::UIBlip);
}

void PhoneSystem::selectNextThread() {
    if (!m_threads.empty()) {
        m_activeThreadIndex = (m_activeThreadIndex + 1) % static_cast<int>(m_threads.size());
        engine::audio::AudioSystem::play(engine::audio::SoundEvent::UIBlip);
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

    engine::audio::AudioSystem::play(engine::audio::SoundEvent::DialogueSelect);

    const auto& choice = thread.replyChoices[choiceIndex];
    thread.messages.push_back(PhoneMessage{ "Elijah", choice.sentMessageText });
    if (!choice.autoReplyText.empty()) {
        thread.messages.push_back(PhoneMessage{ thread.contactName, choice.autoReplyText });
    }
    thread.replied = true;
    storyState.setFlag(choice.setFlagKey, choice.setFlagValue);
    storyState.showToast("Sent message to " + thread.contactName, 2.8f);
}

void PhoneSystem::autoCompleteAllReplies(StoryState& storyState) {
    for (auto& thread : m_threads) {
        if (!thread.replied && !thread.replyChoices.empty()) {
            const auto& choice = thread.replyChoices[0];
            thread.messages.push_back(PhoneMessage{ "Elijah", choice.sentMessageText });
            if (!choice.autoReplyText.empty()) {
                thread.messages.push_back(PhoneMessage{ thread.contactName, choice.autoReplyText });
            }
            thread.replied = true;
            storyState.setFlag(choice.setFlagKey, choice.setFlagValue);
        }
    }
    storyState.setFlag("replied_phone_sam", 1);
    storyState.setFlag("replied_phone_maya", 1);
}

int PhoneSystem::getUnreadCount(const StoryState& storyState) const {
    int unread = 0;
    if (!storyState.hasFlag("replied_phone_sam"))  ++unread;
    if (!storyState.hasFlag("replied_phone_maya")) ++unread;
    return unread;
}

void PhoneSystem::render2D(int screenWidth, int screenHeight, const StoryState& storyState) const {
    int unread = getUnreadCount(storyState);

    // -------------------------------------------------------------------------
    // 1. Top-Right Smartphone Mockup Widget with Notification (When Closed)
    //    Styled in sleek Phone Colors: Black Bezel, Titanium Gray Frame, White/Gray Screen
    // -------------------------------------------------------------------------
    if (!m_isOpen || m_threads.empty()) {
        const float miniW = 116.0f;
        const float miniH = 198.0f;
        const float miniX = std::round(static_cast<float>(screenWidth) - miniW - 18.0f);
        const float miniY = std::round(static_cast<float>(screenHeight) - miniH - 14.0f);

        // Titanium-Gray Outer Chassis + Jet-Black Bezel
        engine::ui::TextRenderer::drawRoundedRect2D(
            miniX, miniY, miniW, miniH, 22.0f,
            0.10f, 0.10f, 0.12f, 0.98f,
            0.55f, 0.57f, 0.60f, 0.88f,
            2.0f, true);

        // Inner White/Light-Gray Smartphone Screen
        const float scrX = miniX + 7.0f;
        const float scrY = miniY + 9.0f;
        const float scrW = miniW - 14.0f;
        const float scrH = miniH - 18.0f;

        engine::ui::TextRenderer::drawRoundedRect2D(
            scrX, scrY, scrW, scrH, 16.0f,
            0.95f, 0.96f, 0.97f, 0.99f,
            0.78f, 0.80f, 0.83f, 0.90f,
            1.0f, false);

        // Top Dynamic Island Pill on Screen
        engine::ui::TextRenderer::drawRoundedRect2D(
            scrX + (scrW - 38.0f) * 0.5f, scrY + scrH - 12.0f, 38.0f, 6.5f, 3.2f,
            0.08f, 0.08f, 0.10f, 0.98f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, false);

        // Large Lockscreen Clock & Date in Charcoal Black/Gray
        engine::ui::TextRenderer::drawBitmapTextCentered2D(
            scrX + scrW * 0.5f, scrY + scrH - 34.0f,
            "17:42",
            GLUT_BITMAP_HELVETICA_18,
            0.08f, 0.09f, 0.11f);

        engine::ui::TextRenderer::drawBitmapTextCentered2D(
            scrX + scrW * 0.5f, scrY + scrH - 50.0f,
            "Thu, Nov 14",
            GLUT_BITMAP_HELVETICA_12,
            0.36f, 0.38f, 0.42f);

        // White Notification Card with Soft Gray Border in the center of the lockscreen
        const float notifCardX = scrX + 6.0f;
        const float notifCardY = scrY + 42.0f;
        const float notifCardW = scrW - 12.0f;
        const float notifCardH = 66.0f;

        engine::ui::TextRenderer::drawRoundedRect2D(
            notifCardX, notifCardY, notifCardW, notifCardH, 10.0f,
            1.0f,  1.0f,  1.0f,  0.99f,
            0.76f, 0.78f, 0.82f, 0.95f,
            1.2f, false);

        engine::ui::TextRenderer::drawBitmapTextCentered2D(
            scrX + scrW * 0.5f, notifCardY + 46.0f,
            "MESSAGES",
            GLUT_BITMAP_HELVETICA_12,
            0.38f, 0.40f, 0.45f);

        std::string subLabel = unread > 0
            ? (std::to_string(unread) + " Unread")
            : "All Read";
        engine::ui::TextRenderer::drawBitmapTextCentered2D(
            scrX + scrW * 0.5f, notifCardY + 26.0f,
            subLabel,
            GLUT_BITMAP_HELVETICA_12,
            0.06f, 0.07f, 0.09f);

        engine::ui::TextRenderer::drawBitmapTextCentered2D(
            scrX + scrW * 0.5f, notifCardY + 8.0f,
            unread > 0 ? "Sam & Maya" : "Up to date",
            GLUT_BITMAP_HELVETICA_12,
            0.42f, 0.44f, 0.48f);

        // Dark Charcoal [Press TAB] Pill at bottom of lockscreen
        engine::ui::TextRenderer::drawPillBadge2D(
            scrX + 8.0f, scrY + 12.0f, scrW - 16.0f, 22.0f,
            "Press [TAB]",
            0.14f, 0.15f, 0.18f, 0.98f,
            0.34f, 0.36f, 0.40f,
            0.98f, 0.98f, 1.0f,
            GLUT_BITMAP_HELVETICA_12);

        // Bottom Home Indicator Bar
        engine::ui::TextRenderer::drawRoundedRect2D(
            scrX + (scrW - 36.0f) * 0.5f, scrY + 4.0f, 36.0f, 3.5f, 1.7f,
            0.20f, 0.22f, 0.25f, 0.85f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, false);

        // Red Circular Notification Badge on Top-Right Corner of the Phone
        if (unread > 0) {
            engine::ui::TextRenderer::drawPillBadge2D(
                miniX + miniW - 20.0f, miniY + miniH - 20.0f, 28.0f, 28.0f,
                std::to_string(unread),
                0.90f, 0.20f, 0.24f, 0.99f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                GLUT_BITMAP_HELVETICA_18);
        }
        return;
    }

    // -------------------------------------------------------------------------
    // 2. Centered Sleek Portrait Smartphone Mockup UI (Open via [TAB])
    //    Realistic vertical smartphone aspect ratio (~9:16) with crisp pixel-aligned text
    // -------------------------------------------------------------------------
    const float sw = static_cast<float>(screenWidth);
    const float sh = static_cast<float>(screenHeight);

    // Subtle darkened backdrop behind the centered phone mockup
    engine::ui::TextRenderer::drawAccentBar2D(0.0f, 0.0f, sw, sh, 0.02f, 0.02f, 0.03f, 0.52f);

    const auto& thread = m_threads[m_activeThreadIndex];
    float phoneW = 344.0f;
    if (phoneW > sw - 48.0f) {
        phoneW = sw - 48.0f;
    }
    const float maxBubbleTextW = phoneW - 84.0f;

    // Pre-calculate total height of wrapped messages & reply choices
    float totalMessagesH = 0.0f;
    for (const auto& msg : thread.messages) {
        auto layout = engine::ui::TextRenderer::computeWrappedLayout(
            msg.text, maxBubbleTextW, 17.0f, GLUT_BITMAP_HELVETICA_12);
        float bh = layout.totalHeight + 32.0f;
        totalMessagesH += bh + 10.0f;
    }

    float choicesAreaH = 54.0f;
    if (!thread.replied && !thread.replyChoices.empty()) {
        choicesAreaH = 38.0f;
        for (const auto& ch : thread.replyChoices) {
            auto cl = engine::ui::TextRenderer::computeWrappedLayout(
                ch.label, phoneW - 92.0f, 17.0f, GLUT_BITMAP_HELVETICA_12);
            float chH = cl.totalHeight + 18.0f;
            if (chH < 36.0f) chH = 36.0f;
            choicesAreaH += chH + 8.0f;
        }
    }

    float phoneH = 114.0f + totalMessagesH + choicesAreaH + 28.0f;
    if (phoneH < 576.0f) phoneH = 576.0f;
    if (phoneH > sh - 52.0f) {
        phoneH = sh - 52.0f;
    }

    // Snap phone position to integer pixel coordinates for crisp text & borders
    const float phoneX = std::round((sw - phoneW) * 0.5f);
    const float phoneY = std::round((sh - phoneH) * 0.5f);

    // Physical Side Hardware Buttons (Volume Up/Down on Left, Power Button on Right)
    engine::ui::TextRenderer::drawRoundedRect2D(
        phoneX - 4.0f, phoneY + phoneH - 135.0f, 5.0f, 38.0f, 2.5f,
        0.32f, 0.33f, 0.36f, 0.98f,
        0.52f, 0.54f, 0.58f, 0.80f, 1.0f, false);
    engine::ui::TextRenderer::drawRoundedRect2D(
        phoneX - 4.0f, phoneY + phoneH - 182.0f, 5.0f, 38.0f, 2.5f,
        0.32f, 0.33f, 0.36f, 0.98f,
        0.52f, 0.54f, 0.58f, 0.80f, 1.0f, false);
    engine::ui::TextRenderer::drawRoundedRect2D(
        phoneX + phoneW - 1.0f, phoneY + phoneH - 160.0f, 5.0f, 54.0f, 2.5f,
        0.32f, 0.33f, 0.36f, 0.98f,
        0.52f, 0.54f, 0.58f, 0.80f, 1.0f, false);

    // 1. Outer Brushed Titanium-Gray Smartphone Frame
    engine::ui::TextRenderer::drawRoundedRect2D(
        phoneX, phoneY, phoneW, phoneH, 38.0f,
        0.24f, 0.25f, 0.27f, 0.99f,
        0.62f, 0.64f, 0.68f, 0.92f,
        2.0f, true);

    // 2. Inner Jet-Black Hardware Bezel
    engine::ui::TextRenderer::drawRoundedRect2D(
        phoneX + 4.0f, phoneY + 4.0f, phoneW - 8.0f, phoneH - 8.0f, 35.0f,
        0.05f, 0.05f, 0.06f, 1.0f,
        0.15f, 0.16f, 0.18f, 0.90f,
        1.0f, false);

    // 3. Large Bright Light-Gray / White Display Screen (#F4F5F7)
    const float scrX = phoneX + 12.0f;
    const float scrY = phoneY + 12.0f;
    const float scrW = phoneW - 24.0f;
    const float scrH = phoneH - 24.0f;

    engine::ui::TextRenderer::drawRoundedRect2D(
        scrX, scrY, scrW, scrH, 27.0f,
        0.95f, 0.96f, 0.97f, 1.0f,
        0.82f, 0.84f, 0.86f, 1.0f,
        1.0f, false);

    // 4. Top Status Bar & Jet-Black Dynamic Island Notch
    const float islandW = 92.0f;
    const float islandH = 20.0f;
    const float islandX = std::round(scrX + (scrW - islandW) * 0.5f);
    const float islandY = std::round(scrY + scrH - 26.0f);

    engine::ui::TextRenderer::drawRoundedRect2D(
        islandX, islandY, islandW, islandH, 10.0f,
        0.06f, 0.06f, 0.07f, 1.0f,
        0.20f, 0.20f, 0.22f, 0.80f,
        1.0f, false);

    // Camera lens detail inside Dynamic Island
    engine::ui::TextRenderer::drawRoundedRect2D(
        islandX + islandW - 20.0f, islandY + 5.0f, 10.0f, 10.0f, 5.0f,
        0.14f, 0.15f, 0.18f, 1.0f,
        0.28f, 0.30f, 0.34f, 0.80f,
        1.0f, false);

    // Status bar time (left) & signal/battery (right) in crisp Charcoal Black
    engine::ui::TextRenderer::drawBitmapText2D(
        scrX + 20.0f, scrY + scrH - 21.0f,
        "17:42",
        GLUT_BITMAP_HELVETICA_12,
        0.08f, 0.09f, 0.11f);

    engine::ui::TextRenderer::drawBitmapText2D(
        scrX + scrW - 76.0f, scrY + scrH - 21.0f,
        "5G  100%",
        GLUT_BITMAP_HELVETICA_12,
        0.12f, 0.13f, 0.16f);

    // 5. Pure White Messaging App Header Bar (#FFFFFF) with Gray Divider
    const float hdrH = 44.0f;
    const float hdrY = std::round(scrY + scrH - 34.0f - hdrH);

    engine::ui::TextRenderer::drawRoundedRect2D(
        scrX + 6.0f, hdrY, scrW - 12.0f, hdrH, 12.0f,
        1.0f,  1.0f,  1.0f,  0.99f,
        0.82f, 0.84f, 0.87f, 0.95f,
        1.2f, false);

    // Contact Avatar Badge in Charcoal Black with White Initial
    std::string initial = thread.contactName.empty() ? "?" : thread.contactName.substr(0, 1);
    engine::ui::TextRenderer::drawPillBadge2D(
        scrX + 14.0f, hdrY + 8.0f, 28.0f, 28.0f,
        initial,
        0.14f, 0.15f, 0.18f, 1.0f,
        0.36f, 0.38f, 0.42f,
        1.0f,  1.0f,  1.0f,
        GLUT_BITMAP_HELVETICA_12);

    // Contact Name in Crisp Charcoal Black
    engine::ui::TextRenderer::drawBitmapText2D(
        scrX + 48.0f, hdrY + 16.0f,
        thread.contactName,
        GLUT_BITMAP_HELVETICA_18,
        0.06f, 0.07f, 0.09f);

    // [N] Next Chat Gray Pill Button on the right of the header
    engine::ui::TextRenderer::drawPillBadge2D(
        scrX + scrW - 102.0f, hdrY + 10.0f, 88.0f, 24.0f,
        "[N] Chat",
        0.90f, 0.91f, 0.93f, 1.0f,
        0.72f, 0.74f, 0.78f,
        0.12f, 0.13f, 0.16f,
        GLUT_BITMAP_HELVETICA_12);

    // 6. Render Message Bubbles (Incoming = White/Light-Gray, Outgoing = Charcoal Black)
    float cursorTopY = std::round(hdrY - 12.0f);
    for (const auto& msg : thread.messages) {
        bool isSelf = (msg.sender == "Elijah");
        auto layout = engine::ui::TextRenderer::computeWrappedLayout(
            msg.text, maxBubbleTextW, 17.0f, GLUT_BITMAP_HELVETICA_12);
        std::string senderTag = isSelf ? "You" : msg.sender;
        int senderW = engine::ui::TextRenderer::measureBitmapTextWidth(
            senderTag, GLUT_BITMAP_HELVETICA_12);

        float contentW = layout.maxLineWidth;
        if (static_cast<float>(senderW) > contentW) {
            contentW = static_cast<float>(senderW);
        }
        float bubbleW = std::round(contentW + 26.0f);
        if (bubbleW < 110.0f) bubbleW = 110.0f;
        if (bubbleW > scrW - 24.0f) bubbleW = scrW - 24.0f;

        float bubbleH = std::round(layout.totalHeight + 32.0f);
        float bubbleY = std::round(cursorTopY - bubbleH);
        float bubbleX = std::round(isSelf ? (scrX + scrW - bubbleW - 12.0f) : (scrX + 12.0f));

        // Incoming = Pure White card with soft gray border; Outgoing = Sleek Charcoal Black card
        engine::ui::TextRenderer::drawRoundedRect2D(
            bubbleX, bubbleY, bubbleW, bubbleH, 14.0f,
            isSelf ? 0.14f : 1.00f,
            isSelf ? 0.15f : 1.00f,
            isSelf ? 0.18f : 1.00f,
            0.99f,
            isSelf ? 0.28f : 0.80f,
            isSelf ? 0.30f : 0.82f,
            isSelf ? 0.34f : 0.85f,
            0.95f, 1.2f, false);

        engine::ui::TextRenderer::drawBitmapText2D(
            bubbleX + 13.0f, bubbleY + bubbleH - 16.0f,
            senderTag,
            GLUT_BITMAP_HELVETICA_12,
            isSelf ? 0.76f : 0.38f,
            isSelf ? 0.78f : 0.40f,
            isSelf ? 0.82f : 0.45f);

        engine::ui::TextRenderer::drawWrappedBitmapText2D(
            bubbleX + 13.0f, bubbleY + bubbleH - 33.0f,
            bubbleW - 24.0f, 17.0f,
            msg.text,
            GLUT_BITMAP_HELVETICA_12,
            isSelf ? 0.98f : 0.06f,
            isSelf ? 0.98f : 0.07f,
            isSelf ? 1.00f : 0.09f);

        cursorTopY = bubbleY - 10.0f;
    }

    // 7. Bottom White Input Tray & Gray/Black Reply Buttons
    const float dockH = std::round(choicesAreaH + 22.0f);
    engine::ui::TextRenderer::drawRoundedRect2D(
        scrX + 6.0f, scrY + 16.0f, scrW - 12.0f, dockH - 12.0f, 16.0f,
        1.0f,  1.0f,  1.0f,  0.99f,
        0.82f, 0.84f, 0.87f, 0.95f,
        1.2f, false);

    if (!thread.replied && !thread.replyChoices.empty()) {
        float replyCurY = std::round(scrY + 24.0f);
        for (int i = static_cast<int>(thread.replyChoices.size()) - 1; i >= 0; --i) {
            const std::string& choiceLabel = thread.replyChoices[static_cast<size_t>(i)].label;
            auto cl = engine::ui::TextRenderer::computeWrappedLayout(
                choiceLabel, scrW - 74.0f, 17.0f, GLUT_BITMAP_HELVETICA_12);
            float btnH = std::round(cl.totalHeight + 16.0f);
            if (btnH < 34.0f) btnH = 34.0f;

            // Light-Gray tactile reply button with dark gray border
            engine::ui::TextRenderer::drawRoundedRect2D(
                scrX + 12.0f, replyCurY, scrW - 24.0f, btnH, 10.0f,
                0.93f, 0.94f, 0.96f, 1.0f,
                0.70f, 0.72f, 0.76f, 0.95f,
                1.2f, false);

            // Black Keycap Badge [1] / [2]
            engine::ui::TextRenderer::drawPillBadge2D(
                scrX + 20.0f, std::round(replyCurY + (btnH - 22.0f) * 0.5f), 26.0f, 22.0f,
                std::to_string(i + 1),
                0.12f, 0.13f, 0.16f, 1.0f,
                0.32f, 0.34f, 0.38f,
                1.0f,  1.0f,  1.0f,
                GLUT_BITMAP_HELVETICA_12);

            engine::ui::TextRenderer::drawWrappedBitmapText2D(
                scrX + 52.0f, replyCurY + btnH - 20.0f,
                scrW - 70.0f, 17.0f,
                choiceLabel,
                GLUT_BITMAP_HELVETICA_12,
                0.06f, 0.07f, 0.10f);

            replyCurY += btnH + 8.0f;
        }

        engine::ui::TextRenderer::drawBitmapText2D(
            scrX + 14.0f, replyCurY + 4.0f,
            "Reply: [1] or [2]   |   [TAB] Close",
            GLUT_BITMAP_HELVETICA_12,
            0.30f, 0.32f, 0.36f);
    } else {
        engine::ui::TextRenderer::drawBitmapTextCentered2D(
            scrX + scrW * 0.5f, scrY + 34.0f,
            "Replied  |  Press [N] for Next Chat or [TAB] to Close Phone",
            GLUT_BITMAP_HELVETICA_12,
            0.28f, 0.30f, 0.34f);
    }

    // 8. Bottom Black Home Indicator Bar on Screen
    engine::ui::TextRenderer::drawRoundedRect2D(
        scrX + (scrW - 124.0f) * 0.5f, scrY + 6.0f, 124.0f, 4.5f, 2.2f,
        0.16f, 0.17f, 0.20f, 0.90f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, false);
}

} // namespace narrative
} // namespace game
