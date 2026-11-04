#include "game/scenes/ClassroomScene.h"
#include "engine/core/Math4x4.h"
#include "engine/renderer/PrimitiveRenderer.h"
#include "engine/ui/TextRenderer.h"
#include "engine/input/InputManager.h"
#include <GL/freeglut.h>
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

std::string ClassroomScene::getCurrentLocationTitle() const {
    switch (m_currentLocation) {
        case LocationId::Classroom:   return "ROOM 304 - NOVEMBER 14 (LATE AFTERNOON)";
        case LocationId::Hallway:     return "3RD-FLOOR SCHOOL HALLWAY - TWILIGHT";
        case LocationId::RainyStreet: return "SUNDOWN MART & BUS STOP - RAINY EVENING";
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
        std::string title = "ROOM 304 - CLASSROOM";
        if (targetLocation == LocationId::Hallway)     title = "3RD-FLOOR SCHOOL HALLWAY";
        if (targetLocation == LocationId::RainyStreet) title = "SUNDOWN MART - RAINY STREET";
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
            "chalkboard_schedule", "Classroom Chalkboard", "[E] Read Chalkboard Schedule", "inspect_chalkboard",
            0.0f, 1.55f, -2.85f, 1.45f);
        m_interactions.registerInteractable(
            "sunset_window", "West Courtyard Window", "[E] Look Out Sunset Window", "inspect_window",
            -4.10f, 1.55f, -1.15f, 1.35f);
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
            "hallway_lockers", "Student Lockers", "[E] Inspect Lockers", "inspect_lockers",
            -2.2f, 1.45f, -2.35f, 1.35f);
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
        // Convenience Store Building, Bus Bench, and Streetlight colliders
        m_collisionWorld.addBoxCollider(-1.4f, 1.7f, -2.45f, 5.6f, 3.4f, 1.45f, "SundownMartBuilding");
        m_collisionWorld.addBoxCollider( 2.35f, 0.45f, -0.75f, 1.55f, 0.85f, 0.72f, "BusStopBench");

        // Position Maya under the Sundown Mart awning
        m_npcMaya.setPositionAndDialogue(-0.65f, 0.0f, -1.15f, "maya_street_talk");
        m_npcMaya.registerCollider(m_collisionWorld);

        m_interactions.registerInteractable(
            "maya_street_npc", "Maya (Under Awning)", "[E] Talk to Maya", "maya_street_talk",
            m_npcMaya.getX(), 1.52f, m_npcMaya.getZ(), 1.40f);
        m_interactions.registerInteractable(
            "storefront_window", "24/7 Sundown Mart", "[E] Look Inside Sundown Mart", "inspect_storefront",
            -2.35f, 1.45f, -1.55f, 1.40f);
        m_interactions.registerInteractable(
            "guitar_bench", "Sam's Acoustic Guitar", "[E] Practice Acoustic Song", "maya_street_ready",
            2.35f, 1.15f, -0.75f, 1.45f);
        m_interactions.registerInteractable(
            "bus_stop_sign", "Bus Stop Sign & Lamp", "[E] Check Bus Schedule", "inspect_bus_sign",
            3.35f, 1.45f, -0.65f, 1.35f);
        m_interactions.registerInteractable(
            "gate_to_hallway", "School Gate Stairs", "[E] Return to School Hallway", "",
            -4.15f, 1.45f, 1.15f, 1.45f);
    }
}

