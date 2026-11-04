#ifndef GAME_PLAYER_PLAYERCONTROLLER_H
#define GAME_PLAYER_PLAYERCONTROLLER_H

#include "engine/animation/SpriteAnimator.h"
#include "engine/renderer/SpriteQuad3D.h"
#include "engine/collision/CollisionSystem.h"
#include "engine/assets/Texture2D.h"
#include <string>

namespace game {
namespace player {

class PlayerController {
public:
    PlayerController();
    ~PlayerController();

    bool init(const std::string& spriteSheetPath = "assets/sprites/player_elijah_sheet.png");
    void destroy();

    void update(float dt, const engine::collision::CollisionWorld& collisionWorld, bool allowArrowKeys = true);
    void simulateMoveStep(float moveDirX, float moveDirZ, float dt, const engine::collision::CollisionWorld& collisionWorld);

    void render(float cameraYawDegrees = 0.0f, bool showDebugBounds = false) const;

    void setPosition(float x, float y, float z);
    float getX() const { return m_posX; }
    float getY() const { return m_posY; }
    float getZ() const { return m_posZ; }

    bool isMoving() const { return m_animator.getState() == engine::animation::AnimState::Walk; }
    bool isFacingLeft() const { return m_animator.isFlippedHorizontal(); }
    int  getCurrentFrame() const { return m_animator.getCurrentFrame(); }
    const std::string& getLastHitColliderTag() const { return m_lastHitTag; }
    bool isValid() const { return m_spriteSheet.isValid() && m_spriteQuad.isValid(); }

private:
    float                            m_posX;
    float                            m_posY;
    float                            m_posZ;
    float                            m_moveSpeed;
    float                            m_halfWidthX;
    float                            m_halfDepthZ;
    std::string                      m_lastHitTag;
    engine::assets::Texture2D        m_spriteSheet;
    engine::animation::SpriteAnimator m_animator;
    engine::renderer::SpriteQuad3D   m_spriteQuad;
};

} // namespace player
} // namespace game

#endif // GAME_PLAYER_PLAYERCONTROLLER_H
