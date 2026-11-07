#include "engine/scene/SceneTransition.h"
#include "engine/ui/TextRenderer.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>

namespace engine {
namespace scene {

SceneTransition::SceneTransition()
    : m_active(false)
    , m_elapsed(0.0f)
    , m_duration(1.2f)
    , m_midpointFired(false)
{}

void SceneTransition::startTransition(
    const std::string& bannerTitle,
    std::function<void()> onMidpointCallback,
    float totalDurationSeconds)
{
    m_active           = true;
    m_elapsed          = 0.0f;
    m_duration         = (totalDurationSeconds > 0.2f) ? totalDurationSeconds : 0.2f;
    m_midpointFired    = false;
    m_bannerTitle      = bannerTitle;
    m_midpointCallback = onMidpointCallback;
}

void SceneTransition::update(float dt) {
    if (!m_active) {
        return;
    }

    m_elapsed += dt;
    float half = m_duration * 0.5f;
    if (!m_midpointFired && m_elapsed >= half) {
        m_midpointFired = true;
        if (m_midpointCallback) {
            m_midpointCallback();
        }
    }

    if (m_elapsed >= m_duration) {
        m_active  = false;
        m_elapsed = 0.0f;
    }
}

float SceneTransition::getAlpha() const {
    if (!m_active) {
        return 0.0f;
    }
    float t = m_elapsed / m_duration; // 0..1
    // Triangle wave peaking at 1.0 at midpoint (t = 0.5)
    float a = (t < 0.5f) ? (t * 2.0f) : ((1.0f - t) * 2.0f);
    if (a < 0.0f) a = 0.0f;
    if (a > 1.0f) a = 1.0f;
    return a;
}

void SceneTransition::render2D(int screenWidth, int screenHeight) const {
    if (!m_active) {
        return;
    }

    float alpha = getAlpha();
    float w = static_cast<float>(screenWidth);
    float h = static_cast<float>(screenHeight);

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Full-screen fade quad
    glColor4f(0.03f, 0.03f, 0.06f, alpha);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(w,    0.0f);
    glVertex2f(w,    h);
    glVertex2f(0.0f, h);
    glEnd();

    // Cinematic Letterbox Bars sliding in from top and bottom
    float barH = 68.0f * alpha;
    glColor4f(0.01f, 0.01f, 0.02f, 0.98f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);     glVertex2f(w, 0.0f);     glVertex2f(w, barH); glVertex2f(0.0f, barH);
    glVertex2f(0.0f, h - barH); glVertex2f(w, h - barH); glVertex2f(w, h);    glVertex2f(0.0f, h);
    glEnd();

    glPopAttrib();

    if (alpha > 0.45f && !m_bannerTitle.empty()) {
        ui::TextRenderer::drawStrokeTextCentered2D(
            w * 0.5f, h * 0.5f,
            m_bannerTitle,
            0.20f, 2.2f, GLUT_STROKE_ROMAN,
            1.0f, 0.88f, 0.52f);
    }
}

} // namespace scene
} // namespace engine
