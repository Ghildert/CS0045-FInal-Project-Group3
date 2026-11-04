#ifndef GAME_SCENES_CLASSROOMSCENE_H
#define GAME_SCENES_CLASSROOMSCENE_H

#include "engine/assets/Texture2D.h"
#include "engine/renderer/MeshVBO.h"
#include "engine/renderer/LightingSystem.h"
#include "engine/collision/CollisionSystem.h"
#include "engine/camera/Camera3D.h"
#include "game/player/PlayerController.h"
#include "game/characters/NPCCharacter.h"
#include "game/interactions/InteractionSystem.h"
#include "game/dialogue/DialogueSystem.h"

namespace game {
namespace scenes {

class ClassroomScene {
public:
    ClassroomScene();
    ~ClassroomScene();

    void init();
    void destroy();

    void update(float dt, engine::camera::Camera3D& camera);
    void render3D(const engine::camera::Camera3D& camera);
    void render2D(const engine::camera::Camera3D& camera);

    void onKeyPress(unsigned char key);
    void onSpecialKeyPress(int key);
    void onMouseClick(int button, int state);

    // Automated Phase 3 verification helper
    void runPhase3VerificationStep(float dt, engine::camera::Camera3D& camera);

    bool isValid() const;
    const player::PlayerController& getPlayer() const { return m_player; }
    const characters::NPCCharacter& getMayaNPC() const { return m_npcMaya; }
    const interactions::InteractionManager& getInteractions() const { return m_interactions; }
    const dialogue::DialogueSystem& getDialogue() const { return m_dialogue; }
    engine::renderer::LightingSystem& getLighting() { return m_lighting; }

private:
    void buildCollidersAndInteractables();
    void drawDeskWithChair3D(float x, float z, float yawDegrees, const engine::assets::Texture2D& woodTex);

    engine::renderer::LightingSystem m_lighting;
    engine::collision::CollisionWorld m_collisionWorld;

    // Classroom Textures
    engine::assets::Texture2D m_texWoodFloor;
    engine::assets::Texture2D m_texClassroomWall;
    engine::assets::Texture2D m_texDeskWood;
    engine::assets::Texture2D m_texChalkboard;
    engine::assets::Texture2D m_texWindowGlass;
    engine::assets::Texture2D m_texCheckerDebug;

    // Classroom VBO Geometry
    engine::renderer::MeshVBO m_meshFloor;
    engine::renderer::MeshVBO m_meshBackWall;
    engine::renderer::MeshVBO m_meshSideWall;
    engine::renderer::MeshVBO m_meshChalkboard;
    engine::renderer::MeshVBO m_meshDoorPanel;
    engine::renderer::MeshVBO m_meshWindowPane;
    engine::renderer::MeshVBO m_meshDeskTop;
    engine::renderer::MeshVBO m_meshDeskLeg;
    engine::renderer::MeshVBO m_meshChairSeat;
    engine::renderer::MeshVBO m_meshChairBack;
    engine::renderer::MeshVBO m_meshSketchbookProp;
    engine::renderer::MeshVBO m_meshDynamicPyramid;

    // Gameplay Entities & Systems
    player::PlayerController         m_player;
    characters::NPCCharacter         m_npcMaya;
    interactions::InteractionManager m_interactions;
    dialogue::DialogueSystem         m_dialogue;

    float m_timeSeconds;
    bool  m_showDebugOverlay;
    bool  m_useCheckerTexture;
    bool  m_wireframeProps;
};

} // namespace scenes
} // namespace game

#endif // GAME_SCENES_CLASSROOMSCENE_H
