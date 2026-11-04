#ifndef GAME_CHARACTERS_NPCCHARACTER_H
#define GAME_CHARACTERS_NPCCHARACTER_H

#include "engine/animation/SpriteAnimator.h"
#include "engine/renderer/SpriteQuad3D.h"
#include "engine/assets/Texture2D.h"
#include "engine/collision/CollisionSystem.h"
#include <string>

namespace game {
namespace characters {

class NPCCharacter {
public:
    NPCCharacter();
    ~NPCCharacter();

    bool init(
        const std::string& name,
        const std::string& spriteSheetPath,
        float x, float y, float z,
        const std::string& dialogueId);

    void destroy();

    void registerCollider(engine::collision::CollisionWorld& world) const;
    void update(float dt, float playerX, float playerZ);
    void render(float cameraYawDegrees = 0.0f, bool showDebugBounds = false) const;

    const std::string& getName() const { return m_name; }
    const std::string& getDialogueId() const { return m_dialogueId; }
    float getX() const { return m_posX; }
    float getY() const { return m_posY; }
    float getZ() const { return m_posZ; }
    bool isValid() const { return m_spriteSheet.isValid() && m_spriteQuad.isValid(); }

private:
    std::string                       m_name;
    std::string                       m_dialogueId;
    float                             m_posX;
    float                             m_posY;
    float                             m_posZ;
    float                             m_halfWidthX;
    float                             m_halfDepthZ;
    engine::assets::Texture2D         m_spriteSheet;
    engine::animation::SpriteAnimator m_animator;
    engine::renderer::SpriteQuad3D    m_spriteQuad;
};

} // namespace characters
} // namespace game

#endif // GAME_CHARACTERS_NPCCHARACTER_H
