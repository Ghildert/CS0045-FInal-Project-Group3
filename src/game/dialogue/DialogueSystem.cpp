#include "game/dialogue/DialogueSystem.h"
#include "engine/ui/TextRenderer.h"
#include <GL/freeglut.h>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace game {
namespace dialogue {

DialogueSystem::DialogueSystem()
    : m_active(false)
    , m_currentLineIndex(0)
    , m_selectedChoiceIndex(0)
    , m_charTimer(0.0f)
    , m_visibleChars(0)
    , m_completedCount(0)
{}

DialogueSystem::~DialogueSystem() {
    destroy();
}

void DialogueSystem::destroy() {
    m_portraitElijah.destroy();
    m_portraitMaya.destroy();
    m_portraitSam.destroy();
}

void DialogueSystem::initPortraitsAndData(const std::string& filePath) {
    engine::assets::Texture2D::ensureDefaultTexturesOnDisk();
    m_portraitElijah.loadFromFile("assets/sprites/portrait_elijah.png", true, false, true);
    m_portraitMaya.loadFromFile("assets/sprites/portrait_maya.png", true, false, true);
    m_portraitSam.loadFromFile("assets/sprites/portrait_sam.png", true, false, true);
    loadFromExternalData(filePath);
}

void DialogueSystem::ensureDefaultStoryFileOnDisk(const std::string& filePath) {
    // Check if existing file already has Phase 6 post_minigame_celebration node
    if (std::filesystem::exists(filePath)) {
        std::ifstream chk(filePath);
        std::string content((std::istreambuf_iterator<char>(chk)), std::istreambuf_iterator<char>());
        if (content.find("post_minigame_celebration") != std::string::npos) {
            return;
        }
    }
    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
    std::ofstream out(filePath);
    out << "# Echoes of November - External Branching Narrative Data (Locations 1-3)\n";
    out << "# Format:\n";
    out << "# NODE|node_id|completion_flag_key|completion_flag_val\n";
    out << "# LINE|speaker|dialogue_text\n";
    out << "# CHOICE|choice_label|target_node_id|set_flag_key|set_flag_val\n\n";

    // =========================================================================
    // LOCATION 1: ROOM 304 (CLASSROOM)
    // =========================================================================
    out << "NODE|maya_classroom_talk|talked_to_maya|1\n";
    out << "LINE|Maya|Still here, Elijah? Everyone else rushed out as soon as the last bell rang.\n";
    out << "LINE|Elijah|I was finishing the perspective study for Prof. Reyes. The sunset light is finally right.\n";
    out << "LINE|Maya|Look at the clouds over the viaduct... Sam texted that the band is meeting outside before the storm hits.\n";
    out << "CHOICE|I'll head out right after I grab my sketchbook.|maya_branch_promise|promised_rooftop_rehearsal|1\n";
    out << "CHOICE|Have you felt like this week is repeating itself?|maya_branch_dejavu|asked_maya_dejavu|1\n\n";

    out << "NODE|maya_branch_promise|talked_to_maya|1\n";
    out << "LINE|Elijah|I promised Sam I'd bring the acoustic arrangement. Just need my sketchbook from the front-left desk.\n";
    out << "LINE|Maya|Awesome! Grab your sketchbook and reply to Sam on your phone [Tab], then we can take the hallway door.\n\n";

    out << "NODE|maya_branch_dejavu|talked_to_maya|1\n";
    out << "LINE|Elijah|Maya... do you ever get the feeling we've had this exact November afternoon conversation before?\n";
    out << "LINE|Maya|You too?! When the bell rang at 4:30, I swore I remembered the rain hitting the glass at that exact angle.\n";
    out << "LINE|Maya|Check your sketchbook on the front-left desk—I think you wrote something about it in the margin.\n\n";

    out << "NODE|inspect_sketchbook|has_sketchbook|1\n";
    out << "LINE|Elijah|My hardcover sketchbook. A loose sheet of 3D perspective studies is tucked inside.\n";
    out << "LINE|Elijah|In the margin, my own handwriting says: 'Nov 14 — Meet Sam in the hallway, then Sundown Mart at 5:30 PM.'\n";
    out << "LINE|System|[Obtained Sketchbook & Chords — Flag 'has_sketchbook' = 1]\n\n";

    out << "NODE|inspect_chalkboard|read_chalkboard|1\n";
    out << "LINE|Chalkboard|CS0045 Visual Computing Finals: Modules 1-8 Practical Showcase — Friday, Nov 15.\n";
    out << "LINE|Elijah|Primitives, Vertex Arrays, VBOs, 4x4 Matrices, Viewing, and Phong Lighting... all set.\n\n";

    out << "NODE|inspect_window|looked_out_window|1\n";
    out << "LINE|Elijah|Warm amber sunlight pours across the wooden floorboards from the west courtyard.\n";
    out << "LINE|Elijah|Dark rainclouds are gathering over the train viaduct in the distance.\n\n";

    out << "NODE|inspect_door_locked||0\n";
    out << "LINE|Elijah|The sliding wooden door to the third-floor school hallway.\n";
    out << "LINE|Elijah|I should grab my Sketchbook from the front-left desk and talk to Maya before heading out.\n\n";

    out << "NODE|inspect_door_unlocked|entered_hallway_transition|1\n";
    out << "LINE|Elijah|I've got my sketchbook and checked in with Maya. Stepping out into the 3rd-Floor School Hallway.\n\n";

    // =========================================================================
    // LOCATION 2: 3RD-FLOOR SCHOOL HALLWAY
    // =========================================================================
    out << "NODE|sam_hallway_talk|talked_to_sam|1\n";
    out << "LINE|Sam|There you are, Elijah! Did you bring the chord progression from Room 304?\n";
    out << "LINE|Elijah|Got it right here in my sketchbook. Why are we packing up by the lockers?\n";
    out << "LINE|Sam|Security locked the rooftop stairs because of the thunderstorm warning! Maya already headed down to Sundown Mart.\n";
    out << "CHOICE|Let's practice under the convenience store awning.|sam_branch_awning|street_unlocked|1\n";
    out << "CHOICE|Want to test the rhythm timing right here first?|sam_branch_rhythm|street_unlocked|1\n\n";

    out << "NODE|sam_branch_awning|street_unlocked|1\n";
    out << "LINE|Elijah|The neon canopy outside Sundown Mart is dry and has benches. Let's meet Maya down on the street.\n";
    out << "LINE|Sam|Good call! Take the stairwell exit on the right end of the corridor whenever you're ready.\n\n";

    out << "NODE|sam_branch_rhythm|street_unlocked|1\n";
    out << "LINE|Sam|Haha, I like your energy! Let's head down to the street exit on the right so Maya doesn't wait in the rain.\n\n";

    out << "NODE|inspect_lockers|inspected_lockers|1\n";
    out << "LINE|Elijah|Row of steel-blue student lockers. Locker 314 has a faded sticker from last year's Cultural Festival.\n\n";

    out << "NODE|inspect_noticeboard|inspected_noticeboard|1\n";
    out << "LINE|Noticeboard|Campus Announcement: 'Annual Acoustic Night — Friday 6:00 PM. All student bands report for soundcheck.'\n\n";

    out << "NODE|inspect_stairs_locked||0\n";
    out << "LINE|Elijah|The stairwell leading down to the street. I should check in with Sam by the lockers first.\n\n";

    // =========================================================================
    // LOCATION 3: RAINY STREET / SUNDOWN MART EXTERIOR
    // =========================================================================
    out << "NODE|maya_street_talk|talked_to_maya_street|1\n";
    out << "LINE|Maya|You made it! Listen to the rain hitting the convenience store awning...\n";
    out << "LINE|Elijah|The neon reflection on the wet asphalt looks just like the sketch I drew this afternoon.\n";
    out << "LINE|Maya|Sam left his acoustic guitar by the bus-stop bench so you could run through the melody!\n";
    out << "CHOICE|Let's play the acoustic rhythm song together! [Start Minigame]|maya_street_ready|ready_for_minigame|1\n";
    out << "CHOICE|Let me grab a warm drink from Sundown Mart first.|maya_street_drink|checked_storefront|1\n\n";

    out << "NODE|maya_street_ready|ready_for_minigame|1\n";
    out << "LINE|Maya|Interact with the Acoustic Guitar Case on the bench (or press [G]) when you're ready to play!\n\n";

    out << "NODE|maya_street_drink|checked_storefront|1\n";
    out << "LINE|Maya|Get me a warm ginger tea too! Then inspect the Guitar Case on the bench when you're ready to play.\n\n";

    out << "NODE|inspect_storefront|checked_storefront|1\n";
    out << "LINE|Elijah|24/7 Sundown Mart. Warm fluorescent light glows through the rain-streaked glass window.\n\n";

    out << "NODE|inspect_bus_sign|checked_bus_sign|1\n";
    out << "LINE|Bus Stop|Route 14 Last Bus: 21:30. Puddles ripple around the base of the cast-iron streetlight.\n\n";

    out << "NODE|post_minigame_celebration|story_act1_complete|1\n";
    out << "LINE|Maya|That sounded incredible against the rain, Elijah! The bridge arpeggio finally clicked.\n";
    out << "LINE|Elijah|Playing it under the Sundown Mart neon felt weirdly familiar... in the best way.\n";
    out << "LINE|Maya|Come on, the rain is easing up. We're ready for Friday's Acoustic Night!\n";

    out.close();
}

void DialogueSystem::loadFromExternalData(const std::string& filePath) {
    ensureDefaultStoryFileOnDisk(filePath);
    m_sequences.clear();

    std::ifstream in(filePath);
    if (!in.is_open()) {
        return;
    }

    std::string line;
    DialogueSequence* current = nullptr;
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

        if (parts[0] == "NODE" && parts.size() >= 2) {
            DialogueSequence seq;
            seq.id                  = parts[1];
            seq.completionFlagKey   = (parts.size() >= 3) ? parts[2] : "";
            seq.completionFlagValue = (parts.size() >= 4 && !parts[3].empty()) ? std::stoi(parts[3]) : 1;
            m_sequences[seq.id]     = seq;
            current                 = &m_sequences[seq.id];
        } else if (parts[0] == "LINE" && parts.size() >= 3 && current) {
            current->lines.push_back(DialogueLine{ parts[1], parts[2] });
        } else if (parts[0] == "CHOICE" && parts.size() >= 5 && current) {
            current->choices.push_back(DialogueChoice{
                parts[1],
                parts[2],
                parts[3],
                std::stoi(parts[4])
            });
        }
    }
}

