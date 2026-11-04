#include "engine/renderer/PrimitiveRenderer.h"
#include <cmath>

namespace engine {
namespace renderer {

void PrimitiveRenderer::drawReferenceGrid(float halfExtent, float step) {
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glLineWidth(1.0f);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, kStippleDotted);

    glColor4f(0.45f, 0.48f, 0.55f, 0.45f);
    glBegin(GL_LINES);
    for (float i = -halfExtent; i <= halfExtent + 0.001f; i += step) {
        glVertex3f(i, 0.002f, -halfExtent);
        glVertex3f(i, 0.002f,  halfExtent);

        glVertex3f(-halfExtent, 0.002f, i);
        glVertex3f( halfExtent, 0.002f, i);
    }
    glEnd();

    glDisable(GL_LINE_STIPPLE);
    glPopAttrib();
}

void PrimitiveRenderer::drawCoordinateAxes(float length) {
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(2.5f);

    glBegin(GL_LINES);
    // +X Axis (Red)
    glColor3f(0.95f, 0.25f, 0.25f);
    glVertex3f(0.0f, 0.01f, 0.0f);
    glVertex3f(length, 0.01f, 0.0f);

    // +Y Axis (Green - using Module 2 glColor3ub example style)
    glColor3ub(92, 181, 100);
    glVertex3f(0.0f, 0.01f, 0.0f);
    glVertex3f(0.0f, length, 0.0f);

    // +Z Axis (Blue)
    glColor3f(0.25f, 0.55f, 0.95f);
    glVertex3f(0.0f, 0.01f, 0.0f);
    glVertex3f(0.0f, 0.01f, length);
    glEnd();

    glPopAttrib();
}

void PrimitiveRenderer::drawStippledWireBox(
    float cx, float cy, float cz,
    float sx, float sy, float sz,
    GLfloat r, GLfloat g, GLfloat b,
    GLint factor,
    GLushort pattern,
    float lineWidth)
{
    float hx = sx * 0.5f;
    float hy = sy * 0.5f;
    float hz = sz * 0.5f;

    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glLineWidth(lineWidth);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(factor, pattern);
    glColor3f(r, g, b);

    // Bottom face loop (GL_LINE_LOOP)
    glBegin(GL_LINE_LOOP);
    glVertex3f(cx - hx, cy - hy, cz - hz);
    glVertex3f(cx + hx, cy - hy, cz - hz);
    glVertex3f(cx + hx, cy - hy, cz + hz);
    glVertex3f(cx - hx, cy - hy, cz + hz);
    glEnd();

    // Top face loop (GL_LINE_LOOP)
    glBegin(GL_LINE_LOOP);
    glVertex3f(cx - hx, cy + hy, cz - hz);
    glVertex3f(cx + hx, cy + hy, cz - hz);
    glVertex3f(cx + hx, cy + hy, cz + hz);
    glVertex3f(cx - hx, cy + hy, cz + hz);
    glEnd();

    // Vertical pillars (GL_LINES)
    glBegin(GL_LINES);
    glVertex3f(cx - hx, cy - hy, cz - hz); glVertex3f(cx - hx, cy + hy, cz - hz);
    glVertex3f(cx + hx, cy - hy, cz - hz); glVertex3f(cx + hx, cy + hy, cz - hz);
    glVertex3f(cx + hx, cy - hy, cz + hz); glVertex3f(cx + hx, cy + hy, cz + hz);
    glVertex3f(cx - hx, cy - hy, cz + hz); glVertex3f(cx - hx, cy + hy, cz + hz);
    glEnd();

    glDisable(GL_LINE_STIPPLE);
    glPopAttrib();
}

void PrimitiveRenderer::drawDustParticles(float timeSeconds, int count) {
    glPushAttrib(GL_ENABLE_BIT | GL_POINT_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPointSize(3.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < count; ++i) {
        float seed = static_cast<float>(i) * 1.37f;
        float px = std::sin(seed * 2.1f + timeSeconds * 0.25f) * 3.8f;
        float py = 0.6f + std::fmod(seed + timeSeconds * 0.18f, 2.8f);
        float pz = std::cos(seed * 1.7f + timeSeconds * 0.20f) * 2.8f;
        float alpha = 0.35f + 0.35f * std::sin(timeSeconds * 1.8f + seed);

        glColor4f(1.0f, 0.88f, 0.58f, alpha);
        glVertex3f(px, py, pz);
    }
    glEnd();

    glPopAttrib();
}

void PrimitiveRenderer::drawRainAndRipples(float timeSeconds, int dropCount) {
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_POINT_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Slanted Rain Streaks using GL_LINES + glLineStipple (Module 2)
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, kStippleDashed);
    glLineWidth(1.4f);

    glBegin(GL_LINES);
    for (int i = 0; i < dropCount; ++i) {
        float seed = static_cast<float>(i) * 1.91f;
        float rx = std::sin(seed * 3.7f) * 5.2f;
        float rz = std::cos(seed * 2.3f) * 3.2f;
        float cycle = std::fmod(seed + timeSeconds * 5.8f, 4.2f);
        float ryTop = 4.2f - cycle;
        float ryBot = ryTop - 0.42f;

        glColor4f(0.68f, 0.82f, 0.98f, 0.55f);
        glVertex3f(rx,        ryTop, rz);
        glColor4f(0.82f, 0.92f, 1.00f, 0.85f);
        glVertex3f(rx - 0.08f, ryBot, rz);
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    // 2. Expanding Ground Puddle Splash Ripples using GL_LINE_LOOP (Module 2)
    constexpr float kTwoPi = 6.28318530718f;
    constexpr int   kSegs  = 16;
    glLineWidth(1.2f);
    for (int p = 0; p < 10; ++p) {
        float pSeed = static_cast<float>(p) * 2.47f;
        float px = std::sin(pSeed * 1.9f) * 4.2f;
        float pz = 0.4f + std::cos(pSeed * 2.7f) * 2.2f;
        float phase = std::fmod(timeSeconds * 1.4f + pSeed, 1.0f);
        float radius = 0.05f + phase * 0.32f;
        float alpha  = (1.0f - phase) * 0.65f;

        glColor4f(0.65f, 0.84f, 0.98f, alpha);
        glBegin(GL_LINE_LOOP);
        for (int k = 0; k < kSegs; ++k) {
            float ang = (static_cast<float>(k) / static_cast<float>(kSegs)) * kTwoPi;
            glVertex3f(px + std::cos(ang) * radius, 0.012f, pz + std::sin(ang) * radius);
        }
        glEnd();
    }

    glPopAttrib();
}

void PrimitiveRenderer::drawModule2PrimitiveShowcase(float originX, float originY, float originZ) {
    glPushAttrib(GL_ENABLE_BIT | GL_POINT_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(originX, originY, originZ);

    // 1. GL_POINTS
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.9f, 0.3f);
    glVertex3f(-2.0f, 0.1f, 0.0f);
    glVertex3f(-1.8f, 0.3f, 0.0f);
    glVertex3f(-1.6f, 0.1f, 0.0f);
    glEnd();

    // 2. GL_LINE_STRIP (with dash-dot stipple 0x0C0F)
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, kStippleDashDot);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    glColor3f(0.4f, 0.85f, 1.0f);
    glVertex3f(-1.3f, 0.0f, 0.0f);
    glVertex3f(-1.1f, 0.35f, 0.0f);
    glVertex3f(-0.9f, 0.1f, 0.0f);
    glVertex3f(-0.7f, 0.4f, 0.0f);
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    // 3. GL_TRIANGLES (smooth per-vertex color interpolation)
    glBegin(GL_TRIANGLES);
    glColor3f(0.95f, 0.3f, 0.3f); glVertex3f(-0.4f, 0.0f, 0.0f);
    glColor3ub(92, 181, 100);     glVertex3f(-0.1f, 0.0f, 0.0f);
    glColor3f(0.3f, 0.5f, 0.95f); glVertex3f(-0.25f, 0.38f, 0.0f);
    glEnd();

    // 4. GL_TRIANGLE_STRIP
    glBegin(GL_TRIANGLE_STRIP);
    glColor3f(0.95f, 0.65f, 0.25f);
    glVertex3f(0.1f, 0.0f, 0.0f);
    glVertex3f(0.2f, 0.35f, 0.0f);
    glColor3f(0.85f, 0.35f, 0.55f);
    glVertex3f(0.35f, 0.0f, 0.0f);
    glVertex3f(0.45f, 0.35f, 0.0f);
    glEnd();

    // 5. GL_TRIANGLE_FAN
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 0.95f, 0.75f);
    glVertex3f(0.8f, 0.18f, 0.0f);
    glColor3f(0.35f, 0.78f, 0.72f);
    glVertex3f(0.65f, 0.0f, 0.0f);
    glVertex3f(0.95f, 0.0f, 0.0f);
    glVertex3f(1.0f,  0.22f, 0.0f);
    glVertex3f(0.8f,  0.38f, 0.0f);
    glVertex3f(0.6f,  0.22f, 0.0f);
    glEnd();