void ClassroomScene::update(float dt, engine::camera::Camera3D& camera) {
    m_timeSeconds += dt;
    m_storyState.update(dt);
    m_transition.update(dt);

    if (m_dialogue.isActive()) {
        m_dialogue.update(dt);
    } else if (!m_phoneSystem.isOpen() && !m_transition.isActive() && !camera.isFreeCameraEnabled()) {
        m_player.update(dt, m_collisionWorld, true);
    }

    if (m_currentLocation == LocationId::Classroom || m_currentLocation == LocationId::RainyStreet) {
        m_npcMaya.update(dt, m_player.getX(), m_player.getZ());
    }
    if (m_currentLocation == LocationId::Hallway) {
        m_npcSam.update(dt, m_player.getX(), m_player.getZ());
    }

    m_interactions.update(m_player.getX(), m_player.getZ());

    camera.updateFreeOrbit(dt);
    if (!camera.isFreeCameraEnabled()) {
        camera.setNarrativeTarget(
            m_player.getX() * 0.58f,
            1.20f,
            m_player.getZ() * 0.45f,
            dt);
    }

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

    // Corridor Wooden Waiting Bench in Foreground
    glPushMatrix();
    glTranslatef(0.0f, 0.45f, 1.65f);
    glScalef(1.35f, 0.8f, 0.85f);
    m_meshDeskTop.drawVBOElements();
    glPopMatrix();
    engine::assets::Texture2D::unbind();

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
    const auto& roadTex  = m_useCheckerTexture ? m_texCheckerDebug : m_texWetAsphalt;
    const auto& storeTex = m_useCheckerTexture ? m_texCheckerDebug : m_texStorefrontNeon;
    const auto& woodTex  = m_useCheckerTexture ? m_texCheckerDebug : m_texDeskWood;

    // 1. Wet Asphalt Street & Sidewalk Floor
    roadTex.bind();
    glPushMatrix();
    m_meshFloor.drawVBOElements();
    glPopMatrix();

    // 2. 24/7 Sundown Mart Convenience Store Building & Glowing Neon Awning
    storeTex.bind();
    glPushMatrix();
    glTranslatef(-1.40f, 1.70f, -2.55f);
    m_meshStoreBuilding.drawVBOElements();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-1.40f, 2.45f, -1.35f);
    m_meshStoreAwning.drawVBOElements();
    glPopMatrix();

    // 3. Wooden Bus-Stop Bench + Sam's Acoustic Guitar Case Prop
    woodTex.bind();
    glPushMatrix();
    glTranslatef(2.35f, 0.46f, -0.75f);
    glScalef(1.15f, 0.85f, 0.85f);
    m_meshDeskTop.drawVBOElements();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.35f, 0.56f, -0.70f);
    glRotatef(22.0f, 0.0f, 1.0f, 0.0f);
    glScalef(1.6f, 1.3f, 1.2f);
    m_meshSketchbookProp.drawVBOElements();
    glPopMatrix();
    engine::assets::Texture2D::unbind();

    // 4. Cast-Iron Streetlight Pole & Glowing Lamp Head (GLUT Cylinder/Sphere/Cone)
    engine::renderer::LightingSystem::applyMaterial(
        engine::renderer::LightingSystem::polishedCeramicMaterial());
    glPushMatrix();
    glTranslatef(3.35f, 1.65f, -0.65f);
    glScalef(0.08f, 3.3f, 0.08f);
    glColor3f(0.22f, 0.25f, 0.30f);
    glutSolidCube(1.0);
    glPopMatrix();

    engine::renderer::LightingSystem::applyMaterial(
        engine::renderer::LightingSystem::glowingLampMaterial());
    glPushMatrix();
    glTranslatef(3.15f, 3.25f, -0.65f);
    glColor3f(1.0f, 0.92f, 0.65f);
    glutSolidSphere(0.18, 16, 16);
    glPopMatrix();

    // 5. Render Maya (under awning) & Elijah sorted by Z
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

    // 6. Module 2 Rain Streaks (GL_LINES + glLineStipple) & Puddle Splash Ripples (GL_LINE_LOOP)
    engine::renderer::PrimitiveRenderer::drawRainAndRipples(m_timeSeconds, 96);
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

    engine::ui::TextRenderer::drawPanel2D(16.0f, static_cast<float>(h - 175), 615.0f, 158.0f);

    engine::ui::TextRenderer::drawStrokeText2D(
        30.0f, static_cast<float>(h - 44),
        getCurrentLocationTitle(),
        0.125f, 2.0f, GLUT_STROKE_ROMAN,
        1.0f, 0.86f, 0.48f);

    std::ostringstream flagsLine;
    flagsLine << "Location: " << getCurrentLocationIdString()
              << " | Sketchbook=" << (m_storyState.hasFlag("has_sketchbook") ? "YES" : "NO")
              << " | SamTalk=" << (m_storyState.hasFlag("talked_to_sam") ? "YES" : "NO")
              << " | StreetReady=" << (m_storyState.hasFlag("ready_for_minigame") ? "YES" : "NO");
    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 70),
        flagsLine.str(), GLUT_BITMAP_HELVETICA_12, 0.92f, 0.98f, 0.88f);

    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 92),
        m_storyState.getCurrentObjectiveText(),
        GLUT_BITMAP_HELVETICA_12, 1.0f, 0.88f, 0.50f);

    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 114),
        "Controls: WASD/Arrows Move | [E] Interact | [1/2] Choice | [Tab] Phone | [F5/F9] Save/Load",
        GLUT_BITMAP_HELVETICA_12, 0.80f, 0.88f, 0.96f);

    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 136),
        "Locations: [F6] Classroom | [F7] Hallway | [F8] Rainy Street | [P] Proj | [F3] Debug",
        GLUT_BITMAP_HELVETICA_12, 0.76f, 0.86f, 0.80f);

    if (m_storyState.hasActiveToast()) {
        engine::ui::TextRenderer::drawBitmapText2D(
            30.0f, static_cast<float>(h - 158),
            ">> " + m_storyState.getToastMessage(),
            GLUT_BITMAP_9_BY_15, 0.55f, 1.0f, 0.75f);
    } else {
        const auto* nearObj = m_interactions.getActiveInteractable();
        std::string targetText = nearObj ? nearObj->displayName : "Explore current location or press [F6/F7/F8] to switch scenes";
        engine::ui::TextRenderer::drawBitmapText2D(
            30.0f, static_cast<float>(h - 158),
            "Nearby: " + targetText,
            GLUT_BITMAP_9_BY_15, 0.68f, 0.92f, 0.85f);
    }

    m_phoneSystem.render2D(w, h, m_storyState);

    if (m_dialogue.isActive()) {
        m_dialogue.render2D(w, h);
    } else if (!m_phoneSystem.isOpen()) {
        m_interactions.render2DPrompt(w, h);
    }

    m_transition.render2D(w, h);
    engine::camera::Camera3D::end2DOverlay();
}

