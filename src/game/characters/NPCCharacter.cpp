#include "game/characters/NPCCharacter.h"
#include "engine/renderer/PrimitiveRenderer.h"
#include <cmath>

namespace game {
namespace characters {

NPCCharacter::NPCCharacter()
    : m_posX(0.0f)
    , m_posY(0.0f)
    , m_posZ(0.0f)
    , m_halfWidthX(0.28f)
    , m_halfDepthZ(0.22f)
    , m_animator(4, 2)
{}

NPCCharacter::~NPCCharacter() {
    destroy();
}

bool NPCCharacter::init(
    const std::string& name,
    const std::string& spriteSheetPath,
    float x, float y, float z,
    const std::string& dialogueId)
{
    m_name       = name;
    m_dialogueId = dialogueId;
    m_posX       = x;
    m_posY       = y;
    m_posZ       = z;

    bool texOk = m_spriteSheet.loadFromFile(spriteSheetPath, true, false, true);
    m_spriteQuad.init(0.88f, 1.35f);
    m_animator.setState(engine::animation::AnimState::Idle);
    m_spriteQuad.updateUVs(m_animator.getCurrentUVRect());
    return texOk && m_spriteQuad.isValid();
}

void NPCCharacter::destroy() {
    m_spriteQuad.destroy();
    m_spriteSheet.destroy();
}

void NPCCharacter::registerCollider(engine::collision::CollisionWorld& world) const {
    world.addBoxCollider(
        m_posX, m_posY + 0.65f, m_posZ,
        m_halfWidthX * 2.0f, 1.30f, m_halfDepthZ * 2.0f,
        "NPC_" + m_name);
}

void NPCCharacter::update(float dt, float playerX, float playerZ) {
    float dx = playerX - m_posX;
    float dz = playerZ - m_posZ;
    float distSq = dx * dx + dz * dz;

    // Face the player when the player is within conversational range
    if (distSq < 6.25f) {
        if (dx < -0.05f) {
            m_animator.setFlipHorizontal(true);
        } else if (dx > 0.05f) {
            m_animator.setFlipHorizontal(false);
        }
    }

    m_animator.update(dt);
    m_spriteQuad.updateUVs(m_animator.getCurrentUVRect());
}

void NPCCharacter::render(float cameraYawDegrees, bool showDebugBounds) const {
    m_spriteQuad.draw(m_spriteSheet, m_posX, m_posY, m_posZ, cameraYawDegrees, true);

    if (showDebugBounds) {
        engine::renderer::PrimitiveRenderer::drawStippledWireBox(
            m_posX, m_posY + 0.65f, m_posZ,
            m_halfWidthX * 2.0f, 1.30f, m_halfDepthZ * 2.0f,
            1.0f, 0.68f, 0.28f,
            1, engine::renderer::PrimitiveRenderer::kStippleDashed, 2.0f);
    }
}

} // namespace characters
} // namespace game