void DialogueSystem::registerSequence(const DialogueSequence& seq) {
    m_sequences[seq.id] = seq;
}

bool DialogueSystem::startDialogue(const std::string& sequenceId) {
    auto it = m_sequences.find(sequenceId);
    if (it == m_sequences.end() || it->second.lines.empty()) {
        return false;
    }

    m_active              = true;
    m_activeSequenceId    = sequenceId;
    m_currentLineIndex    = 0;
    m_selectedChoiceIndex = 0;
    m_charTimer           = 0.0f;
    m_visibleChars        = 0;
    return true;
}

bool DialogueSystem::isWaitingForChoice() const {
    if (!m_active) return false;
    auto it = m_sequences.find(m_activeSequenceId);
    if (it == m_sequences.end()) return false;
    const auto& seq = it->second;
    if (seq.choices.empty()) return false;
    bool onLastLine = (m_currentLineIndex + 1 == static_cast<int>(seq.lines.size()));
    bool lineRevealed = (m_visibleChars >= seq.lines[m_currentLineIndex].text.size());
    return onLastLine && lineRevealed;
}

void DialogueSystem::moveChoiceCursor(int delta) {
    if (!isWaitingForChoice()) return;
    auto it = m_sequences.find(m_activeSequenceId);
    int n = static_cast<int>(it->second.choices.size());
    if (n <= 0) return;
    m_selectedChoiceIndex = (m_selectedChoiceIndex + delta + n) % n;
}

