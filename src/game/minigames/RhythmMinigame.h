#ifndef GAME_MINIGAMES_RHYTHMMINIGAME_H
#define GAME_MINIGAMES_RHYTHMMINIGAME_H

#include "game/narrative/StoryState.h"
#include <string>
#include <vector>

namespace game {
namespace minigames {

struct RhythmNote {
    int   lane;      // 0..3 corresponding to [D/1], [F/2], [J/3], [K/4]
    float hitTime;   // Time in seconds when note crosses the hit line
    bool  resolved;  // True once hit or missed
    bool  wasHit;    // True if player hit it successfully
};

struct HitBurstEffect {
    int   lane;
    float timer;     // Counts down from 0.35s to 0.0f
    float r, g, b;
};

class RhythmMinigame {
public:
    RhythmMinigame();

    void startSession();
    void stopSession();

    void update(float dt, narrative::StoryState& storyState);
    void onKeyPress(unsigned char key, narrative::StoryState& storyState);

    // Automated self-test helper for Phase 6 verification
    void autoPlayStep(float dt, narrative::StoryState& storyState);

    void render2D(int screenWidth, int screenHeight) const;

    bool isActive() const { return m_active; }
    bool isFinished() const { return m_finished; }
    int  getScore() const { return m_score; }
    int  getCombo() const { return m_combo; }
    int  getMaxCombo() const { return m_maxCombo; }
    int  getNotesHit() const { return m_notesHit; }
    int  getTotalNotes() const { return static_cast<int>(m_notes.size()); }
    const std::string& getLastJudgment() const { return m_lastJudgment; }

private:
    void buildAcousticSongChart();
    void triggerLaneInput(int lane, narrative::StoryState& storyState);

    bool                        m_active;
    bool                        m_finished;
    float                       m_songTime;
    float                       m_songDuration;
    int                         m_score;
    int                         m_combo;
    int                         m_maxCombo;
    int                         m_notesHit;
    int                         m_notesMissed;
    std::string                 m_lastJudgment;
    float                       m_judgmentTimer;
    float                       m_laneFlashTimer[4];
    std::vector<RhythmNote>     m_notes;
    std::vector<HitBurstEffect> m_bursts;
};

} // namespace minigames
} // namespace game

#endif // GAME_MINIGAMES_RHYTHMMINIGAME_H
