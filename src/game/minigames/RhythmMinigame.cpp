#include "game/minigames/RhythmMinigame.h"
#include "engine/renderer/PrimitiveRenderer.h"
#include "engine/ui/TextRenderer.h"
#include "engine/audio/AudioSystem.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <sstream>

namespace game {
namespace minigames {

RhythmMinigame::RhythmMinigame()
    : m_active(false)
    , m_finished(false)
    , m_songTime(0.0f)
    , m_songDuration(9.5f)
    , m_score(0)
    , m_combo(0)
    , m_maxCombo(0)
    , m_notesHit(0)
    , m_notesMissed(0)
    , m_lastJudgment("READY")
    , m_judgmentTimer(0.0f)
{
    for (int i = 0; i < 4; ++i) {
        m_laneFlashTimer[i] = 0.0f;
    }
}

void RhythmMinigame::buildAcousticSongChart() {
    m_notes.clear();
    m_bursts.clear();

    // 16-note melodic arpeggio chart ("Echoes of November" Acoustic Motif)
    const int   lanes[16] = { 0, 1, 2, 3,  2, 1, 0, 2,  1, 3, 2, 0,  1, 2, 3, 2 };
    const float times[16] = {
        1.0f, 1.5f, 2.0f, 2.5f,
        3.1f, 3.6f, 4.1f, 4.6f,
        5.2f, 5.7f, 6.2f, 6.7f,
        7.3f, 7.8f, 8.3f, 8.8f
    };

    for (int i = 0; i < 16; ++i) {
        m_notes.push_back(RhythmNote{ lanes[i], times[i], false, false });
    }
    m_songDuration = 9.4f;
}

void RhythmMinigame::startSession() {
    buildAcousticSongChart();
    m_active        = true;
    m_finished      = false;
    m_songTime      = 0.0f;
    m_score         = 0;
    m_combo         = 0;
    m_maxCombo      = 0;
    m_notesHit      = 0;
    m_notesMissed   = 0;
    m_lastJudgment  = "STRUM ON THE BEAT!";
    m_judgmentTimer = 1.2f;
    for (int i = 0; i < 4; ++i) {
        m_laneFlashTimer[i] = 0.0f;
    }
}

void RhythmMinigame::stopSession() {
    m_active = false;
}

void RhythmMinigame::triggerLaneInput(int lane, narrative::StoryState& storyState) {
    (void)storyState;
    if (lane < 0 || lane >= 4 || m_finished) {
        return;
    }
    m_laneFlashTimer[lane] = 0.22f;

    // Find closest unresolved note in this lane within hit window
    constexpr float kPerfectWindow = 0.14f;
    constexpr float kGoodWindow    = 0.30f;

    int   bestIdx  = -1;
    float bestDiff = 1e9f;

    for (size_t i = 0; i < m_notes.size(); ++i) {
        if (m_notes[i].resolved || m_notes[i].lane != lane) {
            continue;
        }
        float diff = std::fabs(m_notes[i].hitTime - m_songTime);
        if (diff <= kGoodWindow && diff < bestDiff) {
            bestDiff = diff;
            bestIdx  = static_cast<int>(i);
        }
    }

    if (bestIdx >= 0) {
        m_notes[bestIdx].resolved = true;
        m_notes[bestIdx].wasHit   = true;
        ++m_notesHit;
        ++m_combo;
        if (m_combo > m_maxCombo) {
            m_maxCombo = m_combo;
        }

        if (bestDiff <= kPerfectWindow) {
            m_score        += 150 + m_combo * 15;
            m_lastJudgment  = "PERFECT! (+150)";
            m_bursts.push_back(HitBurstEffect{ lane, 0.38f, 1.0f, 0.88f, 0.32f });
        } else {
            m_score        += 100 + m_combo * 10;
            m_lastJudgment  = "GREAT! (+100)";
            m_bursts.push_back(HitBurstEffect{ lane, 0.32f, 0.38f, 0.95f, 0.72f });
        }
        engine::audio::AudioSystem::play(engine::audio::SoundEvent::GuitarChord);
        m_judgmentTimer = 0.85f;
    }
}

void RhythmMinigame::update(float dt, narrative::StoryState& storyState) {
    if (!m_active) {
        return;
    }

    for (int i = 0; i < 4; ++i) {
        if (m_laneFlashTimer[i] > 0.0f) {
            m_laneFlashTimer[i] -= dt;
            if (m_laneFlashTimer[i] < 0.0f) m_laneFlashTimer[i] = 0.0f;
        }
    }

    if (m_judgmentTimer > 0.0f) {
        m_judgmentTimer -= dt;
        if (m_judgmentTimer < 0.0f) m_judgmentTimer = 0.0f;
    }

    for (size_t i = 0; i < m_bursts.size();) {
        m_bursts[i].timer -= dt;
        if (m_bursts[i].timer <= 0.0f) {
            m_bursts.erase(m_bursts.begin() + static_cast<std::ptrdiff_t>(i));
        } else {
            ++i;
        }
    }

    if (m_finished) {
        return;
    }

    m_songTime += dt;

    // Check for missed notes past the hit window
    for (auto& note : m_notes) {
        if (!note.resolved && (m_songTime - note.hitTime) > 0.32f) {
            note.resolved   = true;
            note.wasHit     = false;
            m_combo         = 0;
            ++m_notesMissed;
            m_lastJudgment  = "MISS";
            m_judgmentTimer = 0.65f;
        }
    }

    // Complete song when timer reaches end
    if (m_songTime >= m_songDuration) {
        m_finished      = true;
        m_lastJudgment  = "SONG COMPLETE! Press [E] or [Enter] to Return";
        m_judgmentTimer = 10.0f;
        storyState.setFlag("completed_rhythm_minigame", 1);
        storyState.setFlag("rhythm_high_score", m_score);
        storyState.showToast("Acoustic Song Complete! Score: " + std::to_string(m_score), 4.0f);
    }
}

void RhythmMinigame::autoPlayStep(float dt, narrative::StoryState& storyState) {
    if (!m_active) {
        return;
    }
    // Automatically hit any note within 0.08s of current songTime for automated verification
    for (const auto& note : m_notes) {
        if (!note.resolved && std::fabs(note.hitTime - m_songTime) <= 0.08f) {
            triggerLaneInput(note.lane, storyState);
        }
    }
    update(dt, storyState);
}

void RhythmMinigame::onKeyPress(unsigned char key, narrative::StoryState& storyState) {
    if (!m_active) {
        return;
    }

    if (m_finished) {
        if (key == 'e' || key == 'E' || key == 13 || key == ' ') {
            stopSession();
        }
        return;
    }

    // Exit early with Backspace (8)
    if (key == 8) {
        m_finished = true;
        storyState.setFlag("completed_rhythm_minigame", 1);
        stopSession();
        return;
    }

    if (key == 'd' || key == 'D' || key == '1') triggerLaneInput(0, storyState);
    if (key == 'f' || key == 'F' || key == '2') triggerLaneInput(1, storyState);
    if (key == 'j' || key == 'J' || key == '3') triggerLaneInput(2, storyState);
    if (key == 'k' || key == 'K' || key == '4') triggerLaneInput(3, storyState);
}

void RhythmMinigame::render2D(int screenWidth, int screenHeight) const {
    if (!m_active) {
        return;
    }

    float w = static_cast<float>(screenWidth);
    float h = static_cast<float>(screenHeight);

    float boardW = 680.0f;
    float boardH = 520.0f;
    float boardX = (w - boardW) * 0.5f;
    float boardY = (h - boardH) * 0.5f;

    // 1. Acoustic Fretboard Stage Panel
    engine::ui::TextRenderer::drawPanel2D(
        boardX, boardY, boardW, boardH,
        0.05f, 0.06f, 0.11f, 0.95f,
        0.96f, 0.78f, 0.36f);

    // 2. Vector Stroke Header Title (Module 3 glutStrokeString + Module 7 glTranslatef/glScalef)
    engine::ui::TextRenderer::drawStrokeText2D(
        boardX + 26.0f, boardY + boardH - 42.0f,
        "ACOUSTIC RHYTHM SESSION - ECHOES IN THE RAIN",
        0.13f, 2.0f, GLUT_STROKE_ROMAN,
        1.0f, 0.88f, 0.48f);

    // Score, Combo & Progress readout
    std::ostringstream stats;
    stats << "Score: " << m_score
          << "   |   Combo: " << m_combo << "x (Max " << m_maxCombo << "x)"
          << "   |   Notes Hit: " << m_notesHit << "/" << m_notes.size();
    engine::ui::TextRenderer::drawBitmapText2D(
        boardX + 28.0f, boardY + boardH - 70.0f,
        stats.str(), GLUT_BITMAP_HELVETICA_18,
        0.92f, 0.96f, 1.0f);

    // 3. Draw Fretboard Lanes, Stippled Measure Bars, Target Hit Rings & Scrolling Notes
    const float laneStartX = boardX + 145.0f;
    const float laneSpacing = 130.0f;
    const float trackTopY   = boardY + boardH - 96.0f;
    const float hitLineY    = boardY + 95.0f;
    const float scrollSpeed = 215.0f; // Pixels per second

    const char* keyLabels[4] = { "[D / 1]", "[F / 2]", "[J / 3]", "[K / 4]" };
    const float laneColors[4][3] = {
        { 0.98f, 0.45f, 0.42f }, // Warm Coral
        { 0.98f, 0.82f, 0.35f }, // Golden Amber
        { 0.38f, 0.88f, 0.75f }, // Mint Teal
        { 0.48f, 0.72f, 0.98f }  // Sky Blue
    };

    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_POINT_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Acoustic Soundhole Rosette Decoration in Center of Fretboard (GL_LINE_LOOP + glLineStipple)
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(2, engine::renderer::PrimitiveRenderer::kStippleDashDot);
    glLineWidth(1.5f);
    glColor4f(0.85f, 0.65f, 0.32f, 0.28f);
    glBegin(GL_LINE_LOOP);
    constexpr float kTwoPi = 6.28318530718f;
    for (int k = 0; k < 32; ++k) {
        float ang = (static_cast<float>(k) / 32.0f) * kTwoPi;
        glVertex2f(boardX + boardW * 0.5f + std::cos(ang) * 95.0f,
                   (trackTopY + hitLineY) * 0.5f + std::sin(ang) * 95.0f);
    }
    glEnd();

    // Horizontal Stippled Fret Wires (Module 2 GL_LINES + glLineStipple)
    glLineStipple(1, engine::renderer::PrimitiveRenderer::kStippleDashed);
    glColor4f(0.55f, 0.62f, 0.75f, 0.40f);
    glBegin(GL_LINES);
    for (int f = 0; f <= 4; ++f) {
        float fy = hitLineY + static_cast<float>(f) * ((trackTopY - hitLineY) * 0.25f);
        glVertex2f(boardX + 65.0f,         fy);
        glVertex2f(boardX + boardW - 65.0f, fy);
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    // 4 Vertical Guitar Strings (GL_LINES) & Target Hit Rings (GL_LINE_LOOP + GL_TRIANGLE_FAN)
    for (int lane = 0; lane < 4; ++lane) {
        float lx = laneStartX + static_cast<float>(lane) * laneSpacing;
        float flash = m_laneFlashTimer[lane] / 0.22f;

        // Guitar string line
        glLineWidth(2.2f + flash * 2.5f);
        glBegin(GL_LINES);
        glColor4f(laneColors[lane][0], laneColors[lane][1], laneColors[lane][2], 0.45f + flash * 0.5f);
        glVertex2f(lx, trackTopY);
        glVertex2f(lx, hitLineY - 18.0f);
        glEnd();

        // Target Receptor Glow (GL_TRIANGLE_FAN when key pressed)
        if (flash > 0.01f) {
            glBegin(GL_TRIANGLE_FAN);
            glColor4f(laneColors[lane][0], laneColors[lane][1], laneColors[lane][2], 0.55f * flash);
            glVertex2f(lx, hitLineY);
            glColor4f(laneColors[lane][0], laneColors[lane][1], laneColors[lane][2], 0.0f);
            for (int k = 0; k <= 16; ++k) {
                float ang = (static_cast<float>(k) / 16.0f) * kTwoPi;
                glVertex2f(lx + std::cos(ang) * 28.0f, hitLineY + std::sin(ang) * 28.0f);
            }
            glEnd();
        }

        // Target Receptor Ring (GL_LINE_LOOP)
        glLineWidth(2.2f);
        glColor4f(laneColors[lane][0], laneColors[lane][1], laneColors[lane][2], 0.92f);
        glBegin(GL_LINE_LOOP);
        for (int k = 0; k < 16; ++k) {
            float ang = (static_cast<float>(k) / 16.0f) * kTwoPi;
            glVertex2f(lx + std::cos(ang) * 20.0f, hitLineY + std::sin(ang) * 20.0f);
        }
        glEnd();
    }

    // 5. Render Scrolling Musical Note Gems using Matrix Transformations (glPushMatrix/glTranslatef/glRotatef/glScalef)
    for (const auto& note : m_notes) {
        if (note.resolved) {
            continue;
        }
        float timeUntilHit = note.hitTime - m_songTime;
        float ny = hitLineY + timeUntilHit * scrollSpeed;
        if (ny < boardY + 45.0f || ny > trackTopY + 10.0f) {
            continue;
        }

        float nx = laneStartX + static_cast<float>(note.lane) * laneSpacing;
        const float* col = laneColors[note.lane];

        glPushMatrix();
        glTranslatef(nx, ny, 0.0f);
        glRotatef(m_songTime * 110.0f, 0.0f, 0.0f, 1.0f);
        float pulse = 1.0f + 0.12f * std::sin(m_songTime * 10.0f);
        glScalef(pulse, pulse, 1.0f);

        // Filled diamond note gem (Module 2 GL_POLYGON with smooth vertex color gradient)
        glBegin(GL_POLYGON);
        glColor3f(1.0f, 0.98f, 0.90f);
        glVertex2f( 0.0f,  16.0f);
        glColor3f(col[0], col[1], col[2]);
        glVertex2f( 16.0f,  0.0f);
        glVertex2f( 0.0f, -16.0f);
        glVertex2f(-16.0f,  0.0f);
        glEnd();

        // Crisp white border outline (GL_LINE_LOOP)
        glLineWidth(1.6f);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f( 0.0f,  16.0f);
        glVertex2f( 16.0f,  0.0f);
        glVertex2f( 0.0f, -16.0f);
        glVertex2f(-16.0f,  0.0f);
        glEnd();

        glPopMatrix();
    }

    // 6. Render Hit Burst Star Particles (Module 2 GL_POINTS + Transformed GL_TRIANGLES)
    for (const auto& burst : m_bursts) {
        float bx = laneStartX + static_cast<float>(burst.lane) * laneSpacing;
        float progress = 1.0f - (burst.timer / 0.38f);
        float radius   = 14.0f + progress * 36.0f;
        float alpha    = burst.timer / 0.38f;

        glPointSize(5.0f);
        glColor4f(burst.r, burst.g, burst.b, alpha);
        glBegin(GL_POINTS);
        for (int p = 0; p < 8; ++p) {
            float ang = (static_cast<float>(p) / 8.0f) * kTwoPi + progress * 1.5f;
            glVertex2f(bx + std::cos(ang) * radius, hitLineY + std::sin(ang) * radius);
        }
        glEnd();
    }

    glPopAttrib();

    // 7. Key Labels underneath each lane receptor
    for (int lane = 0; lane < 4; ++lane) {
        float lx = laneStartX + static_cast<float>(lane) * laneSpacing;
        engine::ui::TextRenderer::drawBitmapText2D(
            lx - 26.0f, hitLineY - 40.0f,
            keyLabels[lane],
            GLUT_BITMAP_HELVETICA_18,
            laneColors[lane][0], laneColors[lane][1], laneColors[lane][2]);
    }

    // 8. Live Judgment Banner & Footer Instructions
    if (m_judgmentTimer > 0.0f && !m_lastJudgment.empty()) {
        engine::ui::TextRenderer::drawBitmapText2D(
            boardX + 220.0f, boardY + 22.0f,
            m_lastJudgment,
            GLUT_BITMAP_HELVETICA_18,
            1.0f, 0.92f, 0.45f);
    } else {
        engine::ui::TextRenderer::drawBitmapText2D(
            boardX + 125.0f, boardY + 20.0f,
            "Press [D/F/J/K] or [1/2/3/4] as notes cross the rings! ([Backspace] Exit)",
            GLUT_BITMAP_HELVETICA_12,
            0.78f, 0.86f, 0.94f);
    }
}

} // namespace minigames
} // namespace game
