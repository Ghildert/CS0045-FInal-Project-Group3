#include "game/player/PlayerController.h"
#include "engine/input/InputManager.h"
#include "engine/renderer/PrimitiveRenderer.h"
#include <GL/freeglut.h>
#include <cmath>

namespace game {
namespace player {

PlayerController::PlayerController()
    : m_posX(-0.5f)
    , m_posY(0.0f)
    , m_posZ(1.05f)
    , m_moveSpeed(2.65f)
    , m_halfWidthX(0.26f)
    , m_halfDepthZ(0.20f)
    , m_animator(4, 2)
{}

PlayerController::~PlayerController() {
    destroy();
}

bool PlayerController::init(const std::string& spriteSheetPath) {
    bool texOk = m_spriteSheet.loadFromFile(spriteSheetPath, true, false, true);
    m_spriteQuad.init(0.88f, 1.35f);
    m_spriteQuad.updateUVs(m_animator.getCurrentUVRect());
    return texOk && m_spriteQuad.isValid();
}

void PlayerController::destroy() {
    m_spriteQuad.destroy();
    m_spriteSheet.destroy();
}

void PlayerController::setPosition(float x, float y, float z) {
    m_posX = x;
    m_posY = y;
    m_posZ = z;
}

void PlayerController::simulateMoveStep(
    float moveDirX,
    float moveDirZ,
    float dt,
    const engine::collision::CollisionWorld& collisionWorld)
{
    float lenSq = moveDirX * moveDirX + moveDirZ * moveDirZ;
    if (lenSq > 0.0001f) {
        float invLen = 1.0f / std::sqrt(lenSq);
        moveDirX *= invLen;
        moveDirZ *= invLen;

        if (moveDirX < -0.05f) {
            m_animator.setFlipHorizontal(true);
        } else if (moveDirX > 0.05f) {
            m_animator.setFlipHorizontal(false);
        }

        m_animator.setState(engine::animation::AnimState::Walk);

        float dx = moveDirX * m_moveSpeed * dt;
        float dz = moveDirZ * m_moveSpeed * dt;
        collisionWorld.resolveMovementXZ(
            m_posX, m_posZ, dx, dz,
            m_halfWidthX, m_halfDepthZ,
            &m_lastHitTag);
    } else {
        m_animator.setState(engine::animation::AnimState::Idle);
    }

    m_animator.update(dt);
    m_spriteQuad.updateUVs(m_animator.getCurrentUVRect());
}

void PlayerController::update(
    float dt,
    const engine::collision::CollisionWorld& collisionWorld,
    bool allowArrowKeys)
{
    using engine::input::InputManager;

    float moveX = 0.0f;
    float moveZ = 0.0f;

    if (InputManager::isKeyDown('a')) moveX -= 1.0f;
    if (InputManager::isKeyDown('d')) moveX += 1.0f;
    if (InputManager::isKeyDown('w')) moveZ -= 1.0f;
    if (InputManager::isKeyDown('s')) moveZ += 1.0f;

    if (allowArrowKeys) {
        if (InputManager::isSpecialDown(GLUT_KEY_LEFT))  moveX -= 1.0f;
        if (InputManager::isSpecialDown(GLUT_KEY_RIGHT)) moveX += 1.0f;
        if (InputManager::isSpecialDown(GLUT_KEY_UP))    moveZ -= 1.0f;
        if (InputManager::isSpecialDown(GLUT_KEY_DOWN))  moveZ += 1.0f;
    }

    simulateMoveStep(moveX, moveZ, dt, collisionWorld);
}

void PlayerController::render(float cameraYawDegrees, bool showDebugBounds) const {
    m_spriteQuad.draw(m_spriteSheet, m_posX, m_posY, m_posZ, cameraYawDegrees, true);

    if (showDebugBounds) {
        engine::renderer::PrimitiveRenderer::drawStippledWireBox(
            m_posX, m_posY + 0.65f, m_posZ,
            m_halfWidthX * 2.0f, 1.30f, m_halfDepthZ * 2.0f,
            0.35f, 0.92f, 1.0f,
            1, engine::renderer::PrimitiveRenderer::kStippleDashed, 2.0f);
    }
}

} // namespace player
} // namespace game
