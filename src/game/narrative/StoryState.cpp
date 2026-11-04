#include "game/narrative/StoryState.h"
#include "engine/core/Logger.h"
#include "engine/audio/AudioSystem.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace game {
namespace narrative {

StoryState::StoryState()
    : m_toastTimer(0.0f)
{
    resetNewGame();
}

void StoryState::resetNewGame() {
    m_flags.clear();
    m_flags["has_sketchbook"]             = 0;
    m_flags["talked_to_maya"]             = 0;
    m_flags["promised_rooftop_rehearsal"] = 0;
    m_flags["replied_phone_sam"]          = 0;
    m_flags["replied_phone_maya"]         = 0;
    m_flags["hallway_unlocked"]           = 0;
}

void StoryState::setFlag(const std::string& key, int value) {
    if (key.empty()) {
        return;
    }
    m_flags[key] = value;

    // Automatically unlock the hallway exit once Elijah has his sketchbook and talked to Maya
    if (getFlag("has_sketchbook") > 0 && getFlag("talked_to_maya") > 0) {
        m_flags["hallway_unlocked"] = 1;
    }
}

int StoryState::getFlag(const std::string& key) const {
    auto it = m_flags.find(key);
    return (it != m_flags.end()) ? it->second : 0;
}

bool StoryState::hasFlag(const std::string& key) const {
    return getFlag(key) > 0;
}

std::string StoryState::getCurrentObjectiveText() const {
    if (!hasFlag("has_sketchbook")) {
        return "Objective: Inspect your Sketchbook on the front-left desk.";
    }
    if (!hasFlag("talked_to_maya")) {
        return "Objective: Talk to Maya by the sunset window about the rooftop rehearsal.";
    }
    if (!hasFlag("replied_phone_sam")) {
        return "Objective: Open your Smartphone [Tab] and reply to Sam's band message.";
    }
    return "Objective: Head to the Hallway Door on the right wall (or press [F5] to Save).";
}

void StoryState::showToast(const std::string& message, float durationSeconds) {
    m_toastMessage = message;
    m_toastTimer   = durationSeconds;
}

void StoryState::update(float dt) {
    if (m_toastTimer > 0.0f) {
        m_toastTimer -= dt;
        if (m_toastTimer <= 0.0f) {
            m_toastTimer = 0.0f;
            m_toastMessage.clear();
        }
    }
}

bool StoryState::saveToFile(
    const std::string& filePath,
    const std::string& currentSceneId,
    float playerX,
    float playerZ)
{
    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
    std::ofstream out(filePath);
    if (!out.is_open()) {
        engine::core::Logger::error("Failed to write save file: " + filePath);
        return false;
    }

    out << "# Echoes of November Save File\n";
    out << "scene=" << currentSceneId << "\n";
    out << "player_x=" << playerX << "\n";
    out << "player_z=" << playerZ << "\n";
    for (const auto& kv : m_flags) {
        out << "flag." << kv.first << "=" << kv.second << "\n";
    }
    out.close();

    showToast("Game Saved to " + filePath, 3.5f);
    engine::audio::AudioSystem::play(engine::audio::SoundEvent::SaveChime);
    engine::core::Logger::info("Saved game state to " + filePath);
    return true;
}

bool StoryState::loadFromFile(
    const std::string& filePath,
    std::string& outSceneId,
    float& outPlayerX,
    float& outPlayerZ)
{
    std::ifstream in(filePath);
    if (!in.is_open()) {
        showToast("No save file found at " + filePath, 3.0f);
        return false;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        if (key == "scene") {
            outSceneId = val;
        } else if (key == "player_x") {
            outPlayerX = std::stof(val);
        } else if (key == "player_z") {
            outPlayerZ = std::stof(val);
        } else if (key.rfind("flag.", 0) == 0) {
            std::string flagName = key.substr(5);
            m_flags[flagName] = std::stoi(val);
        }
    }

    showToast("Game Loaded from " + filePath, 3.5f);
    engine::core::Logger::info("Loaded game state from " + filePath);
    return true;
}

} // namespace narrative
} // namespace game
