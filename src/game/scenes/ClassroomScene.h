#ifndef GAME_SCENES_CLASSROOMSCENE_H
#define GAME_SCENES_CLASSROOMSCENE_H

#include "engine/assets/Texture2D.h"
#include "engine/renderer/MeshVBO.h"
#include "engine/renderer/LightingSystem.h"
#include "engine/collision/CollisionSystem.h"
#include "engine/camera/Camera3D.h"
#include "engine/scene/SceneTransition.h"
#include "game/player/PlayerController.h"
#include "game/characters/NPCCharacter.h"
#include "game/interactions/InteractionSystem.h"
#include "game/dialogue/DialogueSystem.h"
#include "game/narrative/StoryState.h"
#include "game/narrative/PhoneSystem.h"
#include "game/minigames/RhythmMinigame.h"
#include <string>

namespace game {
namespace scenes {

enum class LocationId {
    Classroom   = 0, // Room 304 (Late Afternoon)
    Hallway     = 1, // 3rd-Floor School Corridor
    RainyStreet = 2  // 24/7 Sundown Mart & Bus Stop Exterior
};

class ClassroomScene {
public:
    ClassroomScene();
    ~ClassroomScene();

    void init();
    void destroy();

    void startNewGame();
    void quickSave();
    void quickLoad();
    void launchMinigameDirectly();

    void switchLocation(LocationId targetLocation, float spawnX, float spawnZ, bool withTransition = true);
    LocationId getCurrentLocation() const { return m_currentLocation; }
    std::string getCurrentLocationIdString() const;
    std::string getCurrentLocationTitle() const;

    void update(float dt, engine::camera::Camera3D& camera);
    void render3D(const engine::camera::Camera3D& camera);
    void render2D(const engine::camera::Camera3D& camera);

    void onKeyPress(unsigned char key);
    void onSpecialKeyPress(int key);
    void onMouseClick(int button, int state);

    // Developer Mode (activated via key combination Ctrl+Shift+D, Ctrl+D, or typing D-E-V)
    void toggleDevMode();
    void executeDevAction(int actionIndex);
    void bypassCurrentTask();
    void bypassAllTasks();
    bool isDevModeUnlocked() const { return m_devModeUnlocked; }
    bool isDevModeOpen() const { return m_devModeOpen; }

    // Automated Phase 5/6 verification helper
    void runPhase5VerificationStep(float dt, engine::camera::Camera3D& camera);
    void runPhase6VerificationStep(float dt, engine::camera::Camera3D& camera);

    bool isValid() const;
    const player::PlayerController& getPlayer() const { return m_player; }
    const characters::NPCCharacter& getMayaNPC() const { return m_npcMaya; }
    const characters::NPCCharacter& getSamNPC() const { return m_npcSam; }
    const interactions::InteractionManager& getInteractions() const { return m_interactions; }
    const dialogue::DialogueSystem& getDialogue() const { return m_dialogue; }
    const narrative::StoryState& getStoryState() const { return m_storyState; }
    const narrative::PhoneSystem& getPhoneSystem() const { return m_phoneSystem; }
    const minigames::RhythmMinigame& getRhythmMinigame() const { return m_rhythmMinigame; }
    engine::renderer::LightingSystem& getLighting() { return m_lighting; }
    int getLocationsVisitedCount() const { return m_locationsVisitedMask; }

private:
    void buildCollidersAndInteractablesForLocation(LocationId loc);
    void renderClassroom3D(const engine::camera::Camera3D& camera);
    void renderHallway3D(const engine::camera::Camera3D& camera);
    void renderRainyStreet3D(const engine::camera::Camera3D& camera);
    void renderDevModeOverlay2D(int screenWidth, int screenHeight) const;
    void drawDeskWithChair3D(float x, float z, float yawDegrees, const engine::assets::Texture2D& woodTex);
    void drawBench3D(float x, float z, float yawDegrees, float lengthScale, const engine::assets::Texture2D& woodTex);

    LocationId                        m_currentLocation;
    int                               m_locationsVisitedMask;
    engine::renderer::LightingSystem  m_lighting;
    engine::collision::CollisionWorld m_collisionWorld;
    engine::scene::SceneTransition    m_transition;

    // Shared Environment Textures (Module 8 Subtopic 2)
    engine::assets::Texture2D m_texWoodFloor;
    engine::assets::Texture2D m_texClassroomWall;
    engine::assets::Texture2D m_texDeskWood;
    engine::assets::Texture2D m_texChalkboard;
    engine::assets::Texture2D m_texWindowGlass;
    engine::assets::Texture2D m_texCheckerDebug;
    engine::assets::Texture2D m_texHallwayTile;
    engine::assets::Texture2D m_texSchoolLockers;
    engine::assets::Texture2D m_texWetAsphalt;
    engine::assets::Texture2D m_texStorefrontNeon;

    // Reusable VBO Geometry (Modules 4, 5, 6)
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
    engine::renderer::MeshVBO m_meshChairLeg;
    engine::renderer::MeshVBO m_meshSketchbookProp;
    engine::renderer::MeshVBO m_meshLockerBank;
    engine::renderer::MeshVBO m_meshStoreBuilding;
    engine::renderer::MeshVBO m_meshStoreAwning;
    engine::renderer::MeshVBO m_meshDynamicPyramid;

    // Gameplay Entities & Narrative Systems
    player::PlayerController         m_player;
    characters::NPCCharacter         m_npcMaya;
    characters::NPCCharacter         m_npcSam;
    interactions::InteractionManager m_interactions;
    dialogue::DialogueSystem         m_dialogue;
    narrative::StoryState            m_storyState;
    narrative::PhoneSystem           m_phoneSystem;
    minigames::RhythmMinigame        m_rhythmMinigame;

    float m_timeSeconds;
    bool  m_showDebugOverlay;
    bool  m_useCheckerTexture;
    bool  m_wireframeProps;
    int   m_verifyStep;

    // Developer Mode state & key-combo tracker
    bool        m_devModeUnlocked;
    bool        m_devModeOpen;
    int         m_devMenuCursor;
    std::string m_keyComboBuffer;
    bool        m_ctrlDChordLatch;
};

} // namespace scenes
} // namespace game

#endif // GAME_SCENES_CLASSROOMSCENE_H
