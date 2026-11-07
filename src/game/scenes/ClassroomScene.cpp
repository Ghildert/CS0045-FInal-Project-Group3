#include "game/scenes/ClassroomScene.h"
#include "engine/audio/AudioSystem.h"
#include "engine/core/Math4x4.h"
#include "engine/renderer/PrimitiveRenderer.h"
#include "engine/ui/TextRenderer.h"
#include "engine/input/InputManager.h"
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include <GL/freeglut.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace game {
namespace scenes {

ClassroomScene::ClassroomScene()
    : m_currentLocation(LocationId::Classroom)
    , m_locationsVisitedMask(1)
    , m_timeSeconds(0.0f)
    , m_showDebugOverlay(false)
    , m_useCheckerTexture(false)
    , m_wireframeProps(false)
    , m_verifyStep(0)
    , m_devModeUnlocked(false)
    , m_devModeOpen(false)
    , m_devMenuCursor(0)
    , m_keyComboBuffer("")
    , m_ctrlDChordLatch(false)
{}

ClassroomScene::~ClassroomScene() {
    destroy();
}

void ClassroomScene::init() {
    engine::assets::Texture2D::ensureDefaultTexturesOnDisk();

    m_texWoodFloor.loadFromFile("assets/textures/wood_floor.png", true, true);
    m_texClassroomWall.loadFromFile("assets/textures/classroom_wall.png", true, true);
    m_texDeskWood.loadFromFile("assets/textures/desk_wood.png", true, true);
    m_texChalkboard.loadFromFile("assets/textures/chalkboard.png", true, false);
    m_texWindowGlass.loadFromFile("assets/textures/window_glass.png", true, false);
    m_texCheckerDebug.loadFromFile("assets/textures/checker_debug.png", true, true);
    m_texHallwayTile.loadFromFile("assets/textures/hallway_tile.png", true, true);
    m_texSchoolLockers.loadFromFile("assets/textures/school_lockers.png", true, true);
    m_texWetAsphalt.loadFromFile("assets/textures/wet_asphalt.png", true, true);
    m_texStorefrontNeon.loadFromFile("assets/textures/storefront_neon.png", true, false);

    // Build reusable VBO meshes shared across all 3 environments
    m_meshFloor.buildQuadXZ(9.6f, 6.8f, 6.0f, 4.0f, GL_STATIC_DRAW);
    m_meshBackWall.buildTexturedBox(9.6f, 3.8f, 0.22f, 4.0f, 1.0f, GL_STATIC_DRAW);
    m_meshSideWall.buildTexturedBox(0.22f, 3.8f, 6.8f, 3.0f, 1.0f, GL_STATIC_DRAW);
    m_meshChalkboard.buildTexturedBox(4.3f, 1.45f, 0.08f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshDoorPanel.buildTexturedBox(0.10f, 2.35f, 1.15f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshWindowPane.buildTexturedBox(0.08f, 1.75f, 2.15f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshDeskTop.buildTexturedBox(1.30f, 0.09f, 0.72f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshDeskLeg.buildTexturedBox(0.08f, 0.72f, 0.08f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshChairSeat.buildTexturedBox(0.48f, 0.06f, 0.46f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshChairBack.buildTexturedBox(0.48f, 0.44f, 0.06f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshChairLeg.buildTexturedBox(0.06f, 0.44f, 0.06f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshSketchbookProp.buildTexturedBox(0.38f, 0.06f, 0.28f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshLockerBank.buildTexturedBox(3.4f, 2.25f, 0.55f, 4.0f, 1.0f, GL_STATIC_DRAW);
    m_meshStoreBuilding.buildTexturedBox(5.6f, 3.4f, 1.4f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshStoreAwning.buildTexturedBox(5.8f, 0.18f, 1.35f, 2.0f, 1.0f, GL_STATIC_DRAW);
    m_meshDynamicPyramid.buildPyramid(0.38f, 0.48f, GL_DYNAMIC_DRAW);

    engine::core::Math4x4::translateVertexInMappedVBO(
        m_meshDynamicPyramid.getVBOId(), 0.0f, 0.02f, 0.0f, 0);

    // Initialize Player (Elijah) and NPCs (Maya & Sam)
    m_player.init("assets/sprites/player_elijah_sheet.png");
    m_npcMaya.init("Maya", "assets/sprites/npc_maya_sheet.png", -2.75f, 0.0f, -0.55f, "maya_classroom_talk");
    m_npcSam.init("Sam",   "assets/sprites/npc_sam_sheet.png",  -1.15f, 0.0f, -1.65f, "sam_hallway_talk");

    m_storyState.resetNewGame();
    m_dialogue.initPortraitsAndData("data/dialogue/story_dialogues.txt");
    m_phoneSystem.loadFromExternalData("data/dialogue/phone_messages.txt");

    m_lighting.initClassroomLighting();
    switchLocation(LocationId::Classroom, -0.35f, 1.15f, false);
}

void ClassroomScene::destroy() {
    m_dialogue.destroy();
    m_player.destroy();
    m_npcMaya.destroy();
    m_npcSam.destroy();

    m_meshFloor.destroy();
    m_meshBackWall.destroy();
    m_meshSideWall.destroy();
    m_meshChalkboard.destroy();
    m_meshDoorPanel.destroy();
    m_meshWindowPane.destroy();
    m_meshDeskTop.destroy();
    m_meshDeskLeg.destroy();
    m_meshChairSeat.destroy();
    m_meshChairBack.destroy();
    m_meshChairLeg.destroy();
    m_meshSketchbookProp.destroy();
    m_meshLockerBank.destroy();
    m_meshStoreBuilding.destroy();
    m_meshStoreAwning.destroy();
    m_meshDynamicPyramid.destroy();

    m_texWoodFloor.destroy();
    m_texClassroomWall.destroy();
    m_texDeskWood.destroy();
    m_texChalkboard.destroy();
    m_texWindowGlass.destroy();
    m_texCheckerDebug.destroy();
    m_texHallwayTile.destroy();
    m_texSchoolLockers.destroy();
    m_texWetAsphalt.destroy();
    m_texStorefrontNeon.destroy();
}

std::string ClassroomScene::getCurrentLocationIdString() const {
    switch (m_currentLocation) {
        case LocationId::Classroom:   return "classroom";
        case LocationId::Hallway:     return "hallway";
        case LocationId::RainyStreet: return "rainy_street";
    }
    return "classroom";
}

void ClassroomScene::startNewGame() {
    m_storyState.resetNewGame();
    m_phoneSystem.loadFromExternalData("data/dialogue/phone_messages.txt");
    switchLocation(LocationId::Classroom, -0.35f, 1.15f, true);
}

void ClassroomScene::quickSave() {
    m_storyState.saveToFile(
        "data/saves/save_slot_1.sav",
        getCurrentLocationIdString(),
        m_player.getX(),
        m_player.getZ());
}

void ClassroomScene::quickLoad() {
    std::string sceneId;
    float px = m_player.getX();
    float pz = m_player.getZ();
    if (m_storyState.loadFromFile("data/saves/save_slot_1.sav", sceneId, px, pz)) {
        LocationId target = LocationId::Classroom;
        if (sceneId == "hallway")      target = LocationId::Hallway;
        if (sceneId == "rainy_street") target = LocationId::RainyStreet;
        switchLocation(target, px, pz, true);
    }
}

void ClassroomScene::launchMinigameDirectly() {
    switchLocation(LocationId::RainyStreet, 2.0f, 0.2f, false);
    m_rhythmMinigame.startSession();
}

std::string ClassroomScene::getCurrentLocationTitle() const {
    switch (m_currentLocation) {
        case LocationId::Classroom:   return "ROOM 304 - FEU TECH LAB (LATE AFTERNOON)";
        case LocationId::Hallway:     return "3RD-FLOOR CAMPUS HALLWAY - TWILIGHT";
        case LocationId::RainyStreet: return "937 P. PAREDES ST - FEU TECH & CANTEENS";
    }
    return "";
}

void ClassroomScene::switchLocation(LocationId targetLocation, float spawnX, float spawnZ, bool withTransition) {
    auto applySwitch = [this, targetLocation, spawnX, spawnZ]() {
        m_currentLocation = targetLocation;
        m_locationsVisitedMask |= (1 << static_cast<int>(targetLocation));
        m_player.setPosition(spawnX, 0.0f, spawnZ);
        buildCollidersAndInteractablesForLocation(targetLocation);
    };

    if (withTransition) {
        std::string title = "ROOM 304 - FEU TECH";
        if (targetLocation == LocationId::Hallway)     title = "3RD-FLOOR CAMPUS HALLWAY";
        if (targetLocation == LocationId::RainyStreet) title = "937 P. PAREDES ST - FEU TECH";
        m_transition.startTransition(title, applySwitch, 1.1f);
    } else {
        applySwitch();
    }
}

void ClassroomScene::buildCollidersAndInteractablesForLocation(LocationId loc) {
    m_collisionWorld.clear();
    m_interactions.clear();

    // Common outer boundary walls
    m_collisionWorld.addBoxCollider( 0.0f, 1.9f, -3.30f, 9.6f, 3.8f, 0.35f, "BackWall");
    m_collisionWorld.addBoxCollider( 0.0f, 1.9f,  3.35f, 9.6f, 3.8f, 0.25f, "FrontBoundary");
    m_collisionWorld.addBoxCollider(-4.65f, 1.9f, 0.0f,  0.35f, 3.8f, 6.8f, "LeftWall");
    m_collisionWorld.addBoxCollider( 4.65f, 1.9f, 0.0f,  0.35f, 3.8f, 6.8f, "RightWall");

    if (loc == LocationId::Classroom) {
        // Position Maya by the classroom sunset window
        m_npcMaya.setPositionAndDialogue(-2.75f, 0.0f, -0.55f, "maya_classroom_talk");
        m_npcMaya.registerCollider(m_collisionWorld);

        // Desks
        m_collisionWorld.addBoxCollider( 1.85f, 0.42f, -1.85f, 1.40f, 0.85f, 0.82f, "TeacherDesk");
        m_collisionWorld.addBoxCollider(-1.85f, 0.42f,  0.05f, 1.32f, 0.85f, 0.82f, "StudentDesk_R1C1");
        m_collisionWorld.addBoxCollider( 0.15f, 0.42f,  0.05f, 1.32f, 0.85f, 0.82f, "StudentDesk_R1C2");
        m_collisionWorld.addBoxCollider( 2.15f, 0.42f,  0.05f, 1.32f, 0.85f, 0.82f, "StudentDesk_R1C3");
        m_collisionWorld.addBoxCollider(-1.85f, 0.42f,  1.85f, 1.32f, 0.85f, 0.82f, "StudentDesk_R2C1");
        m_collisionWorld.addBoxCollider( 0.15f, 0.42f,  1.85f, 1.32f, 0.85f, 0.82f, "StudentDesk_R2C2");
        m_collisionWorld.addBoxCollider( 2.15f, 0.42f,  1.85f, 1.32f, 0.85f, 0.82f, "StudentDesk_R2C3");

        m_interactions.registerInteractable(
            "maya_npc", "Maya (Classmate)", "[E] Talk to Maya", "maya_classroom_talk",
            m_npcMaya.getX(), 1.52f, m_npcMaya.getZ(), 1.35f);
        m_interactions.registerInteractable(
            "sketchbook_desk", "Elijah's Sketchbook", "[E] Inspect Sketchbook & Notes", "inspect_sketchbook",
            -1.85f, 1.15f, 0.05f, 1.30f);
        m_interactions.registerInteractable(
            "pyramid_desk", "Graphics Lab Desk", "[E] Inspect OpenGL Pyramid Model", "inspect_pyramid_desk",
            0.15f, 1.15f, 0.05f, 1.25f);
        m_interactions.registerInteractable(
            "teacher_desk", "Teacher's Desk & Models", "[E] Inspect Teapot & Sphere Models", "inspect_teacher_desk",
            1.85f, 1.15f, -1.85f, 1.35f);
        m_interactions.registerInteractable(
            "student_desks_back", "Back-Row Student Desks", "[E] Inspect Student Desks", "inspect_back_desks",
            0.15f, 1.05f, 1.85f, 1.85f);
        m_interactions.registerInteractable(
            "chalkboard_schedule", "Classroom Chalkboard", "[E] Read Chalkboard Schedule", "inspect_chalkboard",
            0.0f, 1.55f, -2.85f, 1.45f);
        m_interactions.registerInteractable(
            "sunset_window", "West Courtyard Window", "[E] Look Out Sunset Window", "inspect_window",
            -4.10f, 1.55f, -1.15f, 1.35f);
        m_interactions.registerInteractable(
            "sunset_window_south", "South Courtyard Window", "[E] Look Out Courtyard Window", "inspect_window",
            -4.10f, 1.55f, 1.25f, 1.35f);
        m_interactions.registerInteractable(
            "classroom_door", "Hallway Sliding Door", "[E] Enter 3rd-Floor Hallway", "inspect_door_locked",
            4.15f, 1.45f, -1.65f, 1.40f);
    } else if (loc == LocationId::Hallway) {
        // Locker banks & hallway bench colliders
        m_collisionWorld.addBoxCollider(-2.2f, 1.1f, -2.85f, 3.4f, 2.25f, 0.60f, "LockerBank_West");
        m_collisionWorld.addBoxCollider( 2.2f, 1.1f, -2.85f, 3.4f, 2.25f, 0.60f, "LockerBank_East");
        m_collisionWorld.addBoxCollider( 0.0f, 0.4f,  1.65f, 1.8f, 0.75f, 0.65f, "CorridorBench");

        m_npcSam.registerCollider(m_collisionWorld);

        m_interactions.registerInteractable(
            "sam_npc", "Sam (Band Club)", "[E] Talk to Sam", "sam_hallway_talk",
            m_npcSam.getX(), 1.52f, m_npcSam.getZ(), 1.40f);
        m_interactions.registerInteractable(
            "hallway_lockers", "West Student Lockers", "[E] Inspect West Lockers", "inspect_lockers",
            -2.2f, 1.45f, -2.35f, 1.35f);
        m_interactions.registerInteractable(
            "hallway_lockers_east", "East Student Lockers", "[E] Inspect East Lockers", "inspect_lockers_east",
            2.2f, 1.45f, -2.35f, 1.35f);
        m_interactions.registerInteractable(
            "hallway_bench", "Corridor Waiting Bench", "[E] Inspect Corridor Bench", "inspect_hallway_bench",
            0.0f, 0.95f, 1.65f, 1.35f);
        m_interactions.registerInteractable(
            "hallway_noticeboard", "Campus Noticeboard", "[E] Read Band Night Poster", "inspect_noticeboard",
            0.0f, 1.55f, -2.85f, 1.35f);
        m_interactions.registerInteractable(
            "door_to_classroom", "Room 304 Door", "[E] Return to Room 304", "",
            -4.15f, 1.45f, -1.20f, 1.40f);
        m_interactions.registerInteractable(
            "stairs_to_street", "Stairwell Exit to Street", "[E] Go Down to Rainy Street", "inspect_stairs_locked",
            4.15f, 1.45f, -1.20f, 1.45f);
    } else if (loc == LocationId::RainyStreet) {
        // P. Paredes St. - FEU Institute of Technology & Canteen Stalls Colliders
        m_collisionWorld.addBoxCollider(-2.45f, 1.7f, -2.35f, 4.4f, 3.4f, 1.25f, "FEUTechBuilding");
        m_collisionWorld.addBoxCollider( 2.20f, 1.7f, -2.15f, 4.6f, 3.4f, 1.35f, "ParedesCanteenStalls");
        m_collisionWorld.addBoxCollider( 3.65f, 0.45f, -0.55f, 0.75f, 0.85f, 0.65f, "YellowCratesStack");

        // Position Maya on the P. Paredes sidewalk between FEU Tech and the food stalls
        m_npcMaya.setPositionAndDialogue(-0.35f, 0.0f, -0.95f, "maya_street_talk");
        m_npcMaya.registerCollider(m_collisionWorld);

        m_interactions.registerInteractable(
            "maya_street_npc", "Maya (Outside FEU Tech)", "[E] Talk to Maya", "maya_street_talk",
            m_npcMaya.getX(), 1.52f, m_npcMaya.getZ(), 1.40f);
        m_interactions.registerInteractable(
            "gate_to_hallway", "FEU Tech West Entrance Stairs", "[E] Enter FEU Tech Hallway", "",
            -1.95f, 1.45f, -1.45f, 1.45f);
        m_interactions.registerInteractable(
            "feu_tech_facade", "FEU Tech Autonomous Status Wall", "[E] Inspect FEU Tech Facade", "inspect_feu_facade",
            -3.55f, 1.45f, -1.40f, 1.45f);
        m_interactions.registerInteractable(
            "silver_shutter", "FEU Tech East Entrance Stairs", "[E] Inspect East Entrance Stairs", "inspect_silver_shutter",
            -0.85f, 1.45f, -1.45f, 1.35f);
        m_interactions.registerInteractable(
            "storefront_window", "Sidewalk Food Stall & Stools", "[E] Inspect Food Stall & Stools", "inspect_storefront",
            1.05f, 1.35f, -1.05f, 1.40f);
        m_interactions.registerInteractable(
            "guitar_bench", "Sam's Acoustic Guitar", "[E] Play Acoustic Song on P. Paredes", "maya_street_ready",
            2.15f, 1.05f, -0.75f, 1.45f);
        m_interactions.registerInteractable(
            "raloys_canteen", "Mang Rody's Eatery & Byte & Snacks", "[E] Inspect Mang Rody's Eatery", "inspect_raloys_canteen",
            3.05f, 1.45f, -1.15f, 1.45f);
        m_interactions.registerInteractable(
            "bus_stop_sign", "Tater Cube Kiosk & Stacked Crates", "[E] Inspect Tater Cube & Crates", "inspect_bus_sign",
            3.75f, 1.15f, -0.55f, 1.35f);
    }
}

void ClassroomScene::update(float dt, engine::camera::Camera3D& camera) {
    m_timeSeconds += dt;
    m_storyState.update(dt);
    m_transition.update(dt);

#ifdef _WIN32
    // Hardware key-combination check: [Ctrl + Shift + D] or [Ctrl + D] toggles Dev Mode
    const bool ctrlHeld = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    const bool dHeld    = (GetAsyncKeyState('D') & 0x8000) != 0;
    const bool chordNow = ctrlHeld && dHeld;
    if (chordNow && !m_ctrlDChordLatch && !m_transition.isActive()) {
        m_ctrlDChordLatch = true;
        toggleDevMode();
    } else if (!chordNow) {
        m_ctrlDChordLatch = false;
    }
#endif

    if (m_rhythmMinigame.isActive()) {
        m_rhythmMinigame.update(dt, m_storyState);
    } else if (m_dialogue.isActive()) {
        m_dialogue.update(dt);
    } else if (!m_phoneSystem.isOpen() && !m_devModeOpen && !m_transition.isActive() && !camera.isFreeCameraEnabled()) {
        m_player.update(dt, m_collisionWorld, true);
    }

    if (m_currentLocation == LocationId::Classroom || m_currentLocation == LocationId::RainyStreet) {
        m_npcMaya.update(dt, m_player.getX(), m_player.getZ());
    }
    if (m_currentLocation == LocationId::Hallway) {
        m_npcSam.update(dt, m_player.getX(), m_player.getZ());
    }

    m_interactions.update(m_player.getX(), m_player.getZ());

    // Keep gameplay camera locked in 1 fixed position so players cannot move it
    // while still seeing all objects in the scene from far away
    camera.resetToDefault25D();

    float bobVel = std::cos(m_timeSeconds * 3.0f) * 0.0015f;
    engine::core::Math4x4::translateVertexInMappedVBO(
        m_meshDynamicPyramid.getVBOId(), 0.0f, bobVel, 0.0f, 0);
}

void ClassroomScene::runPhase5VerificationStep(float dt, engine::camera::Camera3D& camera) {
    m_timeSeconds += dt;
    m_storyState.update(dt);
    m_transition.update(dt);

    if (m_verifyStep == 0) {
        // Location 1 (Classroom): unlock hallway and transition to Location 2 (Hallway)
        m_storyState.setFlag("has_sketchbook", 1);
        m_storyState.setFlag("talked_to_maya", 1);
        switchLocation(LocationId::Hallway, -3.2f, 0.4f, false);
        m_dialogue.startDialogue("sam_hallway_talk");
        m_verifyStep = 1;
    } else if (m_verifyStep == 1) {
        // Advance Sam's dialogue in the Hallway and pick Choice 0 ("street_unlocked")
        if (m_dialogue.isWaitingForChoice()) {
            m_dialogue.selectChoice(0, &m_storyState);
            m_verifyStep = 2;
        } else {
            m_dialogue.advanceOrCompleteLine(&m_storyState);
        }
    } else if (m_verifyStep == 2) {
        if (m_dialogue.isActive()) {
            m_dialogue.advanceOrCompleteLine(&m_storyState);
        } else {
            // Transition to Location 3 (Rainy Street / Sundown Mart)
            switchLocation(LocationId::RainyStreet, -2.5f, 0.8f, false);
            m_dialogue.startDialogue("maya_street_talk");
            m_verifyStep = 3;
        }
    } else if (m_verifyStep == 3) {
        if (m_dialogue.isWaitingForChoice()) {
            m_dialogue.selectChoice(0, &m_storyState);
            m_verifyStep = 4;
        } else if (m_dialogue.isActive()) {
            m_dialogue.advanceOrCompleteLine(&m_storyState);
        }
    }

    m_npcMaya.update(dt, m_player.getX(), m_player.getZ());
    m_npcSam.update(dt, m_player.getX(), m_player.getZ());
    camera.setNarrativeTarget(m_player.getX() * 0.58f, 1.20f, m_player.getZ() * 0.45f, dt);
}

void ClassroomScene::runPhase6VerificationStep(float dt, engine::camera::Camera3D& camera) {
    m_timeSeconds += dt;
    m_storyState.update(dt);
    m_transition.update(dt);

    if (m_verifyStep == 0) {
        // Switch to RainyStreet and start the Acoustic Guitar Rhythm Minigame
        switchLocation(LocationId::RainyStreet, 2.0f, 0.2f, false);
        m_rhythmMinigame.startSession();
        m_verifyStep = 1;
    } else if (m_verifyStep == 1) {
        // Auto-play the 16-note acoustic rhythm chart at accelerated step
        if (!m_rhythmMinigame.isFinished()) {
            for (int k = 0; k < 14 && !m_rhythmMinigame.isFinished(); ++k) {
                m_rhythmMinigame.autoPlayStep(0.08f, m_storyState);
            }
        } else {
            m_rhythmMinigame.stopSession();
            m_dialogue.startDialogue("post_minigame_celebration");
            m_verifyStep = 2;
        }
    } else if (m_verifyStep == 2) {
        if (m_dialogue.isActive()) {
            m_dialogue.advanceOrCompleteLine(&m_storyState);
        }
    }

    m_npcMaya.update(dt, m_player.getX(), m_player.getZ());
    camera.setNarrativeTarget(m_player.getX() * 0.58f, 1.20f, m_player.getZ() * 0.45f, dt);
}

void ClassroomScene::drawDeskWithChair3D(
    float x, float z, float yawDegrees,
    const engine::assets::Texture2D& woodTex)
{
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(yawDegrees, 0.0f, 1.0f, 0.0f);

    woodTex.bind();

    glPushMatrix();
    glTranslatef(0.0f, 0.74f, 0.0f);
    m_meshDeskTop.drawVBOElements();
    glPopMatrix();

    const float lx = 0.54f;
    const float lz = 0.27f;
    const float legCoords[4][2] = {
        { -lx, -lz }, {  lx, -lz },
        { -lx,  lz }, {  lx,  lz }
    };
    for (int i = 0; i < 4; ++i) {
        glPushMatrix();
        glTranslatef(legCoords[i][0], 0.36f, legCoords[i][1]);
        m_meshDeskLeg.drawVBOArrays();
        glPopMatrix();
    }

    glPushMatrix();
    glTranslatef(0.0f, 0.44f, 0.46f);
    m_meshChairSeat.drawVBOElements();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.68f, 0.66f);
    m_meshChairBack.drawVBOElements();
    glPopMatrix();

    // 4 Chair Legs / Feet reaching down to the floor (y: 0.00 .. 0.44)
    const float clx = 0.19f;
    const float clzFront = 0.28f;
    const float clzBack  = 0.64f;
    const float chairLegCoords[4][2] = {
        { -clx, clzFront }, {  clx, clzFront },
        { -clx, clzBack  }, {  clx, clzBack  }
    };
    for (int i = 0; i < 4; ++i) {
        glPushMatrix();
        glTranslatef(chairLegCoords[i][0], 0.22f, chairLegCoords[i][1]);
        m_meshChairLeg.drawVBOElements();
        glPopMatrix();
    }

    engine::assets::Texture2D::unbind();
    glPopMatrix();
}

void ClassroomScene::drawBench3D(
    float x, float z, float yawDegrees, float lengthScale,
    const engine::assets::Texture2D& woodTex)
{
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(yawDegrees, 0.0f, 1.0f, 0.0f);

    woodTex.bind();

    // 1. Bench Seat Plank (y = 0.44f)
    glPushMatrix();
    glTranslatef(0.0f, 0.44f, 0.0f);
    glScalef(lengthScale, 0.85f, 0.78f);
    m_meshDeskTop.drawVBOElements();
    glPopMatrix();

    // 2. Bench Backrest Plank (y = 0.76f, z = -0.23f)
    glPushMatrix();
    glTranslatef(0.0f, 0.76f, -0.23f);
    glScalef(lengthScale * 2.65f, 0.62f, 1.0f);
    m_meshChairBack.drawVBOElements();
    glPopMatrix();

    // 3. Bench Backrest Vertical Support Posts
    const float postX = 0.48f * lengthScale;
    const float postSides[2] = { -postX, postX };
    for (int i = 0; i < 2; ++i) {
        glPushMatrix();
        glTranslatef(postSides[i], 0.62f, -0.23f);
        glScalef(1.1f, 0.85f, 1.1f);
        m_meshChairLeg.drawVBOElements();
        glPopMatrix();
    }

    // 4. 6 Sturdy Bench Legs / Feet reaching the floor (y: 0.00 .. 0.44)
    const float blx = 0.50f * lengthScale;
    const float blz = 0.20f;
    const float benchLegCoords[6][2] = {
        { -blx, -blz }, { 0.0f, -blz }, {  blx, -blz },
        { -blx,  blz }, { 0.0f,  blz }, {  blx,  blz }
    };
    for (int i = 0; i < 6; ++i) {
        glPushMatrix();
        glTranslatef(benchLegCoords[i][0], 0.22f, benchLegCoords[i][1]);
        glScalef(1.25f, 1.0f, 1.25f);
        m_meshChairLeg.drawVBOElements();
        glPopMatrix();
    }

    engine::assets::Texture2D::unbind();
    glPopMatrix();
}

void ClassroomScene::renderClassroom3D(const engine::camera::Camera3D& camera) {
    const auto& floorTex = m_useCheckerTexture ? m_texCheckerDebug : m_texWoodFloor;
    const auto& wallTex  = m_useCheckerTexture ? m_texCheckerDebug : m_texClassroomWall;
    const auto& deskTex  = m_useCheckerTexture ? m_texCheckerDebug : m_texDeskWood;
    const auto& boardTex = m_useCheckerTexture ? m_texCheckerDebug : m_texChalkboard;

    floorTex.bind();
    glPushMatrix();
    m_meshFloor.drawVBOElements();
    glPopMatrix();

    wallTex.bind();
    glPushMatrix();
    glTranslatef(0.0f, 1.9f, -3.40f);
    m_meshBackWall.drawVBOArrays();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-4.75f, 1.9f, 0.0f);
    m_meshSideWall.drawClientSideElements();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(4.75f, 1.9f, 0.0f);
    m_meshSideWall.drawVBOArrays();
    glPopMatrix();

    boardTex.bind();
    glPushMatrix();
    glTranslatef(0.0f, 1.95f, -3.22f);
    m_meshChalkboard.drawVBOArrayElement();
    glPopMatrix();

    deskTex.bind();
    glPushMatrix();
    glTranslatef(4.58f, 1.18f, -1.65f);
    m_meshDoorPanel.drawVBOElements();
    glPopMatrix();
    engine::assets::Texture2D::unbind();

    drawDeskWithChair3D( 1.85f, -1.85f, 172.0f, deskTex);
    drawDeskWithChair3D(-1.85f,  0.05f,   0.0f, deskTex);
    drawDeskWithChair3D( 0.15f,  0.05f,   0.0f, deskTex);
    drawDeskWithChair3D( 2.15f,  0.05f,   0.0f, deskTex);
    drawDeskWithChair3D(-1.85f,  1.85f,   0.0f, deskTex);
    drawDeskWithChair3D( 0.15f,  1.85f,   0.0f, deskTex);
    drawDeskWithChair3D( 2.15f,  1.85f,   0.0f, deskTex);

    if (!m_storyState.hasFlag("has_sketchbook")) {
        glPushMatrix();
        glTranslatef(-1.72f, 0.81f, 0.08f);
        glRotatef(14.0f, 0.0f, 1.0f, 0.0f);
        m_texCheckerDebug.bind();
        m_meshSketchbookProp.drawVBOElements();
        engine::assets::Texture2D::unbind();
        glPopMatrix();
    }

    glPushMatrix();
    glTranslatef(0.15f, 0.80f, 0.05f);
    glRotatef(m_timeSeconds * 38.0f, 0.0f, 1.0f, 0.0f);
    m_meshDynamicPyramid.drawVBOArrays();
    glPopMatrix();

    engine::renderer::LightingSystem::applyMaterial(
        engine::renderer::LightingSystem::polishedCeramicMaterial());
    glPushMatrix();
    glTranslatef(1.65f, 0.91f, -1.80f);
    glRotatef(m_timeSeconds * 25.0f, 0.0f, 1.0f, 0.0f);
    glColor3f(0.95f, 0.82f, 0.48f);
    if (m_wireframeProps) glutWireTeapot(0.15); else glutSolidTeapot(0.15);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.10f, 0.92f, -1.88f);
    glColor3f(0.45f, 0.76f, 0.88f);
    if (m_wireframeProps) glutWireSphere(0.13, 16, 16); else glutSolidSphere(0.13, 16, 16);
    glPopMatrix();

    engine::renderer::LightingSystem::applyMaterial(
        engine::renderer::LightingSystem::glowingLampMaterial());
    glPushMatrix();
    glTranslatef(1.5f, 3.62f, -0.8f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 0.95f, 0.75f);
    glutSolidCone(0.22, 0.18, 16, 4);
    glPopMatrix();

    engine::renderer::LightingSystem::applyMaterial(
        engine::renderer::LightingSystem::matteWoodMaterial());
    if (m_npcMaya.getZ() < m_player.getZ()) {
        m_npcMaya.render(camera.getYawDegrees(), m_showDebugOverlay);
        m_player.render(camera.getYawDegrees(), m_showDebugOverlay);
    } else {
        m_player.render(camera.getYawDegrees(), m_showDebugOverlay);
        m_npcMaya.render(camera.getYawDegrees(), m_showDebugOverlay);
    }

    m_interactions.render3DIndicators(m_timeSeconds, m_showDebugOverlay);
    engine::renderer::PrimitiveRenderer::drawDustParticles(m_timeSeconds, 34);

    m_texWindowGlass.bind();
    glPushMatrix();
    glTranslatef(-4.60f, 2.05f, -1.25f);
    m_meshWindowPane.drawVBOElements();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-4.60f, 2.05f, 1.25f);
    m_meshWindowPane.drawVBOElements();
    glPopMatrix();
    engine::assets::Texture2D::unbind();
}

void ClassroomScene::renderHallway3D(const engine::camera::Camera3D& camera) {
    const auto& floorTex  = m_useCheckerTexture ? m_texCheckerDebug : m_texHallwayTile;
    const auto& wallTex   = m_useCheckerTexture ? m_texCheckerDebug : m_texClassroomWall;
    const auto& lockerTex = m_useCheckerTexture ? m_texCheckerDebug : m_texSchoolLockers;
    const auto& woodTex   = m_useCheckerTexture ? m_texCheckerDebug : m_texDeskWood;
    const auto& boardTex  = m_useCheckerTexture ? m_texCheckerDebug : m_texChalkboard;

    // 1. Terrazzo Corridor Floor
    floorTex.bind();
    glPushMatrix();
    m_meshFloor.drawVBOElements();
    glPopMatrix();

    // 2. Corridor Back & Side Walls
    wallTex.bind();
    glPushMatrix();
    glTranslatef(0.0f, 1.9f, -3.40f);
    m_meshBackWall.drawVBOArrays();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-4.75f, 1.9f, 0.0f);
    m_meshSideWall.drawVBOArrays();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(4.75f, 1.9f, 0.0f);
    m_meshSideWall.drawVBOArrays();
    glPopMatrix();

    // 3. Two Banks of Steel-Blue Student Lockers along Back Corridor Wall
    lockerTex.bind();
    glPushMatrix();
    glTranslatef(-2.25f, 1.12f, -2.95f);
    m_meshLockerBank.drawVBOElements();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.25f, 1.12f, -2.95f);
    m_meshLockerBank.drawVBOElements();
    glPopMatrix();

    // 4. Center Campus Noticeboard between locker banks
    boardTex.bind();
    glPushMatrix();
    glTranslatef(0.0f, 1.85f, -3.22f);
    glScalef(0.32f, 0.72f, 1.0f);
    m_meshChalkboard.drawVBOElements();
    glPopMatrix();

    // 5. Left Door (Back to Room 304) & Right Stairwell Door (Down to Rainy Street)
    woodTex.bind();
    glPushMatrix();
    glTranslatef(-4.58f, 1.18f, -1.20f);
    m_meshDoorPanel.drawVBOElements();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(4.58f, 1.18f, -1.20f);
    m_meshDoorPanel.drawVBOElements();
    glPopMatrix();
    engine::assets::Texture2D::unbind();

    // Corridor Wooden Waiting Bench with Legs & Backrest in Foreground
    drawBench3D(0.0f, 1.65f, 0.0f, 1.35f, woodTex);

    // Render Sam (NPC) & Elijah (Player) sorted by Z
    if (m_npcSam.getZ() < m_player.getZ()) {
        m_npcSam.render(camera.getYawDegrees(), m_showDebugOverlay);
        m_player.render(camera.getYawDegrees(), m_showDebugOverlay);
    } else {
        m_player.render(camera.getYawDegrees(), m_showDebugOverlay);
        m_npcSam.render(camera.getYawDegrees(), m_showDebugOverlay);
    }

    m_interactions.render3DIndicators(m_timeSeconds, m_showDebugOverlay);
    engine::renderer::PrimitiveRenderer::drawDustParticles(m_timeSeconds, 24);
}

void ClassroomScene::renderRainyStreet3D(const engine::camera::Camera3D& camera) {
    const auto& roadTex = m_useCheckerTexture ? m_texCheckerDebug : m_texWetAsphalt;
    const auto& tileTex = m_useCheckerTexture ? m_texCheckerDebug : m_texHallwayTile;
    const auto& woodTex = m_useCheckerTexture ? m_texCheckerDebug : m_texDeskWood;

    // Refined voxel block renderer: draws solid lit box + crisp subtle voxel edge wireframe
    // so architectural blocks, trim, and props read as detailed 2.5D voxel structures
    auto drawBox3D = [](float cx, float cy, float cz,
                        float sx, float sy, float sz,
                        float r, float g, float b,
                        bool drawVoxelEdge = true) {
        glPushMatrix();
        glTranslatef(cx, cy, cz);
        glScalef(sx, sy, sz);
        glColor3f(r, g, b);
        glutSolidCube(1.0);
        if (drawVoxelEdge) {
            glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glLineWidth(1.0f);
            glColor4f(r * 0.42f, g * 0.42f, b * 0.45f, 0.55f);
            glutWireCube(1.003);
            glPopAttrib();
        }
        glPopMatrix();
    };

    // Helper lambda to draw 3D world-space signage text on building facades
    auto drawSignText3D = [](float x, float y, float z,
                             float scale, float lineWidth,
                             const char* text,
                             float r, float g, float b) {
        glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glLineWidth(lineWidth);
        glColor4f(r, g, b, 1.0f);
        glPushMatrix();
        glTranslatef(x, y, z);
        glScalef(scale, scale, scale);
        glutStrokeString(GLUT_STROKE_ROMAN, reinterpret_cast<const unsigned char*>(text));
        glPopMatrix();
        glPopAttrib();
    };

    // =========================================================================
    // 1. P. PAREDES STREET ASPHALT ROAD & SEGMENTED CONCRETE SIDEWALK CURB
    // =========================================================================
    roadTex.bind();
    glPushMatrix();
    m_meshFloor.drawVBOElements();
    glPopMatrix();

    // Raised Light-Gray Concrete Sidewalk (z = -3.30 .. +0.35)
    tileTex.bind();
    glPushMatrix();
    glTranslatef(0.0f, 0.03f, -1.48f);
    glScalef(1.0f, 0.06f, 0.54f);
    glColor3f(0.76f, 0.77f, 0.79f);
    glutSolidCube(9.6);
    glPopMatrix();
    engine::assets::Texture2D::unbind();

    // Segmented Concrete Sidewalk Curb Stones along z = +0.35 (alternating tone blocks)
    for (int cb = 0; cb < 12; ++cb) {
        float cbx = -4.40f + static_cast<float>(cb) * 0.80f;
        float shade = (cb % 2 == 0) ? 0.64f : 0.58f;
        drawBox3D(cbx, 0.055f, 0.35f, 0.78f, 0.11f, 0.15f, shade, shade + 0.01f, shade + 0.03f);
    }

    // Street-level painted "P. PAREDES ST" road marking in the foreground lane
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(2.2f);
    glColor4f(0.88f, 0.90f, 0.92f, 0.65f);
    glPushMatrix();
    glTranslatef(1.35f, 0.02f, 2.35f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.0022f, 0.0028f, 0.0022f);
    glutStrokeString(GLUT_STROKE_ROMAN, reinterpret_cast<const unsigned char*>("P. PAREDES ST"));
    glPopMatrix();
    glPopAttrib();

    // =========================================================================
    // 2. LEFT SIDE: FEU INSTITUTE OF TECHNOLOGY BUILDING (937 P. PAREDES ST)
    // =========================================================================
    // 2a. Upper Multi-Story Architectural Concrete Facade with Pillar Ribs & Cornice
    drawBox3D(-2.45f, 2.88f, -2.55f, 4.45f, 2.45f, 0.88f, 0.60f, 0.58f, 0.55f);
    // Top roof cornice ledge & horizontal spandrel beam
    drawBox3D(-2.45f, 4.06f, -2.06f, 4.52f, 0.14f, 0.22f, 0.48f, 0.47f, 0.45f);
    drawBox3D(-2.45f, 2.74f, -2.06f, 4.50f, 0.12f, 0.18f, 0.50f, 0.49f, 0.47f);
    // Vertical architectural concrete pilasters separating window bays
    const float pilasterXs[4] = { -4.55f, -2.75f, -0.35f, -1.55f };
    for (int i = 0; i < 3; ++i) {
        drawBox3D(pilasterXs[i], 3.38f, -2.08f, 0.18f, 1.35f, 0.16f, 0.52f, 0.51f, 0.49f);
    }

    // Recessed Upper Floor Window Bays with Aluminum Mullions
    drawBox3D(-3.65f, 3.35f, -2.10f, 1.50f, 0.76f, 0.06f, 0.12f, 0.16f, 0.22f);
    drawBox3D(-3.65f, 3.35f, -2.07f, 0.05f, 0.76f, 0.05f, 0.42f, 0.44f, 0.48f); // Center mullion
    drawBox3D(-3.65f, 3.35f, -2.07f, 1.50f, 0.04f, 0.05f, 0.42f, 0.44f, 0.48f); // Transom bar

    drawBox3D(-1.55f, 3.35f, -2.10f, 1.92f, 0.76f, 0.06f, 0.12f, 0.16f, 0.22f);
    drawBox3D(-1.90f, 3.35f, -2.07f, 0.05f, 0.76f, 0.05f, 0.42f, 0.44f, 0.48f);
    drawBox3D(-1.20f, 3.35f, -2.07f, 0.05f, 0.76f, 0.05f, 0.42f, 0.44f, 0.48f);
    drawBox3D(-1.55f, 3.35f, -2.07f, 1.92f, 0.04f, 0.05f, 0.42f, 0.44f, 0.48f);

    // Balcony Concrete Planter Boxes + Multi-Voxel Cascading Green Foliage Clusters
    drawBox3D(-3.65f, 2.86f, -1.96f, 1.56f, 0.16f, 0.24f, 0.66f, 0.65f, 0.62f);
    drawBox3D(-1.55f, 2.86f, -1.96f, 1.98f, 0.16f, 0.24f, 0.66f, 0.65f, 0.62f);
    for (int b = 0; b < 6; ++b) {
        float bx1 = -4.28f + static_cast<float>(b) * 0.25f;
        float bh1 = (b % 2 == 0) ? 0.24f : 0.18f;
        drawBox3D(bx1, 2.98f, -1.92f, 0.22f, bh1, 0.24f,
                  (b % 2 == 0) ? 0.20f : 0.28f,
                  (b % 2 == 0) ? 0.48f : 0.56f,
                  0.20f);
    }
    for (int b = 0; b < 7; ++b) {
        float bx2 = -2.36f + static_cast<float>(b) * 0.27f;
        float bh2 = (b % 2 == 0) ? 0.25f : 0.19f;
        drawBox3D(bx2, 2.98f, -1.92f, 0.24f, bh2, 0.24f,
                  (b % 2 == 0) ? 0.22f : 0.30f,
                  (b % 2 == 0) ? 0.50f : 0.58f,
                  0.20f);
    }

    // 2b. Ground-Floor Polished Black Granite Columns & Twin Stairway Portals
    // Left wing black granite wall (x = -4.65 .. -2.38)
    drawBox3D(-3.52f, 1.25f, -2.20f, 2.28f, 2.50f, 0.55f, 0.10f, 0.11f, 0.13f);
    // Deep dark lobby interior behind BOTH the West & East Entrance Stairways (x = -2.38 .. -0.35)
    drawBox3D(-1.36f, 1.25f, -2.88f, 2.10f, 2.50f, 0.36f, 0.05f, 0.06f, 0.08f);
    // Center Polished Black Granite Divider Column between the two Entrance Stairways (x = -1.36)
    drawBox3D(-1.36f, 1.18f, -2.02f, 0.22f, 2.36f, 0.44f, 0.11f, 0.12f, 0.15f);
    // Far-right Polished Black Granite End Pillar (x = -0.34)
    drawBox3D(-0.34f, 1.25f, -2.02f, 0.22f, 2.50f, 0.48f, 0.11f, 0.12f, 0.15f);
    // Cylindrical security wall sconces mounted on the granite pillars
    drawBox3D(-1.36f, 1.52f, -1.78f, 0.07f, 0.18f, 0.07f, 0.46f, 0.48f, 0.52f);
    drawBox3D(-0.34f, 1.52f, -1.76f, 0.07f, 0.18f, 0.07f, 0.46f, 0.48f, 0.52f);

    // 2c. Far-Left "FEU TECH - AUTONOMOUS STATUS" Framed Green & Gold Banner + Louvers
    drawBox3D(-3.75f, 2.28f, -1.92f, 1.70f, 0.52f, 0.05f, 0.86f, 0.72f, 0.22f); // Gold outer frame
    drawBox3D(-3.75f, 2.28f, -1.90f, 1.64f, 0.46f, 0.06f, 0.05f, 0.38f, 0.22f); // FEU green face
    drawSignText3D(-4.35f, 2.34f, -1.86f, 0.0011f, 2.0f, "FEU TECH", 0.96f, 0.98f, 0.96f);
    drawSignText3D(-4.48f, 2.14f, -1.86f, 0.00078f, 1.8f, "AUTONOMOUS STATUS", 0.96f, 0.80f, 0.24f);

    // Dark horizontal window louvers + vertical mullions below the Autonomous Status banner
    drawBox3D(-3.75f, 1.52f, -1.93f, 1.62f, 0.82f, 0.04f, 0.07f, 0.08f, 0.10f);
    for (int lv = 0; lv < 6; ++lv) {
        float ly = 1.22f + static_cast<float>(lv) * 0.12f;
        drawBox3D(-3.75f, ly, -1.90f, 1.60f, 0.045f, 0.08f, 0.20f, 0.22f, 0.25f);
    }
    drawBox3D(-4.15f, 1.52f, -1.89f, 0.04f, 0.80f, 0.06f, 0.15f, 0.16f, 0.18f);
    drawBox3D(-3.35f, 1.52f, -1.89f, 0.04f, 0.80f, 0.06f, 0.15f, 0.16f, 0.18f);

    // Refined Multi-Voxel Hedge Planter & Voxel Palm Tree in front of Left Wing
    drawBox3D(-3.65f, 0.16f, -1.62f, 1.78f, 0.28f, 0.46f, 0.28f, 0.29f, 0.31f); // Dark stone planter Curb
    for (int h = 0; h < 7; ++h) {
        float hx = -4.36f + static_cast<float>(h) * 0.24f;
        float hy = (h % 2 == 0) ? 0.42f : 0.38f;
        float hs = (h % 3 == 0) ? 0.32f : 0.26f;
        drawBox3D(hx, hy, -1.60f, 0.25f, hs, 0.38f,
                  (h % 2 == 0) ? 0.16f : 0.22f,
                  (h % 2 == 0) ? 0.46f : 0.54f,
                  0.18f);
    }
    // Segmented Voxel Palm Tree Trunk (5 stacked wood rings) & Multi-Voxel Crown Fronds
    for (int tr = 0; tr < 5; ++tr) {
        float ty = 0.52f + static_cast<float>(tr) * 0.26f;
        float tx = -3.88f + ((tr % 2 == 0) ? 0.015f : -0.01f);
        drawBox3D(tx, ty, -1.52f, 0.13f, 0.24f, 0.13f, 0.42f, 0.30f, 0.19f);
    }
    // Layered voxel palm fronds radiating from top of trunk
    drawBox3D(-3.88f, 1.82f, -1.52f, 0.32f, 0.20f, 0.32f, 0.22f, 0.54f, 0.22f);
    drawBox3D(-4.22f, 1.76f, -1.52f, 0.38f, 0.14f, 0.24f, 0.18f, 0.48f, 0.18f);
    drawBox3D(-3.54f, 1.76f, -1.52f, 0.38f, 0.14f, 0.24f, 0.18f, 0.48f, 0.18f);
    drawBox3D(-3.88f, 1.76f, -1.22f, 0.24f, 0.14f, 0.36f, 0.20f, 0.50f, 0.20f);
    drawBox3D(-3.88f, 1.76f, -1.80f, 0.24f, 0.14f, 0.36f, 0.16f, 0.44f, 0.18f);
    drawBox3D(-4.42f, 1.66f, -1.52f, 0.20f, 0.12f, 0.20f, 0.15f, 0.42f, 0.16f);
    drawBox3D(-3.34f, 1.66f, -1.52f, 0.20f, 0.12f, 0.20f, 0.15f, 0.42f, 0.16f);

    // 2d. 3-Panel Cream Wall Display with Clean Green & Yellow Horizontal Stripes (Left of Stairs)
    drawBox3D(-2.58f, 1.05f, -1.93f, 0.68f, 0.94f, 0.04f, 0.22f, 0.24f, 0.26f); // Outer dark frame
    for (int p = 0; p < 3; ++p) {
        float px = -2.80f + static_cast<float>(p) * 0.22f;
        // Cream vertical panel backing
        drawBox3D(px, 1.05f, -1.91f, 0.19f, 0.88f, 0.04f, 0.95f, 0.96f, 0.92f);
        // Clean full-width green & yellow horizontal stripes across the bottom of each panel (no FEU text)
        drawBox3D(px, 0.76f, -1.895f, 0.19f, 0.10f, 0.04f, 0.08f, 0.46f, 0.22f, false); // Green band
        drawBox3D(px, 0.66f, -1.895f, 0.19f, 0.10f, 0.04f, 0.92f, 0.76f, 0.18f, false); // Yellow/Gold band
    }

    // 2e. TWIN RECESSED 6-STEP CONCRETE ENTRANCE STAIRWAYS (West Bay x=-1.86 & East Bay x=-0.85)
    // Both bays under the FEU Tech fascia are open staircases with step nosings & handrails!
    const float stairBayXs[2] = { -1.86f, -0.85f };
    for (int bay = 0; bay < 2; ++bay) {
        float bx = stairBayXs[bay];
        for (int st = 0; st < 6; ++st) {
            float stepY = 0.06f + static_cast<float>(st) * 0.085f;
            float stepZ = -1.74f - static_cast<float>(st) * 0.15f;
            // Concrete riser & tread
            drawBox3D(bx, stepY, stepZ, 0.78f, 0.08f, 0.16f, 0.74f, 0.73f, 0.70f);
            // Dark non-slip step nosing strip on front edge of each step
            drawBox3D(bx, stepY + 0.035f, stepZ + 0.07f, 0.76f, 0.02f, 0.025f, 0.32f, 0.33f, 0.35f, false);
        }
        // Top landing slab & framed lobby doorway inside each stair bay
        drawBox3D(bx, 0.56f, -2.68f, 0.78f, 0.08f, 0.28f, 0.68f, 0.67f, 0.64f);
        drawBox3D(bx, 1.32f, -2.78f, 0.56f, 1.45f, 0.06f, 0.18f, 0.24f, 0.28f); // Glass lobby door
        // Stainless-steel sloped handrails flanking each staircase
        drawBox3D(bx - 0.36f, 0.55f, -2.12f, 0.03f, 0.52f, 0.03f, 0.76f, 0.78f, 0.82f);
        drawBox3D(bx + 0.36f, 0.55f, -2.12f, 0.03f, 0.52f, 0.03f, 0.76f, 0.78f, 0.82f);
    }
    // Warm cream inner lobby side wall on the left of the West Stairway
    drawBox3D(-2.28f, 1.18f, -2.35f, 0.08f, 1.75f, 0.68f, 0.82f, 0.78f, 0.70f);

    // 2f. Iconic Cream-White Overhanging Fascia Sign: "FEU INSTITUTE OF TECHNOLOGY"
    // Panelled architectural cladding blocks with subtle vertical joint lines
    drawBox3D(-1.52f, 2.38f, -1.76f, 2.44f, 0.64f, 0.38f, 0.90f, 0.89f, 0.84f);
    drawBox3D(-1.52f, 2.05f, -1.76f, 2.46f, 0.05f, 0.40f, 0.22f, 0.23f, 0.25f); // Bottom soffit trim
    // Circular FEU seal emblem on left of fascia
    glPushMatrix();
    glTranslatef(-2.48f, 2.36f, -1.55f);
    glColor3f(0.06f, 0.38f, 0.20f);
    glutSolidSphere(0.14, 14, 14);
    glTranslatef(0.0f, 0.0f, 0.03f);
    glColor3f(0.92f, 0.78f, 0.22f);
    glutSolidSphere(0.09, 12, 12);
    glPopMatrix();

    // Bold 3D Green Lettering: "FEU" + "INSTITUTE OF TECHNOLOGY"
    drawSignText3D(-2.26f, 2.25f, -1.55f, 0.0019f, 3.0f, "FEU", 0.06f, 0.34f, 0.20f);
    drawSignText3D(-1.62f, 2.42f, -1.55f, 0.00095f, 2.2f, "INSTITUTE OF", 0.06f, 0.34f, 0.20f);
    drawSignText3D(-1.62f, 2.22f, -1.55f, 0.00095f, 2.2f, "TECHNOLOGY", 0.06f, 0.34f, 0.20f);

    // Potted multi-voxel palm plant right between FEU Tech black granite & the food stalls
    drawBox3D(-0.22f, 0.18f, -1.52f, 0.26f, 0.30f, 0.26f, 0.52f, 0.28f, 0.16f); // Terracotta pot
    drawBox3D(-0.22f, 0.34f, -1.52f, 0.30f, 0.06f, 0.30f, 0.58f, 0.32f, 0.18f); // Pot rim
    drawBox3D(-0.22f, 0.62f, -1.50f, 0.22f, 0.52f, 0.22f, 0.16f, 0.46f, 0.18f); // Center foliage
    drawBox3D(-0.34f, 0.72f, -1.48f, 0.22f, 0.36f, 0.24f, 0.22f, 0.54f, 0.22f); // Left frond cluster
    drawBox3D(-0.10f, 0.70f, -1.48f, 0.22f, 0.36f, 0.24f, 0.20f, 0.50f, 0.20f); // Right frond cluster

    // =========================================================================
    // 3. RIGHT SIDE: FICTIONAL STUDENT CANTEENS, BILLBOARDS & FOOD STALLS
    // =========================================================================
    // 3a. Concrete Utility Column & Electrical Meter Box between FEU Tech and Stalls
    drawBox3D(-0.02f, 1.55f, -1.95f, 0.28f, 3.00f, 0.55f, 0.68f, 0.67f, 0.63f);
    drawBox3D(-0.02f, 1.65f, -1.65f, 0.16f, 0.26f, 0.08f, 0.36f, 0.38f, 0.40f); // Meter box

    // 3b. Refined Corrugated Metal Canopy Awning (18 Ribbed Roof Panels + White Fascia + Steel Poles)
    for (int rb = 0; rb < 18; ++rb) {
        float rx = 0.14f + static_cast<float>(rb) * 0.245f;
        float rShade = (rb % 2 == 0) ? 0.78f : 0.68f;
        drawBox3D(rx, 1.98f, -1.38f, 0.24f, 0.08f, 1.18f, rShade, rShade + 0.01f, rShade);
    }
    // White front fascia board & dark steel support beam under canopy
    drawBox3D(2.25f, 1.92f, -0.79f, 4.46f, 0.15f, 0.06f, 0.92f, 0.92f, 0.90f);
    drawBox3D(2.25f, 1.84f, -0.84f, 4.44f, 0.05f, 0.06f, 0.26f, 0.28f, 0.30f);

    // 5 Vertical Steel Pipe Support Poles with concrete foot baseplates on sidewalk
    const float poleXs[5] = { 0.10f, 1.12f, 2.15f, 3.25f, 4.35f };
    for (int i = 0; i < 5; ++i) {
        drawBox3D(poleXs[i], 0.96f, -0.84f, 0.05f, 1.86f, 0.05f, 0.28f, 0.30f, 0.33f);
        drawBox3D(poleXs[i], 0.06f, -0.84f, 0.12f, 0.06f, 0.12f, 0.44f, 0.46f, 0.48f); // Baseplate
    }

    // 3c. Upper Left Above Canopy: Framed Beverage Promotion Billboard
    // ("FROST & BREW - ICED COFFEE & MILK TEA / BUY 1 TAKE 1 P49 ONLY")
    drawBox3D(1.12f, 2.88f, -1.68f, 2.16f, 1.62f, 0.08f, 0.24f, 0.26f, 0.28f); // Steel frame backing
    drawBox3D(1.12f, 2.88f, -1.64f, 2.08f, 1.54f, 0.06f, 0.11f, 0.23f, 0.52f); // Deep blue billboard face
    drawBox3D(1.12f, 2.30f, -1.60f, 2.02f, 0.34f, 0.04f, 0.94f, 0.80f, 0.18f); // Yellow promo ribbon
    drawSignText3D(0.20f, 3.20f, -1.58f, 0.00145f, 2.6f, "FROST & BREW", 0.98f, 0.84f, 0.20f);
    drawSignText3D(0.18f, 2.88f, -1.58f, 0.00072f, 2.0f, "ICED COFFEE & MILK TEA", 0.98f, 0.98f, 0.96f);
    drawSignText3D(0.22f, 2.62f, -1.58f, 0.00072f, 1.8f, "FRESH BREWED DAILY", 0.72f, 0.90f, 1.0f);
    drawSignText3D(0.20f, 2.24f, -1.56f, 0.00072f, 2.0f, "BUY 1 TAKE 1   P49 ONLY", 0.10f, 0.14f, 0.28f);

    // 3d. Upper Right Above Canopy: Weathered 2nd-Floor Timber Wall & "MANG RODY'S EATERY" Sign
    drawBox3D(3.38f, 2.88f, -1.85f, 2.20f, 1.58f, 0.45f, 0.46f, 0.55f, 0.66f);
    drawBox3D(3.38f, 3.68f, -1.72f, 2.28f, 0.10f, 0.58f, 0.34f, 0.36f, 0.40f); // Upper roof eave
    // 2 Detailed White-Framed Sliding Windows with Cross-Mullions & Dark Glass Panes
    const float winXs[2] = { 2.85f, 3.92f };
    for (int w = 0; w < 2; ++w) {
        float wx = winXs[w];
        drawBox3D(wx, 3.16f, -1.62f, 0.70f, 0.70f, 0.05f, 0.90f, 0.91f, 0.92f); // Outer white frame
        drawBox3D(wx, 3.16f, -1.60f, 0.58f, 0.58f, 0.05f, 0.16f, 0.20f, 0.26f); // Dark window glass
        drawBox3D(wx, 3.16f, -1.58f, 0.04f, 0.58f, 0.04f, 0.88f, 0.89f, 0.90f); // Vertical mullion
        drawBox3D(wx, 3.16f, -1.58f, 0.58f, 0.04f, 0.04f, 0.88f, 0.89f, 0.90f); // Horizontal bar
    }
    // Framed Red & White "MANG RODY'S EATERY" Signboard (Fictional Canteen Brand)
    drawBox3D(3.18f, 2.42f, -1.58f, 1.62f, 0.56f, 0.05f, 0.96f, 0.96f, 0.96f); // White outer border
    drawBox3D(3.18f, 2.42f, -1.56f, 1.54f, 0.48f, 0.06f, 0.82f, 0.16f, 0.18f); // Crimson signboard
    drawSignText3D(2.48f, 2.46f, -1.50f, 0.00078f, 2.0f, "MANG RODY'S", 0.99f, 0.99f, 0.99f);
    drawSignText3D(2.68f, 2.26f, -1.50f, 0.00075f, 1.9f, "EATERY", 0.98f, 0.86f, 0.28f);

    // 3e. Ground Floor Left Under Canopy: Detailed Sidewalk Food Stall, Jugs, Display Case & Stools
    drawBox3D(2.25f, 1.00f, -2.15f, 4.40f, 1.95f, 0.30f, 0.12f, 0.13f, 0.15f);
    // Stacked yellow-and-orange plastic delivery crates with dark hollow interiors (x = 0.24, z = -1.35)
    for (int c = 0; c < 4; ++c) {
        float cy = 0.11f + static_cast<float>(c) * 0.12f;
        drawBox3D(0.24f, cy, -1.35f, 0.42f, 0.10f, 0.36f,
                  (c % 2 == 0) ? 0.94f : 0.92f,
                  (c % 2 == 0) ? 0.76f : 0.46f,
                  0.12f);
        drawBox3D(0.24f, cy + 0.02f, -1.35f, 0.34f, 0.07f, 0.28f, 0.22f, 0.18f, 0.10f, false);
    }

    // Tiled Food Counter Base + Stainless Steel Countertop Lip (x = 0.60 .. 1.95)
    drawBox3D(1.25f, 0.40f, -1.52f, 1.32f, 0.72f, 0.46f, 0.54f, 0.56f, 0.58f);
    drawBox3D(1.25f, 0.79f, -1.50f, 1.38f, 0.06f, 0.52f, 0.82f, 0.84f, 0.86f); // Stainless top ledge
    drawBox3D(1.25f, 0.42f, -1.28f, 1.16f, 0.48f, 0.03f, 0.88f, 0.54f, 0.16f); // Warm orange front menu panel

    // 2 Detailed Blue Gulaman/Water Jugs with White Lids & Silver Spigots
    const float jugXs[2] = { 0.78f, 1.06f };
    for (int j = 0; j < 2; ++j) {
        float jx = jugXs[j];
        drawBox3D(jx, 0.98f, -1.48f, 0.22f, 0.32f, 0.22f, 0.14f, 0.46f, 0.88f); // Jug body
        drawBox3D(jx, 1.16f, -1.48f, 0.18f, 0.05f, 0.18f, 0.94f, 0.95f, 0.97f); // White top lid
        drawBox3D(jx, 0.88f, -1.35f, 0.04f, 0.04f, 0.06f, 0.80f, 0.82f, 0.85f); // Front tap spigot
    }

    // Framed Glass Food Display Case with Internal Shelves & Food Trays (x = 1.54)
    drawBox3D(1.54f, 1.05f, -1.48f, 0.54f, 0.46f, 0.34f, 0.72f, 0.82f, 0.90f); // Glass cabinet
    drawBox3D(1.54f, 1.29f, -1.48f, 0.56f, 0.04f, 0.36f, 0.84f, 0.86f, 0.88f); // Top metal cap
    drawBox3D(1.54f, 1.04f, -1.48f, 0.50f, 0.03f, 0.30f, 0.80f, 0.82f, 0.84f); // Middle shelf
    drawBox3D(1.42f, 0.92f, -1.46f, 0.18f, 0.06f, 0.20f, 0.88f, 0.62f, 0.24f); // Food tray 1
    drawBox3D(1.65f, 0.92f, -1.46f, 0.18f, 0.06f, 0.20f, 0.82f, 0.34f, 0.22f); // Food tray 2

    // 3 Detailed White 4-Legged Plastic Monobloc Stools in front of the food counter
    const float stoolXs[3] = { 0.55f, 1.08f, 1.62f };
    for (int s = 0; s < 3; ++s) {
        float sx = stoolXs[s];
        float sz = -1.06f;
        drawBox3D(sx, 0.42f, sz, 0.26f, 0.045f, 0.26f, 0.93f, 0.94f, 0.96f); // Stool seat
        drawBox3D(sx, 0.445f, sz, 0.08f, 0.01f, 0.04f, 0.35f, 0.36f, 0.40f, false); // Center hand slot
        drawBox3D(sx - 0.09f, 0.21f, sz - 0.09f, 0.035f, 0.38f, 0.035f, 0.88f, 0.89f, 0.91f);
        drawBox3D(sx + 0.09f, 0.21f, sz - 0.09f, 0.035f, 0.38f, 0.035f, 0.88f, 0.89f, 0.91f);
        drawBox3D(sx - 0.09f, 0.21f, sz + 0.09f, 0.035f, 0.38f, 0.035f, 0.88f, 0.89f, 0.91f);
        drawBox3D(sx + 0.09f, 0.21f, sz + 0.09f, 0.035f, 0.38f, 0.035f, 0.88f, 0.89f, 0.91f);
    }

    // 3f. Ground Floor Right Under Canopy: "BYTE & SNACKS" Turf Counters & "TATER CUBE" Kiosk
    // Framed white overhead "byte & snacks" sign banner (Fictional replacement for munch & chillz)
    drawBox3D(3.02f, 1.72f, -1.36f, 1.32f, 0.28f, 0.06f, 0.96f, 0.96f, 0.95f);
    drawSignText3D(2.46f, 1.68f, -1.31f, 0.00072f, 1.8f, "byte & snacks", 0.72f, 0.22f, 0.16f);

    // Two detailed green artificial-turf counters with wooden top ledges & condiment bottles
    drawBox3D(2.45f, 0.40f, -1.45f, 0.52f, 0.72f, 0.44f, 0.14f, 0.44f, 0.16f);
    drawBox3D(2.45f, 0.78f, -1.44f, 0.56f, 0.05f, 0.48f, 0.72f, 0.54f, 0.34f); // Wood counter top
    drawBox3D(2.38f, 0.86f, -1.38f, 0.05f, 0.12f, 0.05f, 0.86f, 0.18f, 0.16f); // Ketchup bottle
    drawBox3D(2.48f, 0.86f, -1.38f, 0.05f, 0.12f, 0.05f, 0.94f, 0.78f, 0.14f); // Cheese bottle

    drawBox3D(3.48f, 0.40f, -1.45f, 0.68f, 0.72f, 0.44f, 0.14f, 0.44f, 0.16f);
    drawBox3D(3.48f, 0.78f, -1.44f, 0.72f, 0.05f, 0.48f, 0.72f, 0.54f, 0.34f); // Wood counter top
    drawBox3D(3.38f, 0.88f, -1.42f, 0.38f, 0.14f, 0.26f, 0.76f, 0.78f, 0.80f); // Stainless griddle

    // Yellow interior range hood visible inside Byte & Snacks
    drawBox3D(3.05f, 1.32f, -1.82f, 0.82f, 0.24f, 0.30f, 0.92f, 0.78f, 0.18f);

    // Detailed Yellow & Green "TATER CUBE" Fries Kiosk on the far right (x = 4.12)
    drawBox3D(4.12f, 0.66f, -1.52f, 0.48f, 1.24f, 0.44f, 0.92f, 0.82f, 0.18f);
    drawBox3D(4.12f, 0.86f, -1.30f, 0.36f, 0.38f, 0.04f, 0.16f, 0.20f, 0.24f); // Kiosk service window
    drawBox3D(4.12f, 1.16f, -1.29f, 0.46f, 0.24f, 0.05f, 0.16f, 0.52f, 0.22f); // Green header sign
    drawSignText3D(3.93f, 1.13f, -1.25f, 0.00042f, 1.4f, "TATER CUBE", 0.98f, 0.92f, 0.22f);

    // Foreground stack of hollow yellow-and-orange plastic crates on the right sidewalk
    for (int c = 0; c < 5; ++c) {
        float cy = 0.11f + static_cast<float>(c) * 0.11f;
        drawBox3D(3.68f, cy, -0.55f, 0.56f, 0.09f, 0.46f,
                  (c % 2 == 0) ? 0.95f : 0.92f,
                  (c % 2 == 0) ? 0.78f : 0.46f,
                  0.14f);
        drawBox3D(3.68f, cy + 0.02f, -0.55f, 0.46f, 0.06f, 0.36f, 0.22f, 0.18f, 0.10f, false);
    }

    // Sam's Acoustic Guitar Case Prop resting beside the sidewalk bench/stools at x = 2.15, z = -0.75
    woodTex.bind();
    glPushMatrix();
    glTranslatef(2.15f, 0.22f, -0.75f);
    glRotatef(18.0f, 0.0f, 1.0f, 0.0f);
    glScalef(1.6f, 1.3f, 1.2f);
    m_meshSketchbookProp.drawVBOElements();
    glPopMatrix();
    engine::assets::Texture2D::unbind();

    // 3g. Overhead Street Utility Wires across P. Paredes St. (Module 2 GL_LINE_STRIP)
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.6f);
    glColor4f(0.12f, 0.13f, 0.15f, 0.85f);
    for (int w = 0; w < 3; ++w) {
        float wy = 2.95f + static_cast<float>(w) * 0.14f;
        glBegin(GL_LINE_STRIP);
        for (int seg = 0; seg <= 16; ++seg) {
            float t = static_cast<float>(seg) / 16.0f;
            float wx = -4.6f + t * 9.2f;
            float sag = std::sin(t * 3.14159265f) * 0.18f;
            glVertex3f(wx, wy - sag, -1.15f + static_cast<float>(w) * 0.12f);
        }
        glEnd();
    }
    glPopAttrib();

    // =========================================================================
    // 4. RENDER MAYA & ELIJAH ON P. PAREDES SIDEWALK (DEPTH-SORTED BY Z)
    // =========================================================================
    engine::renderer::LightingSystem::applyMaterial(
        engine::renderer::LightingSystem::matteWoodMaterial());
    if (m_npcMaya.getZ() < m_player.getZ()) {
        m_npcMaya.render(camera.getYawDegrees(), m_showDebugOverlay);
        m_player.render(camera.getYawDegrees(), m_showDebugOverlay);
    } else {
        m_player.render(camera.getYawDegrees(), m_showDebugOverlay);
        m_npcMaya.render(camera.getYawDegrees(), m_showDebugOverlay);
    }

    m_interactions.render3DIndicators(m_timeSeconds, m_showDebugOverlay);

    // Subtle atmospheric street particles
    engine::renderer::PrimitiveRenderer::drawDustParticles(m_timeSeconds, 28);
}

void ClassroomScene::render3D(const engine::camera::Camera3D& camera) {
    camera.applyProjection();
    camera.applyView();

    m_lighting.applyLights(m_timeSeconds);
    engine::renderer::LightingSystem::applyMaterial(
        engine::renderer::LightingSystem::matteWoodMaterial());

    if (m_currentLocation == LocationId::Classroom) {
        renderClassroom3D(camera);
    } else if (m_currentLocation == LocationId::Hallway) {
        renderHallway3D(camera);
    } else {
        renderRainyStreet3D(camera);
    }

    if (m_showDebugOverlay) {
        engine::renderer::PrimitiveRenderer::drawReferenceGrid(4.5f, 0.75f);
        engine::renderer::PrimitiveRenderer::drawCoordinateAxes(1.5f);
        m_collisionWorld.drawDebugColliders(m_player.getLastHitColliderTag());
        engine::renderer::PrimitiveRenderer::drawModule2PrimitiveShowcase(0.0f, 3.05f, -3.10f);
    }
}

void ClassroomScene::render2D(const engine::camera::Camera3D& camera) {
    const int w = camera.getViewportWidth();
    const int h = camera.getViewportHeight();

    engine::camera::Camera3D::begin2DOverlay(w, h);

    // -------------------------------------------------------------------------
    // 1. Top-Left HTML-Styled Scene & Status Card (dynamically wrapped to text)
    // -------------------------------------------------------------------------
    const std::string locTitle = getCurrentLocationTitle();
    const int titleW = engine::ui::TextRenderer::measureBitmapTextWidth(
        locTitle, GLUT_BITMAP_HELVETICA_12);

    std::string statusText;
    const auto* nearObj = m_interactions.getActiveInteractable();
    if (m_storyState.hasActiveToast()) {
        statusText = m_storyState.getToastMessage();
    } else if (nearObj) {
        statusText = "Nearby: " + nearObj->displayName + "   [Press E to Interact]";
    }

    float maxLeftCardW = static_cast<float>(w) * 0.46f;
    if (maxLeftCardW < 340.0f) maxLeftCardW = 340.0f;

    auto statusLayout = engine::ui::TextRenderer::computeWrappedLayout(
        statusText, maxLeftCardW - 44.0f, 16.0f, GLUT_BITMAP_HELVETICA_12);

    float cardW = static_cast<float>(titleW) + (m_devModeUnlocked ? 168.0f : 98.0f);
    if (!statusText.empty()) {
        cardW = std::max(cardW, statusLayout.maxLineWidth + 52.0f);
    } else {
        cardW = std::max(cardW, 386.0f);
    }
    if (cardW > maxLeftCardW) cardW = maxLeftCardW;

    float statusAreaH = (!statusText.empty())
        ? std::max(26.0f, statusLayout.totalHeight + 12.0f)
        : 24.0f;
    float cardH = 48.0f + statusAreaH + 12.0f;
    float cardX = 16.0f;
    float cardY = static_cast<float>(h) - cardH - 14.0f;

    engine::ui::TextRenderer::drawRoundedRect2D(
        cardX, cardY, cardW, cardH, 14.0f,
        0.05f, 0.08f, 0.15f, 0.92f,
        0.44f, 0.54f, 0.68f, 0.45f,
        1.3f, true);

    // Inner header bar dynamically sized to cardW
    engine::ui::TextRenderer::drawRoundedRect2D(
        cardX + 8.0f, cardY + cardH - 38.0f, cardW - 16.0f, 30.0f, 9.0f,
        0.10f, 0.15f, 0.26f, 0.94f,
        0.42f, 0.56f, 0.76f, 0.32f,
        1.0f, false);

    engine::ui::TextRenderer::drawPillBadge2D(
        cardX + 14.0f, cardY + cardH - 33.0f, 52.0f, 20.0f,
        "SCENE",
        0.18f, 0.36f, 0.62f, 0.96f,
        0.46f, 0.68f, 0.94f,
        0.92f, 0.97f, 1.0f,
        GLUT_BITMAP_HELVETICA_10);

    engine::ui::TextRenderer::drawBitmapText2D(
        cardX + 76.0f, cardY + cardH - 28.0f,
        locTitle,
        GLUT_BITMAP_HELVETICA_12,
        0.92f, 0.96f, 1.0f);

    if (m_devModeUnlocked) {
        engine::ui::TextRenderer::drawPillBadge2D(
            cardX + cardW - 76.0f, cardY + cardH - 33.0f, 62.0f, 20.0f,
            "DEV [~]",
            0.10f, 0.38f, 0.30f, 0.96f,
            0.32f, 0.88f, 0.68f,
            0.85f, 1.0f, 0.94f,
            GLUT_BITMAP_HELVETICA_10);
    }

    // Status / Toast / Nearby line dynamically wrapped inside its pill (no gold outline)
    if (m_storyState.hasActiveToast()) {
        float pillW = std::min(cardW - 20.0f, statusLayout.maxLineWidth + 32.0f);
        engine::ui::TextRenderer::drawRoundedRect2D(
            cardX + 10.0f, cardY + 10.0f, pillW, statusAreaH, 11.0f,
            0.10f, 0.32f, 0.26f, 0.95f,
            0.32f, 0.82f, 0.64f, 0.75f,
            1.2f, false);
        float ty = cardY + 10.0f + statusAreaH - 16.0f;
        for (const auto& line : statusLayout.lines) {
            engine::ui::TextRenderer::drawBitmapText2D(
                cardX + 24.0f, ty, line, GLUT_BITMAP_HELVETICA_12,
                0.85f, 1.0f, 0.92f);
            ty -= 16.0f;
        }
    } else if (nearObj) {
        float pillW = std::min(cardW - 20.0f, statusLayout.maxLineWidth + 32.0f);
        engine::ui::TextRenderer::drawRoundedRect2D(
            cardX + 10.0f, cardY + 10.0f, pillW, statusAreaH, 11.0f,
            0.13f, 0.22f, 0.38f, 0.95f,
            0.46f, 0.70f, 0.96f, 0.72f,
            1.2f, false);
        float ty = cardY + 10.0f + statusAreaH - 16.0f;
        for (const auto& line : statusLayout.lines) {
            engine::ui::TextRenderer::drawBitmapText2D(
                cardX + 24.0f, ty, line, GLUT_BITMAP_HELVETICA_12,
                0.90f, 0.96f, 1.0f);
            ty -= 16.0f;
        }
    } else {
        bool hasBook = m_storyState.hasFlag("has_sketchbook");
        bool metMaya = m_storyState.hasFlag("talked_to_maya");
        bool metSam  = m_storyState.hasFlag("talked_to_sam");

        std::string b1 = std::string("Sketchbook: ") + (hasBook ? "Found" : "No");
        std::string b2 = std::string("Maya: ") + (metMaya ? "Spoken" : "Pending");
        std::string b3 = std::string("Sam: ") + (metSam ? "Spoken" : "Pending");

        float w1 = static_cast<float>(engine::ui::TextRenderer::measureBitmapTextWidth(b1, GLUT_BITMAP_HELVETICA_12)) + 24.0f;
        float w2 = static_cast<float>(engine::ui::TextRenderer::measureBitmapTextWidth(b2, GLUT_BITMAP_HELVETICA_12)) + 24.0f;
        float w3 = static_cast<float>(engine::ui::TextRenderer::measureBitmapTextWidth(b3, GLUT_BITMAP_HELVETICA_12)) + 24.0f;

        float px = cardX + 10.0f;
        engine::ui::TextRenderer::drawPillBadge2D(
            px, cardY + 10.0f, w1, 22.0f, b1,
            hasBook ? 0.12f : 0.10f, hasBook ? 0.30f : 0.14f, hasBook ? 0.24f : 0.22f, 0.92f,
            hasBook ? 0.34f : 0.38f, hasBook ? 0.78f : 0.46f, hasBook ? 0.58f : 0.58f,
            hasBook ? 0.82f : 0.78f, hasBook ? 0.98f : 0.84f, hasBook ? 0.88f : 0.92f,
            GLUT_BITMAP_HELVETICA_12);
        px += w1 + 8.0f;

        engine::ui::TextRenderer::drawPillBadge2D(
            px, cardY + 10.0f, w2, 22.0f, b2,
            metMaya ? 0.12f : 0.10f, metMaya ? 0.30f : 0.14f, metMaya ? 0.24f : 0.22f, 0.92f,
            metMaya ? 0.34f : 0.38f, metMaya ? 0.78f : 0.46f, metMaya ? 0.58f : 0.58f,
            metMaya ? 0.82f : 0.78f, metMaya ? 0.98f : 0.84f, metMaya ? 0.88f : 0.92f,
            GLUT_BITMAP_HELVETICA_12);
        px += w2 + 8.0f;

        engine::ui::TextRenderer::drawPillBadge2D(
            px, cardY + 10.0f, w3, 22.0f, b3,
            metSam ? 0.12f : 0.10f, metSam ? 0.30f : 0.14f, metSam ? 0.24f : 0.22f, 0.92f,
            metSam ? 0.34f : 0.38f, metSam ? 0.78f : 0.46f, metSam ? 0.58f : 0.58f,
            metSam ? 0.82f : 0.78f, metSam ? 0.98f : 0.84f, metSam ? 0.88f : 0.92f,
            GLUT_BITMAP_HELVETICA_12);
    }

    // -------------------------------------------------------------------------
    // 2. Right-Side Stage-Specific TODO Task Card (shows ONLY current stage task)
    // -------------------------------------------------------------------------
    if (!m_phoneSystem.isOpen() && !m_rhythmMinigame.isActive()) {
        const bool hasBook   = m_storyState.hasFlag("has_sketchbook");
        const bool metMaya   = m_storyState.hasFlag("talked_to_maya");
        const bool replied   = m_storyState.hasFlag("replied_phone_sam");
        const bool metSam    = m_storyState.hasFlag("talked_to_sam");
        const bool playedGtr = m_storyState.hasFlag("completed_rhythm_minigame");

        std::string stageTitle = "STAGE 1 - ROOM 304";
        std::string activeTaskText;
        bool stageDone = false;

        if (!hasBook) {
            stageTitle     = "STAGE 1 - ROOM 304";
            activeTaskText = "[ ] Inspect your Sketchbook on the front-left desk.";
        } else if (!metMaya) {
            stageTitle     = "STAGE 1 - ROOM 304";
            activeTaskText = "[ ] Talk to Maya by the sunset window.";
        } else if (m_currentLocation == LocationId::Classroom && !metSam) {
            stageTitle     = "STAGE 1 - ROOM 304";
            activeTaskText = replied
                ? "[ ] Exit Room 304 through the right door into the Hallway."
                : "[ ] Reply to Sam on Phone [Tab], then exit into the Hallway.";
        } else if (!metSam) {
            stageTitle     = "STAGE 2 - HALLWAY";
            activeTaskText = "[ ] Talk to Sam by the 3rd-floor hallway lockers.";
        } else if (m_currentLocation != LocationId::RainyStreet && !playedGtr) {
            stageTitle     = "STAGE 2 - HALLWAY";
            activeTaskText = "[ ] Take the right stairwell down to P. Paredes St.";
        } else if (!playedGtr) {
            stageTitle     = "STAGE 3 - P. PAREDES ST";
            activeTaskText = "[ ] Play the acoustic guitar song outside FEU Tech & Canteens.";
        } else {
            stageTitle     = "ALL STAGES COMPLETE";
            activeTaskText = "[x] Story Complete! Free explore or replay guitar anytime.";
            stageDone      = true;
        }

        const float maxTodoTextW = 250.0f;
        auto taskLayout = engine::ui::TextRenderer::computeWrappedLayout(
            activeTaskText, maxTodoTextW, 16.0f, GLUT_BITMAP_HELVETICA_12);
        int headerTitleW = engine::ui::TextRenderer::measureBitmapTextWidth(
            stageTitle, GLUT_BITMAP_HELVETICA_12);

        float contentW = std::max(taskLayout.maxLineWidth, static_cast<float>(headerTitleW) + 64.0f);
        if (contentW < 190.0f) contentW = 190.0f;

        float todoW    = contentW + 32.0f;
        float taskBoxH = taskLayout.totalHeight + 14.0f;
        float todoH    = 46.0f + taskBoxH + 10.0f;
        float todoX    = static_cast<float>(w) - todoW - 16.0f;
        float todoY    = static_cast<float>(h) - 216.0f - todoH;

        engine::ui::TextRenderer::drawRoundedRect2D(
            todoX, todoY, todoW, todoH, 14.0f,
            0.05f, 0.08f, 0.15f, 0.92f,
            0.44f, 0.54f, 0.68f, 0.45f,
            1.3f, true);

        // Header bar inside TODO card (sleek slate-blue, no gold outline)
        engine::ui::TextRenderer::drawRoundedRect2D(
            todoX + 8.0f, todoY + todoH - 36.0f, todoW - 16.0f, 28.0f, 8.0f,
            0.10f, 0.15f, 0.26f, 0.94f,
            0.42f, 0.56f, 0.76f, 0.32f,
            1.0f, false);

        engine::ui::TextRenderer::drawPillBadge2D(
            todoX + 14.0f, todoY + todoH - 31.0f, 48.0f, 18.0f,
            "TODO",
            0.18f, 0.36f, 0.62f, 0.96f,
            0.46f, 0.68f, 0.94f,
            0.92f, 0.97f, 1.0f,
            GLUT_BITMAP_HELVETICA_10);

        engine::ui::TextRenderer::drawBitmapText2D(
            todoX + 70.0f, todoY + todoH - 26.0f,
            stageTitle,
            GLUT_BITMAP_HELVETICA_12,
            0.92f, 0.96f, 1.0f);

        // Wrapped Stage Task box
        float taskBoxY = todoY + 10.0f;
        engine::ui::TextRenderer::drawRoundedRect2D(
            todoX + 10.0f, taskBoxY, todoW - 20.0f, taskBoxH, 8.0f,
            stageDone ? 0.10f : 0.11f,
            stageDone ? 0.28f : 0.19f,
            stageDone ? 0.22f : 0.32f,
            0.92f,
            stageDone ? 0.34f : 0.40f,
            stageDone ? 0.78f : 0.64f,
            stageDone ? 0.58f : 0.90f,
            0.45f,
            1.0f, false);

        float ty = taskBoxY + taskBoxH - 16.0f;
        for (const auto& line : taskLayout.lines) {
            engine::ui::TextRenderer::drawBitmapText2D(
                todoX + 18.0f, ty, line, GLUT_BITMAP_HELVETICA_12,
                stageDone ? 0.78f : 0.90f,
                stageDone ? 0.98f : 0.96f,
                stageDone ? 0.86f : 1.0f);
            ty -= 16.0f;
        }
    }

    // -------------------------------------------------------------------------
    // 3. Top-Right Smartphone Notification Pill & Overlay
    // -------------------------------------------------------------------------
    m_phoneSystem.render2D(w, h, m_storyState);

    // -------------------------------------------------------------------------
    // 4. Active Overlay (Dev Mode / Minigame / Dialogue / Interaction + Dock)
    // -------------------------------------------------------------------------
    if (m_devModeOpen) {
        renderDevModeOverlay2D(w, h);
    } else if (m_rhythmMinigame.isActive()) {
        m_rhythmMinigame.render2D(w, h);
    } else if (m_dialogue.isActive()) {
        m_dialogue.render2D(w, h);
    } else {
        if (!m_phoneSystem.isOpen()) {
            m_interactions.render2DPrompt(w, h);
        }

        // Sleek HTML-style floating pill dock dynamically sized to controls text
        const std::string ctrlText = m_devModeUnlocked
            ? "[WASD / Arrows] Walk  |  [E] Interact  |  [Tab] Phone  |  [~ / Ctrl+D] Dev Console  |  [F5 / F9] Save / Load  |  [ESC] Pause"
            : "[WASD / Arrows] Walk  |  [E] Interact  |  [Tab] Phone  |  [F5 / F9] Save / Load  |  [Ctrl+Shift+D / DEV] Dev Mode  |  [ESC] Pause";
        float barW = static_cast<float>(engine::ui::TextRenderer::measureBitmapTextWidth(
            ctrlText, GLUT_BITMAP_HELVETICA_12)) + 44.0f;
        if (barW > static_cast<float>(w) - 32.0f) {
            barW = static_cast<float>(w) - 32.0f;
        }
        float barH = 30.0f;
        float barX = (static_cast<float>(w) - barW) * 0.5f;
        float barY = 10.0f;

        engine::ui::TextRenderer::drawRoundedRect2D(
            barX, barY, barW, barH, 15.0f,
            0.05f, 0.08f, 0.15f, 0.90f,
            0.42f, 0.52f, 0.66f, 0.42f,
            1.2f, true);

        engine::ui::TextRenderer::drawBitmapTextCentered2D(
            static_cast<float>(w) * 0.5f, barY + 10.0f,
            ctrlText,
            GLUT_BITMAP_HELVETICA_12,
            0.82f, 0.90f, 0.98f);
    }

    m_transition.render2D(w, h);
    engine::camera::Camera3D::end2DOverlay();
}

void ClassroomScene::toggleDevMode() {
    if (!m_devModeUnlocked) {
        m_devModeUnlocked = true;
        m_devModeOpen     = true;
        m_devMenuCursor   = 0;
        m_phoneSystem.setOpen(false);
        engine::audio::AudioSystem::play(engine::audio::SoundEvent::SaveChime);
        m_storyState.showToast("DEV MODE ACTIVATED! Teleport Scenes [1-3] or Bypass Tasks [4-5]", 4.0f);
    } else {
        m_devModeOpen = !m_devModeOpen;
        if (m_devModeOpen) {
            m_phoneSystem.setOpen(false);
        }
        engine::audio::AudioSystem::play(engine::audio::SoundEvent::UIBlip);
    }
}

void ClassroomScene::bypassCurrentTask() {
    if (!m_storyState.hasFlag("has_sketchbook")) {
        m_storyState.setFlag("has_sketchbook", 1);
        m_storyState.showToast("Dev Bypass: Picked up Elijah's Sketchbook (Stage 1)", 3.5f);
    } else if (!m_storyState.hasFlag("talked_to_maya")) {
        m_storyState.setFlag("talked_to_maya", 1);
        m_storyState.setFlag("promised_rooftop_rehearsal", 1);
        m_storyState.setFlag("hallway_unlocked", 1);
        m_storyState.showToast("Dev Bypass: Completed Maya Talk & Unlocked Hallway (Stage 1)", 3.5f);
    } else if (!m_storyState.hasFlag("replied_phone_sam")) {
        m_phoneSystem.autoCompleteAllReplies(m_storyState);
        m_storyState.setFlag("hallway_unlocked", 1);
        m_storyState.showToast("Dev Bypass: Replied to Sam & Maya on Smartphone (Stage 1)", 3.5f);
    } else if (!m_storyState.hasFlag("talked_to_sam")) {
        m_storyState.setFlag("talked_to_sam", 1);
        m_storyState.setFlag("street_unlocked", 1);
        m_storyState.showToast("Dev Bypass: Completed Sam Talk & Unlocked P. Paredes St (Stage 2)", 3.5f);
    } else if (!m_storyState.hasFlag("completed_rhythm_minigame")) {
        m_storyState.setFlag("completed_rhythm_minigame", 1);
        m_storyState.setFlag("story_act1_complete", 1);
        m_storyState.showToast("Dev Bypass: Completed Acoustic Guitar Song (Stage 3)", 3.5f);
    } else {
        m_storyState.showToast("Dev Mode: All Stage Tasks are already complete!", 3.0f);
    }
    engine::audio::AudioSystem::play(engine::audio::SoundEvent::SaveChime);
}

void ClassroomScene::bypassAllTasks() {
    m_storyState.setFlag("has_sketchbook", 1);
    m_storyState.setFlag("talked_to_maya", 1);
    m_storyState.setFlag("promised_rooftop_rehearsal", 1);
    m_storyState.setFlag("hallway_unlocked", 1);
    m_storyState.setFlag("talked_to_sam", 1);
    m_storyState.setFlag("street_unlocked", 1);
    m_storyState.setFlag("completed_rhythm_minigame", 1);
    m_storyState.setFlag("story_act1_complete", 1);
    m_phoneSystem.autoCompleteAllReplies(m_storyState);
    engine::audio::AudioSystem::play(engine::audio::SoundEvent::SaveChime);
    m_storyState.showToast("Dev Bypass: ALL Tasks Completed & All Scenes Unlocked!", 4.0f);
}

void ClassroomScene::executeDevAction(int actionIndex) {
    switch (actionIndex) {
        case 0: // [1] Teleport -> Room 304 Classroom
            m_devModeOpen = false;
            m_dialogue.closeDialogue();
            engine::audio::AudioSystem::play(engine::audio::SoundEvent::DialogueSelect);
            m_storyState.showToast("Dev Teleport: Room 304 - FEU Tech Lab", 3.0f);
            switchLocation(LocationId::Classroom, -0.35f, 1.15f, false);
            break;
        case 1: // [2] Teleport -> 3rd-Floor Hallway
            m_devModeOpen = false;
            m_dialogue.closeDialogue();
            m_storyState.setFlag("hallway_unlocked", 1);
            engine::audio::AudioSystem::play(engine::audio::SoundEvent::DialogueSelect);
            m_storyState.showToast("Dev Teleport: 3rd-Floor Campus Hallway", 3.0f);
            switchLocation(LocationId::Hallway, -3.2f, 0.3f, false);
            break;
        case 2: // [3] Teleport -> 937 P. Paredes St (FEU Tech & Canteens)
            m_devModeOpen = false;
            m_dialogue.closeDialogue();
            m_storyState.setFlag("hallway_unlocked", 1);
            m_storyState.setFlag("street_unlocked", 1);
            engine::audio::AudioSystem::play(engine::audio::SoundEvent::DialogueSelect);
            m_storyState.showToast("Dev Teleport: 937 P. Paredes St - FEU Tech & Canteens", 3.0f);
            switchLocation(LocationId::RainyStreet, -2.2f, 0.95f, false);
            break;
        case 3: // [4] Bypass Current Stage Task
            bypassCurrentTask();
            break;
        case 4: // [5] Bypass ALL Tasks & Unlock Everything
            bypassAllTasks();
            break;
        case 5: // [6] Reset All Tasks to Stage 1
            m_storyState.resetNewGame();
            m_phoneSystem.loadFromExternalData("data/dialogue/phone_messages.txt");
            engine::audio::AudioSystem::play(engine::audio::SoundEvent::UIBlip);
            m_storyState.showToast("Dev Mode: Reset All Story Tasks & Flags to Stage 1", 3.5f);
            break;
        case 6: // [7] Launch Acoustic Guitar Rhythm Minigame
            m_devModeOpen = false;
            m_dialogue.closeDialogue();
            engine::audio::AudioSystem::play(engine::audio::SoundEvent::DialogueSelect);
            m_rhythmMinigame.startSession();
            break;
        case 7: // [8] Toggle 3D Colliders & Debug Axes
            m_showDebugOverlay = !m_showDebugOverlay;
            engine::audio::AudioSystem::play(engine::audio::SoundEvent::UIBlip);
            m_storyState.showToast(
                std::string("Dev Mode: 3D Collision & Axes Overlay ") + (m_showDebugOverlay ? "ON" : "OFF"),
                2.5f);
            break;
        case 8: // [0] Close Dev Mode Console
        default:
            m_devModeOpen = false;
            engine::audio::AudioSystem::play(engine::audio::SoundEvent::UIBlip);
            break;
    }
}

void ClassroomScene::renderDevModeOverlay2D(int screenWidth, int screenHeight) const {
    const float panelW = 560.0f;
    const float panelH = 472.0f;
    const float panelX = (static_cast<float>(screenWidth) - panelW) * 0.5f;
    const float panelY = (static_cast<float>(screenHeight) - panelH) * 0.5f;

    // Subtle dark backdrop behind the Dev Mode console
    engine::ui::TextRenderer::drawRoundedRect2D(
        panelX, panelY, panelW, panelH, 18.0f,
        0.04f, 0.07f, 0.13f, 0.96f,
        0.28f, 0.82f, 0.66f, 0.65f,
        1.6f, true);

    // Header Bar
    engine::ui::TextRenderer::drawRoundedRect2D(
        panelX + 12.0f, panelY + panelH - 48.0f, panelW - 24.0f, 36.0f, 10.0f,
        0.08f, 0.16f, 0.24f, 0.96f,
        0.28f, 0.78f, 0.64f, 0.42f,
        1.0f, false);

    engine::ui::TextRenderer::drawPillBadge2D(
        panelX + 20.0f, panelY + panelH - 41.0f, 82.0f, 22.0f,
        "DEV MODE",
        0.10f, 0.42f, 0.32f, 0.98f,
        0.34f, 0.90f, 0.70f,
        0.90f, 1.0f, 0.95f,
        GLUT_BITMAP_HELVETICA_10);

    engine::ui::TextRenderer::drawBitmapText2D(
        panelX + 112.0f, panelY + panelH - 35.0f,
        "DEVELOPER TELEPORT & TASK BYPASS CONSOLE",
        GLUT_BITMAP_HELVETICA_12,
        0.90f, 0.98f, 0.95f);

    // Live Flag Status Summary Strip
    const bool hasBook   = m_storyState.hasFlag("has_sketchbook");
    const bool metMaya   = m_storyState.hasFlag("talked_to_maya");
    const bool replied   = m_storyState.hasFlag("replied_phone_sam");
    const bool metSam    = m_storyState.hasFlag("talked_to_sam");
    const bool playedGtr = m_storyState.hasFlag("completed_rhythm_minigame");

    std::string flagSummary =
        std::string("Flags:  Sketchbook[") + (hasBook ? "YES" : "NO") + "]   " +
        "Maya[" + (metMaya ? "YES" : "NO") + "]   " +
        "Phone[" + (replied ? "YES" : "NO") + "]   " +
        "Sam[" + (metSam ? "YES" : "NO") + "]   " +
        "Guitar[" + (playedGtr ? "YES" : "NO") + "]";

    engine::ui::TextRenderer::drawRoundedRect2D(
        panelX + 14.0f, panelY + panelH - 84.0f, panelW - 28.0f, 28.0f, 8.0f,
        0.07f, 0.11f, 0.18f, 0.94f,
        0.32f, 0.48f, 0.64f, 0.35f,
        1.0f, false);

    engine::ui::TextRenderer::drawBitmapTextCentered2D(
        panelX + panelW * 0.5f, panelY + panelH - 75.0f,
        flagSummary,
        GLUT_BITMAP_HELVETICA_12,
        0.76f, 0.92f, 0.86f);

    struct DevOption {
        const char* badge;
        std::string label;
        bool        isSceneOption;
        bool        isActiveScene;
    };

    const DevOption options[8] = {
        { "1", "Teleport -> Scene 1: Room 304 Classroom (FEU Tech Lab)", true,  m_currentLocation == LocationId::Classroom },
        { "2", "Teleport -> Scene 2: 3rd-Floor Campus Hallway",          true,  m_currentLocation == LocationId::Hallway },
        { "3", "Teleport -> Scene 3: 937 P. Paredes St (FEU Tech & Canteens)", true, m_currentLocation == LocationId::RainyStreet },
        { "4", "Bypass Current Stage Task (Complete Active TODO Step)",  false, false },
        { "5", "Bypass ALL Tasks & Unlock All Doors / Scenes",           false, false },
        { "6", "Reset All Tasks & Story Flags Back to Stage 1",          false, false },
        { "7", "Launch Acoustic Guitar Rhythm Minigame Immediately",     false, false },
        { "8", std::string("Toggle 3D Colliders & Axes Debug Overlay (Currently ") + (m_showDebugOverlay ? "ON)" : "OFF)"), false, false }
    };

    float rowY = panelY + panelH - 128.0f;
    const float rowH = 34.0f;
    const float rowGap = 39.0f;

    for (int i = 0; i < 8; ++i) {
        const bool selected = (i == m_devMenuCursor);
        const float rx = panelX + 16.0f;
        const float rw = panelW - 32.0f;

        engine::ui::TextRenderer::drawRoundedRect2D(
            rx, rowY, rw, rowH, 9.0f,
            selected ? 0.12f : 0.08f,
            selected ? 0.28f : 0.12f,
            selected ? 0.34f : 0.20f,
            0.95f,
            selected ? 0.36f : 0.34f,
            selected ? 0.88f : 0.46f,
            selected ? 0.74f : 0.62f,
            selected ? 0.85f : 0.35f,
            selected ? 1.4f  : 1.0f,
            false);

        engine::ui::TextRenderer::drawPillBadge2D(
            rx + 10.0f, rowY + 7.0f, 28.0f, 20.0f,
            options[i].badge,
            selected ? 0.16f : 0.12f,
            selected ? 0.52f : 0.22f,
            selected ? 0.44f : 0.36f,
            0.96f,
            selected ? 0.42f : 0.38f,
            selected ? 0.92f : 0.56f,
            selected ? 0.78f : 0.78f,
            0.94f, 0.99f, 1.0f,
            GLUT_BITMAP_HELVETICA_12);

        engine::ui::TextRenderer::drawBitmapText2D(
            rx + 48.0f, rowY + 11.0f,
            options[i].label,
            GLUT_BITMAP_HELVETICA_12,
            selected ? 0.95f : 0.84f,
            selected ? 1.00f : 0.90f,
            selected ? 0.98f : 0.96f);

        if (options[i].isSceneOption && options[i].isActiveScene) {
            engine::ui::TextRenderer::drawPillBadge2D(
                rx + rw - 76.0f, rowY + 7.0f, 64.0f, 20.0f,
                "CURRENT",
                0.12f, 0.38f, 0.28f, 0.94f,
                0.34f, 0.84f, 0.62f,
                0.85f, 1.0f, 0.92f,
                GLUT_BITMAP_HELVETICA_10);
        }

        rowY -= rowGap;
    }

    // Footer instruction bar
    engine::ui::TextRenderer::drawBitmapTextCentered2D(
        panelX + panelW * 0.5f, panelY + 14.0f,
        "[Up/Down + Enter] or [1-8] Execute   |   [0 / ~ / Ctrl+D / DEV] Close Console",
        GLUT_BITMAP_HELVETICA_12,
        0.72f, 0.86f, 0.94f);
}

void ClassroomScene::onKeyPress(unsigned char key) {
    if (m_transition.isActive()) {
        return;
    }

    // -------------------------------------------------------------------------
    // Developer Mode Key-Combination Detector:
    //   1. [Ctrl + D] or [Ctrl + Shift + D] (ASCII 4 or Win32 chord)
    //   2. Sequential key combo: typing "DEV" (D -> E -> V)
    //   3. [~] or [`] anytime once Dev Mode is unlocked (or Shift + ~ anytime)
    // -------------------------------------------------------------------------
    if (key == 4 || key == '~' || (key == '`' && m_devModeUnlocked)) {
        m_keyComboBuffer.clear();
        toggleDevMode();
        return;
    }

    if (std::isalpha(static_cast<unsigned char>(key))) {
        m_keyComboBuffer.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(key))));
        if (m_keyComboBuffer.size() > 6) {
            m_keyComboBuffer.erase(0, m_keyComboBuffer.size() - 6);
        }
        if (m_keyComboBuffer.size() >= 3 &&
            m_keyComboBuffer.substr(m_keyComboBuffer.size() - 3) == "DEV") {
            m_keyComboBuffer.clear();
            m_dialogue.closeDialogue();
            toggleDevMode();
            return;
        }
    }

    // If Dev Mode Console is open, route keys directly to Dev Mode actions
    if (m_devModeOpen) {
        if (key >= '1' && key <= '8') {
            m_devMenuCursor = static_cast<int>(key - '1');
            executeDevAction(m_devMenuCursor);
        } else if (key == '0' || key == '`') {
            executeDevAction(8);
        } else if (key == 13 || key == ' ' || key == 'e' || key == 'E') {
            executeDevAction(m_devMenuCursor);
        }
        return;
    }

    // If Rhythm Minigame is active, route input directly to minigame
    if (m_rhythmMinigame.isActive()) {
        bool wasFinished = m_rhythmMinigame.isFinished();
        m_rhythmMinigame.onKeyPress(key, m_storyState);
        if (wasFinished && !m_rhythmMinigame.isActive()) {
            m_dialogue.startDialogue("post_minigame_celebration");
        }
        return;
    }

    // Shortcut [G] launches the Acoustic Rhythm Minigame anytime when not in dialogue
    if ((key == 'g' || key == 'G') && !m_dialogue.isActive() && !m_phoneSystem.isOpen()) {
        m_rhythmMinigame.startSession();
        return;
    }

    if (key == 9 || key == 'b' || key == 'B') {
        if (!m_dialogue.isActive()) {
            m_phoneSystem.toggleOpen();
            return;
        }
    }

    if (m_phoneSystem.isOpen()) {
        if (key == '1') {
            m_phoneSystem.selectChoice(0, m_storyState);
        } else if (key == '2') {
            m_phoneSystem.selectChoice(1, m_storyState);
        } else if (key == 'n' || key == 'N') {
            m_phoneSystem.selectNextThread();
        }
        return;
    }

    if (m_dialogue.isActive() && m_dialogue.isWaitingForChoice()) {
        if (key == '1') {
            m_dialogue.selectChoice(0, &m_storyState);
            return;
        } else if (key == '2') {
            m_dialogue.selectChoice(1, &m_storyState);
            return;
        }
    }

    if (key == 'e' || key == 'E' || key == 13 || key == ' ') {
        if (m_dialogue.isActive()) {
            m_dialogue.advanceOrCompleteLine(&m_storyState);
            return;
        }
        auto* activeObj = m_interactions.getActiveInteractableMutable();
        if (!activeObj) {
            // Even when pressing interact away from a specific object, respond with a monologue
            m_dialogue.startDialogue("inspect_room_general");
            return;
        }

        m_interactions.incrementInteractionCount(activeObj);

        // Handle Door / Stairwell / Gate Scene Transitions & Prop Responses across the 3 locations
        if (activeObj->id == "classroom_door") {
            if (m_storyState.hasFlag("hallway_unlocked") || m_devModeUnlocked) {
                engine::audio::AudioSystem::play(engine::audio::SoundEvent::DialogueSelect);
                m_storyState.showToast(m_devModeUnlocked && !m_storyState.hasFlag("hallway_unlocked")
                    ? "Dev Mode: Bypassed lock -> 3rd-Floor Hallway"
                    : "Entered 3rd-Floor School Hallway");
                switchLocation(LocationId::Hallway, -3.4f, 0.2f, true);
            } else {
                m_dialogue.startDialogue("inspect_door_locked");
            }
        } else if (activeObj->id == "door_to_classroom") {
            engine::audio::AudioSystem::play(engine::audio::SoundEvent::DialogueSelect);
            m_storyState.showToast("Returned to Room 304 Classroom");
            switchLocation(LocationId::Classroom, 3.4f, -1.2f, true);
        } else if (activeObj->id == "stairs_to_street") {
            if (m_storyState.hasFlag("street_unlocked") || m_storyState.hasFlag("talked_to_sam") || m_devModeUnlocked) {
                engine::audio::AudioSystem::play(engine::audio::SoundEvent::DialogueSelect);
                m_storyState.showToast(m_devModeUnlocked && !m_storyState.hasFlag("street_unlocked")
                    ? "Dev Mode: Bypassed lock -> 937 P. Paredes St"
                    : "Stepped out onto Rainy Street");
                switchLocation(LocationId::RainyStreet, -3.2f, 0.9f, true);
            } else {
                m_dialogue.startDialogue("inspect_stairs_locked");
            }
        } else if (activeObj->id == "gate_to_hallway") {
            engine::audio::AudioSystem::play(engine::audio::SoundEvent::DialogueSelect);
            m_storyState.showToast("Returned to 3rd-Floor School Hallway");
            switchLocation(LocationId::Hallway, 3.4f, -0.8f, true);
        } else if (activeObj->id == "guitar_bench") {
            engine::audio::AudioSystem::play(engine::audio::SoundEvent::DialogueSelect);
            m_storyState.showToast("Started Acoustic Guitar Practice");
            m_rhythmMinigame.startSession();
        } else if (activeObj->id == "sketchbook_desk" && m_storyState.hasFlag("has_sketchbook")) {
            m_dialogue.startDialogue("inspect_sketchbook_taken");
        } else if (!activeObj->dialogueNodeId.empty()) {
            m_dialogue.startDialogue(activeObj->dialogueNodeId);
        } else {
            m_dialogue.startDialogue("inspect_room_general");
        }
    } else if (key == 't' || key == 'T') {
        m_useCheckerTexture = !m_useCheckerTexture;
    } else if (key == 'm' || key == 'M') {
        m_wireframeProps = !m_wireframeProps;
    } else if (key == 'r' || key == 'R') {
        m_player.setPosition(-0.35f, 0.0f, 1.15f);
    } else if (key == '1') {
        m_lighting.toggleLightingEnabled();
    } else if (key == '2') {
        m_lighting.toggleSunLight();
    } else if (key == '3') {
        m_lighting.toggleSpotLight();
    } else if (key == '4') {
        m_lighting.toggleSpecular();
    }
}

void ClassroomScene::onSpecialKeyPress(int key) {
    if (key == GLUT_KEY_F10) {
        toggleDevMode();
        return;
    }

    if (m_devModeOpen) {
        if (key == GLUT_KEY_UP) {
            m_devMenuCursor = (m_devMenuCursor + 7) % 8;
            engine::audio::AudioSystem::play(engine::audio::SoundEvent::UIBlip);
            return;
        } else if (key == GLUT_KEY_DOWN) {
            m_devMenuCursor = (m_devMenuCursor + 1) % 8;
            engine::audio::AudioSystem::play(engine::audio::SoundEvent::UIBlip);
            return;
        }
    }

    if (m_dialogue.isActive() && m_dialogue.isWaitingForChoice()) {
        if (key == GLUT_KEY_UP) {
            m_dialogue.moveChoiceCursor(-1);
            return;
        } else if (key == GLUT_KEY_DOWN) {
            m_dialogue.moveChoiceCursor(1);
            return;
        }
    }

    if (key == GLUT_KEY_F3) {
        m_showDebugOverlay = !m_showDebugOverlay;
    } else if (key == GLUT_KEY_F5) {
        m_storyState.saveToFile(
            "data/saves/save_slot_1.sav",
            getCurrentLocationIdString(),
            m_player.getX(),
            m_player.getZ());
    } else if (key == GLUT_KEY_F9) {
        std::string sceneId;
        float px = m_player.getX();
        float pz = m_player.getZ();
        if (m_storyState.loadFromFile("data/saves/save_slot_1.sav", sceneId, px, pz)) {
            LocationId target = LocationId::Classroom;
            if (sceneId == "hallway")      target = LocationId::Hallway;
            if (sceneId == "rainy_street") target = LocationId::RainyStreet;
            switchLocation(target, px, pz, true);
        }
    } else if (key == GLUT_KEY_F6) {
        switchLocation(LocationId::Classroom, -0.35f, 1.15f, true);
    } else if (key == GLUT_KEY_F7) {
        switchLocation(LocationId::Hallway, -3.2f, 0.3f, true);
    } else if (key == GLUT_KEY_F8) {
        switchLocation(LocationId::RainyStreet, -2.5f, 0.9f, true);
    }
}

void ClassroomScene::onMouseClick(int button, int state) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (m_dialogue.isActive()) {
            m_dialogue.advanceOrCompleteLine(&m_storyState);
        }
    }
}

bool ClassroomScene::isValid() const {
    return m_texWoodFloor.isValid() &&
           m_texClassroomWall.isValid() &&
           m_texDeskWood.isValid() &&
           m_texChalkboard.isValid() &&
           m_texWindowGlass.isValid() &&
           m_texHallwayTile.isValid() &&
           m_texSchoolLockers.isValid() &&
           m_texWetAsphalt.isValid() &&
           m_texStorefrontNeon.isValid() &&
           m_player.isValid() &&
           m_npcMaya.isValid() &&
           m_npcSam.isValid();
}

} // namespace scenes
} // namespace game