void ClassroomScene::onKeyPress(unsigned char key) {
    if (m_transition.isActive()) {
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
        if (key == 'e' || key == 'E' || key == 13) {
            auto* activeObj = m_interactions.getActiveInteractableMutable();
            if (!activeObj) return;

            m_interactions.incrementInteractionCount(activeObj);

            // Handle Door / Stairwell / Gate Scene Transitions across the 3 locations
            if (activeObj->id == "classroom_door") {
                if (m_storyState.hasFlag("hallway_unlocked")) {
                    switchLocation(LocationId::Hallway, -3.4f, 0.2f, true);
                } else {
                    m_dialogue.startDialogue("inspect_door_locked");
                }
            } else if (activeObj->id == "door_to_classroom") {
                switchLocation(LocationId::Classroom, 3.4f, -1.2f, true);
            } else if (activeObj->id == "stairs_to_street") {
                if (m_storyState.hasFlag("street_unlocked") || m_storyState.hasFlag("talked_to_sam")) {
                    switchLocation(LocationId::RainyStreet, -3.2f, 0.9f, true);
                } else {
                    m_dialogue.startDialogue("inspect_stairs_locked");
                }
            } else if (activeObj->id == "gate_to_hallway") {
                switchLocation(LocationId::Hallway, 3.4f, -0.8f, true);
            } else if (!activeObj->dialogueNodeId.empty()) {
                m_dialogue.startDialogue(activeObj->dialogueNodeId);
            }
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