bool DialogueSystem::selectChoice(int choiceIndex, narrative::StoryState* storyState) {
    if (!isWaitingForChoice()) {
        return false;
    }
    auto it = m_sequences.find(m_activeSequenceId);
    const auto& seq = it->second;
    if (choiceIndex < 0 || choiceIndex >= static_cast<int>(seq.choices.size())) {
        return false;
    }

    // Apply current node completion flag first
    if (storyState && !seq.completionFlagKey.empty()) {
        storyState->setFlag(seq.completionFlagKey, seq.completionFlagValue);
    }

    const auto& choice = seq.choices[choiceIndex];
    if (storyState && !choice.setFlagKey.empty()) {
        storyState->setFlag(choice.setFlagKey, choice.setFlagValue);
    }

    if (!choice.targetNodeId.empty() && m_sequences.count(choice.targetNodeId) > 0) {
        return startDialogue(choice.targetNodeId);
    }

    m_active = false;
    ++m_completedCount;
    return true;
}

void DialogueSystem::advanceOrCompleteLine(narrative::StoryState* storyState) {
    if (!m_active) {
        return;
    }

    auto it = m_sequences.find(m_activeSequenceId);
    if (it == m_sequences.end()) {
        m_active = false;
        return;
    }

    const auto& seq = it->second;
    const std::string& fullText = seq.lines[m_currentLineIndex].text;

    if (m_visibleChars < fullText.size()) {
        m_visibleChars = fullText.size();
        return;
    }

    // If waiting at a branching choice prompt, confirm currently highlighted choice
    if (isWaitingForChoice()) {
        selectChoice(m_selectedChoiceIndex, storyState);
        return;
    }

    ++m_currentLineIndex;
    if (m_currentLineIndex >= static_cast<int>(seq.lines.size())) {
        if (storyState && !seq.completionFlagKey.empty()) {
            storyState->setFlag(seq.completionFlagKey, seq.completionFlagValue);
        }
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
        constexpr float kSecPerChar = 0.016f;
        while (m_charTimer >= kSecPerChar && m_visibleChars < fullText.size()) {
            m_charTimer -= kSecPerChar;
            ++m_visibleChars;
        }
    }
}

