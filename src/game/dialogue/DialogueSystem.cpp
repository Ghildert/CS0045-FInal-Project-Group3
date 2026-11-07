#include "game/dialogue/DialogueSystem.h"
#include "engine/ui/TextRenderer.h"
#include "engine/audio/AudioSystem.h"
#include <GL/freeglut.h>
#include <cctype>
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
    , m_blipCooldown(0.0f)
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
    out << "LINE|Maya|Check your sketchbook on the front-left desk - I think you wrote something about it in the margin.\n\n";

    out << "NODE|inspect_sketchbook|has_sketchbook|1\n";
    out << "LINE|Elijah|My hardcover sketchbook. A loose sheet of 3D perspective studies is tucked inside.\n";
    out << "LINE|Elijah|In the margin, my own handwriting says: 'Nov 14 - Meet Sam in the hallway, then Sundown Mart at 5:30 PM.'\n";
    out << "LINE|System|[Obtained Sketchbook & Chords - Flag 'has_sketchbook' = 1]\n\n";

    out << "NODE|inspect_sketchbook_taken|inspected_desk_again|1\n";
    out << "LINE|Elijah|My front-row desk. I already tucked my sketchbook and acoustic chord chart into my bag.\n";
    out << "LINE|Elijah|Faint pencil smudges from this afternoon's vanishing-point exercise are still on the wood grain.\n\n";

    out << "NODE|inspect_teacher_desk|inspected_teacher_desk|1\n";
    out << "LINE|Elijah|Prof. Reyes's front lectern desk. A ceramic Utah teapot and a glossy blue reference sphere sit on top.\n";
    out << "LINE|Elijah|He always brings those two props to demonstrate Specular Phong highlights vs. matte wood.\n\n";

    out << "NODE|inspect_pyramid_desk|inspected_pyramid_desk|1\n";
    out << "LINE|Elijah|A golden 3D pyramid model gently bobbing on the center desk for our homogeneous matrix lab.\n";
    out << "LINE|Elijah|Watching the sunset catch its facets makes the whole room feel calm.\n\n";

    out << "NODE|inspect_back_desks|inspected_back_desks|1\n";
    out << "LINE|Elijah|A row of scuffed wooden student desks. Someone carved 'BAND CLUB FOREVER' tiny into the corner.\n\n";

    out << "NODE|inspect_chalkboard|read_chalkboard|1\n";
    out << "LINE|Chalkboard|CS0045 Visual Computing Finals: Modules 1-8 Practical Showcase - Friday, Nov 15.\n";
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

    out << "NODE|inspect_lockers_east|inspected_lockers_east|1\n";
    out << "LINE|Elijah|The east bank of lockers near the stairwell. Sam keeps spare guitar strings and capos in Locker 322.\n\n";

    out << "NODE|inspect_noticeboard|inspected_noticeboard|1\n";
    out << "LINE|Noticeboard|Campus Announcement: 'Annual Acoustic Night - Friday 6:00 PM. All student bands report for soundcheck.'\n\n";

    out << "NODE|inspect_hallway_bench|inspected_hallway_bench|1\n";
    out << "LINE|Elijah|A sturdy wooden corridor bench outside Room 304. We always sit here between afternoon lectures.\n\n";

    out << "NODE|inspect_stairs_locked||0\n";
    out << "LINE|Elijah|The stairwell leading down to the street. I should check in with Sam by the lockers first.\n\n";

    // =========================================================================
    // LOCATION 3: P. PAREDES ST. - FEU INSTITUTE OF TECHNOLOGY & CANTEENS
    // =========================================================================
    out << "NODE|maya_street_talk|talked_to_maya_street|1\n";
    out << "LINE|Maya|You made it down to P. Paredes Street! Everyone is grabbing snacks by the canteens after class.\n";
    out << "LINE|Elijah|Seeing the FEU Tech facade and the food stalls next door feels just like the sketch in my notebook.\n";
    out << "LINE|Maya|Sam left his acoustic guitar by the sidewalk bench so you could run through the melody!\n";
    out << "CHOICE|Let's play the acoustic rhythm song right here on P. Paredes! [Start Minigame]|maya_street_ready|ready_for_minigame|1\n";
    out << "CHOICE|Let me check out Mang Rody's Eatery and Byte & Snacks first.|maya_street_drink|checked_storefront|1\n\n";

    out << "NODE|maya_street_ready|ready_for_minigame|1\n";
    out << "LINE|Maya|Interact with Sam's Acoustic Guitar by the sidewalk stools (or press [G]) when you're ready to play!\n\n";

    out << "NODE|maya_street_drink|checked_storefront|1\n";
    out << "LINE|Maya|Grab some cheese fries at Byte & Snacks for me too! Then inspect the Guitar Case when you're ready to play.\n\n";

    out << "NODE|inspect_feu_facade|checked_feu_facade|1\n";
    out << "LINE|Elijah|937 P. Paredes St - FEU Institute of Technology. The green and gold Autonomous Status banner gleams above the hedges.\n";
    out << "LINE|Elijah|Students are heading up and down the twin entrance stairways between late-afternoon lab sessions.\n\n";

    out << "NODE|inspect_silver_shutter|checked_silver_shutter|1\n";
    out << "LINE|Elijah|The east entrance stairway under the FEU Institute of Technology fascia. Both stair bays lead up into the main campus lobby.\n\n";

    out << "NODE|inspect_storefront|checked_storefront|1\n";
    out << "LINE|Elijah|The sidewalk food stall right beside FEU Tech, with white monobloc stools, blue gulaman dispensers, and steamed siomai trays.\n";
    out << "LINE|Elijah|Above the corrugated canopy is a big 'Frost & Brew - Iced Coffee & Milk Tea (Buy 1 Take 1 P49)' promotion billboard.\n\n";

    out << "NODE|inspect_raloys_canteen|checked_raloys_canteen|1\n";
    out << "LINE|Elijah|Mang Rody's Eatery and Byte & Snacks! The bright red sign and green turf counters are always packed after CS0045 class.\n\n";

    out << "NODE|inspect_bus_sign|checked_bus_sign|1\n";
    out << "LINE|Elijah|Stacks of yellow-and-orange plastic delivery crates outside the Tater Cube fries kiosk and Byte & Snacks.\n\n";

    out << "NODE|inspect_room_general||0\n";
    out << "LINE|Elijah|Let's look around closely. Walk up to any glowing amber marker and press [E] to interact.\n\n";

    out << "NODE|post_minigame_celebration|story_act1_complete|1\n";
    out << "LINE|Maya|That sounded incredible right outside FEU Tech, Elijah! Even the crowd by Byte & Snacks stopped to listen.\n";
    out << "LINE|Elijah|Playing it out here on P. Paredes Street made the whole song come together.\n";
    out << "LINE|Maya|Come on, let's celebrate with snacks at Mang Rody's Eatery. We're ready for Friday's Acoustic Night!\n";

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
    m_blipCooldown        = 0.0f;
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
    engine::audio::AudioSystem::play(engine::audio::SoundEvent::UIBlip);
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

    engine::audio::AudioSystem::play(engine::audio::SoundEvent::DialogueSelect);

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
        engine::audio::AudioSystem::playVoiceBlip(seq.lines[m_currentLineIndex].speaker, m_visibleChars);
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
        m_blipCooldown = 0.0f;
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

    if (m_blipCooldown > 0.0f) {
        m_blipCooldown -= dt;
        if (m_blipCooldown < 0.0f) m_blipCooldown = 0.0f;
    }

    const DialogueLine& line = it->second.lines[m_currentLineIndex];
    const std::string& fullText = line.text;
    if (m_visibleChars < fullText.size()) {
        m_charTimer += dt;
        constexpr float kSecPerChar = 0.020f;
        bool playedBlipThisStep = false;
        while (m_charTimer >= kSecPerChar && m_visibleChars < fullText.size()) {
            m_charTimer -= kSecPerChar;
            char ch = fullText[m_visibleChars];
            bool isWordStart = (m_visibleChars == 0) ||
                               (fullText[m_visibleChars - 1] == ' ') ||
                               (fullText[m_visibleChars - 1] == '\n');
            ++m_visibleChars;

            // Play pleasant MP3 text sound per word as dialogue text reveals
            if (!playedBlipThisStep && isWordStart && m_blipCooldown <= 0.0f &&
                std::isalnum(static_cast<unsigned char>(ch))) {
                engine::audio::AudioSystem::playVoiceBlip(line.speaker, m_visibleChars);
                m_blipCooldown = 0.085f;
                playedBlipThisStep = true;
            }
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

    const auto* portrait = getPortraitForSpeaker(line.speaker);
    float portSpace = portrait ? 136.0f : 28.0f;

    // Dynamically measure the full line layout so the dialogue container wraps snugly to its text
    float maxAllowedBoxW = static_cast<float>(screenWidth) - 80.0f;
    if (maxAllowedBoxW > 900.0f) maxAllowedBoxW = 900.0f;
    float maxTextW = maxAllowedBoxW - portSpace - 32.0f;
    if (maxTextW < 260.0f) maxTextW = 260.0f;

    auto fullLayout = engine::ui::TextRenderer::computeWrappedLayout(
        line.text, maxTextW, 25.0f, GLUT_BITMAP_HELVETICA_18);

    std::string progressLabel = isWaitingForChoice()
        ? "Choose Response: [Up/Down] + [Enter/E] or Press [1] / [2]"
        : ("Line " + std::to_string(m_currentLineIndex + 1) + "/" + std::to_string(lines.size()) +
           "   |   [E / Space / Left-Click] Continue");
    float footerW = static_cast<float>(
        engine::ui::TextRenderer::measureBitmapTextWidth(progressLabel, GLUT_BITMAP_HELVETICA_12));

    float contentW = (fullLayout.maxLineWidth > footerW) ? fullLayout.maxLineWidth : footerW;
    float boxW = portSpace + contentW + 40.0f;
    if (boxW < 520.0f) boxW = 520.0f;
    if (boxW > maxAllowedBoxW) boxW = maxAllowedBoxW;

    float neededTextH = fullLayout.totalHeight + 66.0f;
    float minBoxH = portrait ? 136.0f : 112.0f;
    float boxH = (neededTextH > minBoxH) ? neededTextH : minBoxH;

    float boxX = (static_cast<float>(screenWidth) - boxW) * 0.5f;
    float boxY = 18.0f;

    // Main HTML-styled dialogue container card (border-radius: 16px, sleek slate border)
    engine::ui::TextRenderer::drawRoundedRect2D(
        boxX, boxY, boxW, boxH, 16.0f,
        0.05f, 0.08f, 0.15f, 0.95f,
        0.46f, 0.56f, 0.72f, 0.52f,
        1.4f, true);

    // Character Portrait on the left inside the dialogue box with sleek slate rounded frame (no gold outline)
    float textOffsetX = boxX + 28.0f;
    if (portrait) {
        float portSize = 100.0f;
        float portY = boxY + (boxH - portSize) * 0.5f;
        engine::ui::TextRenderer::drawRoundedRect2D(
            boxX + 16.0f, portY - 2.0f, portSize + 4.0f, portSize + 4.0f, 8.0f,
            0.09f, 0.13f, 0.22f, 0.95f,
            0.40f, 0.52f, 0.70f, 0.52f,
            1.2f, false);
        engine::ui::TextRenderer::drawTexturedQuad2D(
            *portrait,
            boxX + 18.0f, portY,
            portSize, portSize);
        textOffsetX = boxX + 136.0f;
    }

    // Speaker Nameplate Pill Badge dynamically sized to speaker name (sleek slate-blue outline, no gold)
    int speakerW = engine::ui::TextRenderer::measureBitmapTextWidth(line.speaker, GLUT_BITMAP_HELVETICA_18);
    float tabW = static_cast<float>(speakerW) + 42.0f;
    if (tabW < 120.0f) tabW = 120.0f;
    engine::ui::TextRenderer::drawPillBadge2D(
        boxX + 20.0f, boxY + boxH - 14.0f, tabW, 30.0f,
        line.speaker,
        0.11f, 0.17f, 0.30f, 0.98f,
        0.44f, 0.58f, 0.78f,
        0.94f, 0.97f, 1.0f,
        GLUT_BITMAP_HELVETICA_18);

    // Spoken dialogue line with automatic word-wrapping inside the dynamically sized box
    float wrapW = (boxX + boxW - 24.0f) - textOffsetX;
    engine::ui::TextRenderer::drawWrappedBitmapText2D(
        textOffsetX, boxY + boxH - 42.0f,
        wrapW, 25.0f,
        shownText,
        GLUT_BITMAP_HELVETICA_18,
        0.95f, 0.97f, 1.0f);

    // If waiting for branching player choice, render choice buttons dynamically wrapped above dialogue box
    if (isWaitingForChoice()) {
        float choiceX = textOffsetX;
        float maxChoiceW = (boxX + boxW) - choiceX;
        int numChoices = static_cast<int>(seq.choices.size());
        float cursorChoiceY = boxY + boxH + 14.0f;

        for (int i = numChoices - 1; i >= 0; --i) {
            bool selected = (i == m_selectedChoiceIndex);
            const std::string& lbl = seq.choices[static_cast<size_t>(i)].label;
            auto choiceLayout = engine::ui::TextRenderer::computeWrappedLayout(
                lbl, maxChoiceW - 64.0f, 18.0f, GLUT_BITMAP_HELVETICA_12);

            float choiceW = choiceLayout.maxLineWidth + 68.0f;
            if (choiceW < 340.0f) choiceW = 340.0f;
            if (choiceW > maxChoiceW) choiceW = maxChoiceW;
            float choiceH = choiceLayout.totalHeight + 18.0f;
            if (choiceH < 36.0f) choiceH = 36.0f;

            engine::ui::TextRenderer::drawRoundedRect2D(
                choiceX, cursorChoiceY, choiceW, choiceH, 10.0f,
                selected ? 0.13f : 0.07f,
                selected ? 0.22f : 0.10f,
                selected ? 0.38f : 0.19f,
                0.96f,
                selected ? 0.48f : 0.38f,
                selected ? 0.74f : 0.48f,
                selected ? 0.98f : 0.64f,
                selected ? 0.88f : 0.42f,
                selected ? 1.6f  : 1.1f,
                true);

            engine::ui::TextRenderer::drawPillBadge2D(
                choiceX + 10.0f, cursorChoiceY + (choiceH - 22.0f) * 0.5f, 26.0f, 22.0f,
                std::to_string(i + 1),
                selected ? 0.22f : 0.12f,
                selected ? 0.46f : 0.17f,
                selected ? 0.78f : 0.28f,
                0.98f,
                selected ? 0.54f : 0.42f,
                selected ? 0.78f : 0.52f,
                selected ? 0.98f : 0.66f,
                selected ? 0.96f : 0.80f,
                selected ? 0.98f : 0.86f,
                selected ? 1.0f  : 0.94f,
                GLUT_BITMAP_HELVETICA_12);

            engine::ui::TextRenderer::drawWrappedBitmapText2D(
                choiceX + 46.0f, cursorChoiceY + choiceH - 22.0f,
                choiceW - 58.0f, 18.0f,
                lbl,
                GLUT_BITMAP_HELVETICA_12,
                selected ? 0.96f : 0.86f,
                selected ? 0.98f : 0.90f,
                selected ? 1.0f  : 0.96f);

            cursorChoiceY += choiceH + 8.0f;
        }
    }

    engine::ui::TextRenderer::drawBitmapText2D(
        boxX + boxW - footerW - 22.0f, boxY + 14.0f,
        progressLabel,
        GLUT_BITMAP_HELVETICA_12,
        isWaitingForChoice() ? 0.72f : 0.75f,
        isWaitingForChoice() ? 0.88f : 0.85f,
        isWaitingForChoice() ? 1.0f  : 0.94f);
}

} // namespace dialogue
} // namespace game