    // 6. GL_QUADS & GL_QUAD_STRIP & GL_POLYGON
    glBegin(GL_QUADS);
    glColor3f(0.75f, 0.55f, 0.92f);
    glVertex3f(1.15f, 0.0f, 0.0f);
    glVertex3f(1.45f, 0.0f, 0.0f);
    glVertex3f(1.45f, 0.32f, 0.0f);
    glVertex3f(1.15f, 0.32f, 0.0f);
    glEnd();

    glBegin(GL_QUAD_STRIP);
    glColor3f(0.45f, 0.75f, 0.55f);
    glVertex3f(1.55f, 0.0f, 0.0f);
    glVertex3f(1.55f, 0.32f, 0.0f);
    glVertex3f(1.75f, 0.05f, 0.0f);
    glVertex3f(1.75f, 0.28f, 0.0f);
    glColor3f(0.85f, 0.75f, 0.35f);
    glVertex3f(1.95f, 0.0f, 0.0f);
    glVertex3f(1.95f, 0.32f, 0.0f);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0.92f, 0.48f, 0.38f);
    glVertex3f(2.15f, 0.0f, 0.0f);
    glVertex3f(2.35f, 0.0f, 0.0f);
    glVertex3f(2.42f, 0.20f, 0.0f);
    glVertex3f(2.25f, 0.36f, 0.0f);
    glVertex3f(2.08f, 0.20f, 0.0f);
    glEnd();

    glPopMatrix();
    glPopAttrib();
}

} // namespace renderer
} // namespace engine
