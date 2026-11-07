#include "game/interactions/InteractionSystem.h"
#include "engine/renderer/PrimitiveRenderer.h"
#include "engine/ui/TextRenderer.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>

namespace game {
namespace interactions {

InteractionManager::InteractionManager()
    : m_activeIndex(-1)
    , m_totalInteractions(0)
{}

void InteractionManager::clear() {
    m_items.clear();
    m_activeIndex = -1;
}

void InteractionManager::registerInteractable(
    const std::string& id,
    const std::string& displayName,
    const std::string& promptLabel,
    const std::string& dialogueNodeId,
    float worldX, float worldY, float worldZ,
    float triggerRadius)
{
    m_items.push_back(InteractableObject{
        id,
        displayName,
        promptLabel,
        dialogueNodeId,
        worldX, worldY, worldZ,
        triggerRadius,
        0
    });
}

void InteractionManager::update(float playerX, float playerZ) {
    m_activeIndex = -1;
    float bestDistSq = 1e9f;

    for (size_t i = 0; i < m_items.size(); ++i) {
        float dx = playerX - m_items[i].worldX;
        float dz = playerZ - m_items[i].worldZ;
        float d2 = dx * dx + dz * dz;
        float r2 = m_items[i].triggerRadius * m_items[i].triggerRadius;
        if (d2 <= r2 && d2 < bestDistSq) {
            bestDistSq    = d2;
            m_activeIndex = static_cast<int>(i);
        }
    }
}

const InteractableObject* InteractionManager::getActiveInteractable() const {
    if (m_activeIndex >= 0 && m_activeIndex < static_cast<int>(m_items.size())) {
        return &m_items[m_activeIndex];
    }
    return nullptr;
}

InteractableObject* InteractionManager::getActiveInteractableMutable() {
    if (m_activeIndex >= 0 && m_activeIndex < static_cast<int>(m_items.size())) {
        return &m_items[m_activeIndex];
    }
    return nullptr;
}

void InteractionManager::incrementInteractionCount(InteractableObject* obj) {
    if (obj) {
        ++obj->interactCount;
    }
    ++m_totalInteractions;
}

void InteractionManager::render3DIndicators(float timeSeconds, bool showAllDebug) const {
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    constexpr float kTwoPi = 6.28318530718f;

    for (size_t i = 0; i < m_items.size(); ++i) {
        bool isActive = (static_cast<int>(i) == m_activeIndex);
        const auto& item = m_items[i];
        float bob = std::sin(timeSeconds * 4.2f + static_cast<float>(i) * 0.9f) * 0.06f;

        // 1. Floating 3D Amber Diamond Marker above the object/NPC (Module 2 GL_TRIANGLES)
        // Always visible from far away so players can spot interactables from the fixed camera
        glPushMatrix();
        glTranslatef(item.worldX, item.worldY + bob, item.worldZ);
        glRotatef(timeSeconds * 90.0f, 0.0f, 1.0f, 0.0f);

        float s = isActive ? 0.11f : 0.075f;
        glColor4f(
            isActive ? 1.0f  : 0.95f,
            isActive ? 0.88f : 0.76f,
            isActive ? 0.30f : 0.32f,
            isActive ? 0.98f : 0.80f);

        glBegin(GL_TRIANGLES);
        // Upper pyramid
        glVertex3f( 0.0f,  s * 1.4f,  0.0f); glVertex3f(-s, 0.0f,  s); glVertex3f( s, 0.0f,  s);
        glVertex3f( 0.0f,  s * 1.4f,  0.0f); glVertex3f( s, 0.0f,  s); glVertex3f( s, 0.0f, -s);
        glVertex3f( 0.0f,  s * 1.4f,  0.0f); glVertex3f( s, 0.0f, -s); glVertex3f(-s, 0.0f, -s);
        glVertex3f( 0.0f,  s * 1.4f,  0.0f); glVertex3f(-s, 0.0f, -s); glVertex3f(-s, 0.0f,  s);
        // Lower pyramid
        glVertex3f( 0.0f, -s * 1.4f,  0.0f); glVertex3f( s, 0.0f,  s); glVertex3f(-s, 0.0f,  s);
        glVertex3f( 0.0f, -s * 1.4f,  0.0f); glVertex3f( s, 0.0f, -s); glVertex3f( s, 0.0f,  s);
        glVertex3f( 0.0f, -s * 1.4f,  0.0f); glVertex3f(-s, 0.0f, -s); glVertex3f( s, 0.0f, -s);
        glVertex3f( 0.0f, -s * 1.4f,  0.0f); glVertex3f(-s, 0.0f,  s); glVertex3f(-s, 0.0f, -s);
        glEnd();
        glPopMatrix();

        if (!isActive && !showAllDebug) {
            continue;
        }

        // 2. Stippled Ground Trigger Ring (Module 2 GL_LINE_LOOP + glLineStipple)
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, isActive ? engine::renderer::PrimitiveRenderer::kStippleDashed
                                  : engine::renderer::PrimitiveRenderer::kStippleDotted);
        glLineWidth(isActive ? 2.0f : 1.2f);
        glColor4f(isActive ? 1.0f : 0.45f, isActive ? 0.85f : 0.75f, 0.35f, isActive ? 0.85f : 0.45f);

        glBegin(GL_LINE_LOOP);
        constexpr int kRingSegs = 24;
        for (int k = 0; k < kRingSegs; ++k) {
            float ang = (static_cast<float>(k) / static_cast<float>(kRingSegs)) * kTwoPi;
            glVertex3f(
                item.worldX + std::cos(ang) * (isActive ? 0.48f : item.triggerRadius),
                0.015f,
                item.worldZ + std::sin(ang) * (isActive ? 0.48f : item.triggerRadius));
        }
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }

    glPopAttrib();
}

void InteractionManager::render2DPrompt(int screenWidth, int screenHeight) const {
    const InteractableObject* active = getActiveInteractable();
    if (!active) {
        return;
    }
    (void)screenHeight;

    int textW = engine::ui::TextRenderer::measureBitmapTextWidth(
        active->promptLabel, GLUT_BITMAP_HELVETICA_18);
    float boxW = static_cast<float>(textW) + 56.0f;
    if (boxW < 260.0f) boxW = 260.0f;
    float boxH = 38.0f;
    float boxX = (static_cast<float>(screenWidth) - boxW) * 0.5f;
    float boxY = 48.0f;

    engine::ui::TextRenderer::drawPillBadge2D(
        boxX, boxY, boxW, boxH,
        active->promptLabel,
        0.07f, 0.12f, 0.22f, 0.95f,
        0.46f, 0.68f, 0.94f,
        0.92f, 0.96f, 1.0f,
        GLUT_BITMAP_HELVETICA_18);
}

} // namespace interactions
} // namespace game
