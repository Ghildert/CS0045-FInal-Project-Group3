#include "game/dialogue/DialogueSystem.h"
#include "engine/ui/TextRenderer.h"
#include <GL/freeglut.h>

namespace game {
namespace dialogue {

DialogueSystem::DialogueSystem()
    : m_active(false)
    , m_currentLineIndex(0)
    , m_charTimer(0.0f)
    , m_visibleChars(0)
    , m_completedCount(0)
{}

void DialogueSystem::registerSequence(const DialogueSequence& seq) {
    m_sequences[seq.id] = seq;
}

void DialogueSystem::loadDefaultClassroomDialogues() {
    m_sequences.clear();

    // 1. NPC Classmate Maya (standing by the late-afternoon classroom window)
    registerSequence(DialogueSequence{
        "maya_classroom_talk",
        {
            { "Maya",   "Still here, Elijah? Everyone else rushed out as soon as the last bell rang." },
            { "Elijah", "I was finishing the perspective study for Prof. Reyes. The light is finally right." },
            { "Maya",   "Look at the sky outside... It feels like every November evening blurs together lately." },
            { "Maya",   "Oh! Did you check the sketchbook on the front-left desk? I think you left your charcoal notes in it." },
            { "Elijah", "Right, thanks Maya. I'll grab it before they lock the hallway." }
        }
    });

    // 2. Interactive Object: Forgotten Sketchbook on Front-Left Student Desk
    registerSequence(DialogueSequence{
        "inspect_sketchbook",
        {
            { "Elijah", "My hardcover sketchbook. A loose page of 3D perspective studies is tucked inside." },
            { "Elijah", "In the margin, someone wrote: 'Don't forget the rooftop rehearsal after 5:00 PM.'" },
            { "System", "[Obtained Note: Rooftop Rehearsal Reminder]" }
        }
    });

    // 3. Interactive Object: Front Chalkboard Exam Schedule
    registerSequence(DialogueSequence{
        "inspect_chalkboard",
        {
            { "Chalkboard", "CS0045 Visual Computing Finals: Modules 1-8 Practical Showcase — Friday, Nov 15." },
            { "Elijah",     "Primitives, Vertex Arrays, VBOs, 4x4 Matrices, Viewing, and Phong Lighting... almost ready." }
        }
    });

    // 4. Interactive Object: Sunset Window on Left Wall
    registerSequence(DialogueSequence{
        "inspect_window",
        {
            { "Elijah", "Warm amber sunlight pours across the wooden floorboards from the west courtyard." },
            { "Elijah", "Dark rainclouds are gathering over the train viaduct in the distance." }
        }
    });

    // 5. Interactive Object: Classroom Exit Door on Right Wall
    registerSequence(DialogueSequence{
        "inspect_door",
        {
            { "Elijah", "The sliding wooden door to the third-floor school hallway." },
            { "Elijah", "I should check in with Maya and grab my sketchbook notes before heading out." }
        }
    });
}

bool DialogueSystem::startDialogue(const std::string& sequenceId) {
    auto it = m_sequences.find(sequenceId);
    if (it == m_sequences.end() || it->second.lines.empty()) {
        return false;
    }

    m_active           = true;
    m_activeSequenceId = sequenceId;
    m_currentLineIndex = 0;
    m_charTimer        = 0.0f;
    m_visibleChars     = 0;
    return true;
}

void DialogueSystem::advanceOrCompleteLine() {
    if (!m_active) {
        return;
    }

    auto it = m_sequences.find(m_activeSequenceId);
    if (it == m_sequences.end()) {
        m_active = false;
        return;
    }

    const auto& lines = it->second.lines;
    const std::string& fullText = lines[m_currentLineIndex].text;

    // If typewriter effect is still revealing characters, reveal full line immediately
    if (m_visibleChars < fullText.size()) {
        m_visibleChars = fullText.size();
        return;
    }

    // Otherwise advance to next line, or end sequence
    ++m_currentLineIndex;
    if (m_currentLineIndex >= static_cast<int>(lines.size())) {
        m_active = false;
        ++m_completedCount;
    } else {
        m_visibleChars = 0;
        m_charTimer    = 0.0f;
    }
}

void DialogueSystem::update(float dt) {
    if (!m_active) {
        return;
    }

    auto it = m_sequences.find(m_activeSequenceId);
    if (it == m_sequences.end()) {
        m_active = false;
        return;
    }

    const std::string& fullText = it->second.lines[m_currentLineIndex].text;
    if (m_visibleChars < fullText.size()) {
        m_charTimer += dt;
        constexpr float kSecPerChar = 0.018f; // Crisp typewriter speed (~55 chars/sec)
        while (m_charTimer >= kSecPerChar && m_visibleChars < fullText.size()) {
            m_charTimer -= kSecPerChar;
            ++m_visibleChars;
        }
    }
}

void DialogueSystem::render2D(int screenWidth, int screenHeight) const {
    if (!m_active) {
        return;
    }
    (void)screenHeight;

    auto it = m_sequences.find(m_activeSequenceId);
    if (it == m_sequences.end()) {
        return;
    }

    const auto& lines = it->second.lines;
    const DialogueLine& line = lines[m_currentLineIndex];
    std::string shownText = line.text.substr(0, m_visibleChars);

    float boxW = static_cast<float>(screenWidth) - 180.0f;
    if (boxW > 960.0f) boxW = 960.0f;
    float boxH = 128.0f;
    float boxX = (static_cast<float>(screenWidth) - boxW) * 0.5f;
    float boxY = 26.0f;

    // Main dialogue container panel
    engine::ui::TextRenderer::drawPanel2D(
        boxX, boxY, boxW, boxH,
        0.05f, 0.06f, 0.11f, 0.94f,
        0.95f, 0.76f, 0.38f);

    // Speaker Nameplate tab above top-left corner of dialogue box
    engine::ui::TextRenderer::drawPanel2D(
        boxX + 18.0f, boxY + boxH - 4.0f, 175.0f, 32.0f,
        0.16f, 0.12f, 0.22f, 0.96f,
        0.98f, 0.82f, 0.42f);

    engine::ui::TextRenderer::drawBitmapText2D(
        boxX + 34.0f, boxY + boxH + 6.0f,
        line.speaker,
        GLUT_BITMAP_HELVETICA_18,
        1.0f, 0.88f, 0.48f);

    // Spoken dialogue line (Module 3 glutBitmapString)
    engine::ui::TextRenderer::drawBitmapText2D(
        boxX + 28.0f, boxY + 74.0f,
        shownText,
        GLUT_BITMAP_HELVETICA_18,
        0.95f, 0.96f, 0.98f);

    // Footer prompt indicator
    std::string progressLabel =
        "Line " + std::to_string(m_currentLineIndex + 1) + "/" + std::to_string(lines.size()) +
        "  |  Press [E] / [Space] / [Left-Click] to continue";
    engine::ui::TextRenderer::drawBitmapText2D(
        boxX + boxW - 415.0f, boxY + 18.0f,
        progressLabel,
        GLUT_BITMAP_HELVETICA_12,
        0.75f, 0.84f, 0.92f);
}

} // namespace dialogue
} // namespace game