const engine::assets::Texture2D* DialogueSystem::getPortraitForSpeaker(const std::string& speaker) const {
    if (speaker == "Elijah" && m_portraitElijah.isValid()) {
        return &m_portraitElijah;
    }
    if (speaker == "Maya" && m_portraitMaya.isValid()) {
        return &m_portraitMaya;
    }
    if (speaker == "Sam" && m_portraitSam.isValid()) {
        return &m_portraitSam;
    }
    return nullptr;
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

    const auto& seq   = it->second;
    const auto& lines = seq.lines;
    const DialogueLine& line = lines[m_currentLineIndex];
    std::string shownText = line.text.substr(0, m_visibleChars);

    float boxW = static_cast<float>(screenWidth) - 160.0f;
    if (boxW > 980.0f) boxW = 980.0f;
    float boxH = 136.0f;
    float boxX = (static_cast<float>(screenWidth) - boxW) * 0.5f;
    float boxY = 22.0f;

    // Main dialogue container panel
    engine::ui::TextRenderer::drawPanel2D(
        boxX, boxY, boxW, boxH,
        0.05f, 0.06f, 0.11f, 0.94f,
        0.95f, 0.76f, 0.38f);

    // Character Portrait on the left inside the dialogue box
    const auto* portrait = getPortraitForSpeaker(line.speaker);
    float textOffsetX = boxX + 28.0f;
    if (portrait) {
        float portSize = 104.0f;
        engine::ui::TextRenderer::drawTexturedQuad2D(
            *portrait,
            boxX + 18.0f, boxY + 16.0f,
            portSize, portSize);
        textOffsetX = boxX + 140.0f;
    }

    // Speaker Nameplate tab above top-left corner of dialogue box
    engine::ui::TextRenderer::drawPanel2D(
        boxX + 18.0f, boxY + boxH - 4.0f, 185.0f, 32.0f,
        0.16f, 0.12f, 0.22f, 0.96f,
        0.98f, 0.82f, 0.42f);

    engine::ui::TextRenderer::drawBitmapText2D(
        boxX + 34.0f, boxY + boxH + 6.0f,
        line.speaker,
        GLUT_BITMAP_HELVETICA_18,
        1.0f, 0.88f, 0.48f);

    // Spoken dialogue line
    engine::ui::TextRenderer::drawBitmapText2D(
        textOffsetX, boxY + 82.0f,
        shownText,
        GLUT_BITMAP_HELVETICA_18,
        0.95f, 0.96f, 0.98f);

    // If waiting for branching player choice, render choice buttons above the dialogue box
    if (isWaitingForChoice()) {
        for (size_t i = 0; i < seq.choices.size(); ++i) {
            bool selected = (static_cast<int>(i) == m_selectedChoiceIndex);
            float choiceW = boxW - 140.0f;
            float choiceH = 34.0f;
            float choiceX = textOffsetX;
            float choiceY = boxY + 42.0f - static_cast<float>(i) * 36.0f;

            engine::ui::TextRenderer::drawPanel2D(
                choiceX, choiceY, choiceW, choiceH,
                selected ? 0.22f : 0.10f,
                selected ? 0.18f : 0.12f,
                selected ? 0.32f : 0.20f,
                0.96f,
                selected ? 1.0f  : 0.55f,
                selected ? 0.84f : 0.65f,
                selected ? 0.35f : 0.78f);

            std::string prefix = selected ? "> [" : "  [";
            engine::ui::TextRenderer::drawBitmapText2D(
                choiceX + 14.0f, choiceY + 10.0f,
                prefix + std::to_string(i + 1) + "] " + seq.choices[i].label,
                GLUT_BITMAP_HELVETICA_12,
                selected ? 1.0f : 0.85f,
                selected ? 0.92f : 0.88f,
                selected ? 0.52f : 0.92f);
        }
    } else {
        std::string progressLabel =
            "Line " + std::to_string(m_currentLineIndex + 1) + "/" + std::to_string(lines.size()) +
            "  |  Press [E] / [Space] / [Left-Click] to continue";
        engine::ui::TextRenderer::drawBitmapText2D(
            boxX + boxW - 415.0f, boxY + 16.0f,
            progressLabel,
            GLUT_BITMAP_HELVETICA_12,
            0.75f, 0.84f, 0.92f);
    }
}

} // namespace dialogue
} // namespace game
