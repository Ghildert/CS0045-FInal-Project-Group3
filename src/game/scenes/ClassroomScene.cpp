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
    : m_timeSeconds(0.0f)
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

    // Build reusable VBO meshes
    m_meshFloor.buildQuadXZ(9.2f, 6.8f, 6.0f, 4.0f, GL_STATIC_DRAW);
    m_meshBackWall.buildTexturedBox(9.2f, 3.8f, 0.22f, 4.0f, 1.0f, GL_STATIC_DRAW);
    m_meshSideWall.buildTexturedBox(0.22f, 3.8f, 6.8f, 3.0f, 1.0f, GL_STATIC_DRAW);
    m_meshChalkboard.buildTexturedBox(4.3f, 1.45f, 0.08f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshDoorPanel.buildTexturedBox(0.10f, 2.35f, 1.15f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshWindowPane.buildTexturedBox(0.08f, 1.75f, 2.15f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshDeskTop.buildTexturedBox(1.30f, 0.09f, 0.72f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshDeskLeg.buildTexturedBox(0.08f, 0.72f, 0.08f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshChairSeat.buildTexturedBox(0.48f, 0.06f, 0.46f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshChairBack.buildTexturedBox(0.48f, 0.44f, 0.06f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshSketchbookProp.buildTexturedBox(0.34f, 0.04f, 0.26f, 1.0f, 1.0f, GL_STATIC_DRAW);
    m_meshDynamicPyramid.buildPyramid(0.38f, 0.48f, GL_DYNAMIC_DRAW);

    engine::core::Math4x4::translateVertexInMappedVBO(
        m_meshDynamicPyramid.getVBOId(), 0.0f, 0.02f, 0.0f, 0);

    // Initialize Player (Elijah) and NPC Classmate (Maya)
    m_player.init("assets/sprites/player_elijah_sheet.png");
    m_player.setPosition(-0.35f, 0.0f, 1.15f);

    m_npcMaya.init(
        "Maya",
        "assets/sprites/npc_maya_sheet.png",
        -2.75f, 0.0f, -0.55f,
        "maya_classroom_talk");

    // Initialize Phase 4 Narrative, Portraits, External Dialogue & Phone Systems
    m_storyState.resetNewGame();
    m_dialogue.initPortraitsAndData("data/dialogue/story_dialogues.txt");
    m_phoneSystem.loadFromExternalData("data/dialogue/phone_messages.txt");

    buildCollidersAndInteractables();
    m_lighting.initClassroomLighting();
}

void ClassroomScene::destroy() {
    m_dialogue.destroy();
    m_player.destroy();
    m_npcMaya.destroy();

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
    m_meshDynamicPyramid.destroy();

    m_texWoodFloor.destroy();
    m_texClassroomWall.destroy();
    m_texDeskWood.destroy();
    m_texChalkboard.destroy();
    m_texWindowGlass.destroy();
    m_texCheckerDebug.destroy();
}

void ClassroomScene::buildCollidersAndInteractables() {
    m_collisionWorld.clear();

    m_collisionWorld.addBoxCollider( 0.0f, 1.9f, -3.30f, 9.4f, 3.8f, 0.35f, "BackWall");
    m_collisionWorld.addBoxCollider( 0.0f, 1.9f,  3.35f, 9.4f, 3.8f, 0.25f, "FrontBoundary");
    m_collisionWorld.addBoxCollider(-4.50f, 1.9f, 0.0f,  0.35f, 3.8f, 6.8f, "LeftWindowWall");
    m_collisionWorld.addBoxCollider( 4.50f, 1.9f, 0.0f,  0.35f, 3.8f, 6.8f, "RightHallwayWall");

    m_collisionWorld.addBoxCollider( 1.85f, 0.42f, -1.85f, 1.40f, 0.85f, 0.82f, "TeacherDesk");
    m_collisionWorld.addBoxCollider(-1.85f, 0.42f,  0.05f, 1.32f, 0.85f, 0.82f, "StudentDesk_R1C1");
    m_collisionWorld.addBoxCollider( 0.15f, 0.42f,  0.05f, 1.32f, 0.85f, 0.82f, "StudentDesk_R1C2");
    m_collisionWorld.addBoxCollider( 2.15f, 0.42f,  0.05f, 1.32f, 0.85f, 0.82f, "StudentDesk_R1C3");
    m_collisionWorld.addBoxCollider(-1.85f, 0.42f,  1.85f, 1.32f, 0.85f, 0.82f, "StudentDesk_R2C1");
    m_collisionWorld.addBoxCollider( 0.15f, 0.42f,  1.85f, 1.32f, 0.85f, 0.82f, "StudentDesk_R2C2");
    m_collisionWorld.addBoxCollider( 2.15f, 0.42f,  1.85f, 1.32f, 0.85f, 0.82f, "StudentDesk_R2C3");

    m_npcMaya.registerCollider(m_collisionWorld);

    m_interactions.clear();
    m_interactions.registerInteractable(
        "maya_npc",
        "Maya (Classmate)",
        "[E] Talk to Maya",
        "maya_classroom_talk",
        m_npcMaya.getX(), 1.52f, m_npcMaya.getZ(),
        1.35f);

    m_interactions.registerInteractable(
        "sketchbook_desk",
        "Elijah's Sketchbook",
        "[E] Inspect Sketchbook & Notes",
        "inspect_sketchbook",
        -1.85f, 1.15f, 0.05f,
        1.30f);

    m_interactions.registerInteractable(
        "chalkboard_schedule",
        "Classroom Chalkboard",
        "[E] Read Chalkboard Schedule",
        "inspect_chalkboard",
        0.0f, 1.55f, -2.85f,
        1.45f);

    m_interactions.registerInteractable(
        "sunset_window",
        "West Courtyard Window",
        "[E] Look Out Sunset Window",
        "inspect_window",
        -4.10f, 1.55f, -1.15f,
        1.35f);

    m_interactions.registerInteractable(
        "classroom_door",
        "Hallway Sliding Door",
        "[E] Open Hallway Door",
        "inspect_door_locked",
        4.15f, 1.45f, -1.65f,
        1.35f);
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

    m_npcMaya.update(dt, m_player.getX(), m_player.getZ());
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

void ClassroomScene::runPhase4VerificationStep(float dt, engine::camera::Camera3D& camera) {
    m_timeSeconds += dt;
    m_storyState.update(dt);
    m_transition.update(dt);

    if (m_verifyStep == 0) {
        // Step 0: Start Maya's branching dialogue
        m_interactions.update(m_npcMaya.getX(), m_npcMaya.getZ());
        if (auto* obj = m_interactions.getActiveInteractableMutable()) {
            m_interactions.incrementInteractionCount(obj);
        }
        m_dialogue.startDialogue("maya_classroom_talk");
        m_verifyStep = 1;
    } else if (m_verifyStep == 1) {
        // Step 1: Advance through Maya's lines until waiting for branching choice, then pick Choice 0
        if (m_dialogue.isWaitingForChoice()) {
            m_dialogue.selectChoice(0, &m_storyState);
            m_verifyStep = 2;
        } else {
            m_dialogue.advanceOrCompleteLine(&m_storyState);
        }
    } else if (m_verifyStep == 2) {
        // Step 2: Finish the branched target sequence ("maya_branch_promise")
        if (m_dialogue.isActive()) {
            m_dialogue.advanceOrCompleteLine(&m_storyState);
        } else {
            // Also obtain the sketchbook flag, reply on the smartphone, and test Save/Load + Transition
            m_storyState.setFlag("has_sketchbook", 1);
            m_phoneSystem.setOpen(true);
            m_phoneSystem.selectChoice(0, m_storyState);
            m_storyState.saveToFile("data/saves/save_slot_1.sav", "classroom", m_player.getX(), m_player.getZ());

            std::string loadedScene;
            float lx = 0.0f, lz = 0.0f;
            m_storyState.loadFromFile("data/saves/save_slot_1.sav", loadedScene, lx, lz);
            m_transition.startTransition("SCHOOL HALLWAY - 3RD FLOOR", nullptr, 0.5f);
            m_verifyStep = 3;
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

    engine::assets::Texture2D::unbind();
    glPopMatrix();
}

void ClassroomScene::render3D(const engine::camera::Camera3D& camera) {
    camera.applyProjection();
    camera.applyView();

    m_lighting.applyLights(m_timeSeconds);
    engine::renderer::LightingSystem::applyMaterial(
        engine::renderer::LightingSystem::matteWoodMaterial());

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
    glTranslatef(-4.60f, 1.9f, 0.0f);
    m_meshSideWall.drawClientSideElements();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(4.60f, 1.9f, 0.0f);
    m_meshSideWall.drawVBOArrays();
    glPopMatrix();

    boardTex.bind();
    glPushMatrix();
    glTranslatef(0.0f, 1.95f, -3.22f);
    m_meshChalkboard.drawVBOArrayElement();
    glPopMatrix();

    deskTex.bind();
    glPushMatrix();
    glTranslatef(4.44f, 1.18f, -1.65f);
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

    // Sketchbook prop on front-left desk (hidden once picked up!)
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
    if (m_wireframeProps) {
        glutWireTeapot(0.15);
    } else {
        glutSolidTeapot(0.15);
    }
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.10f, 0.92f, -1.88f);
    glColor3f(0.45f, 0.76f, 0.88f);
    if (m_wireframeProps) {
        glutWireSphere(0.13, 16, 16);
    } else {
        glutSolidSphere(0.13, 16, 16);
    }
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
    glTranslatef(-4.46f, 2.05f, -1.25f);
    m_meshWindowPane.drawVBOElements();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-4.46f, 2.05f, 1.25f);
    m_meshWindowPane.drawVBOElements();
    glPopMatrix();
    engine::assets::Texture2D::unbind();

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

    // Top-left Narrative & Controls HUD Panel
    engine::ui::TextRenderer::drawPanel2D(16.0f, static_cast<float>(h - 175), 595.0f, 158.0f);

    engine::ui::TextRenderer::drawStrokeText2D(
        30.0f, static_cast<float>(h - 44),
        "ROOM 304 - NOVEMBER 14 (LATE AFTERNOON)",
        0.13f, 2.0f, GLUT_STROKE_ROMAN,
        1.0f, 0.86f, 0.48f);

    std::ostringstream flagsLine;
    flagsLine << "Story Flags: Sketchbook=" << (m_storyState.hasFlag("has_sketchbook") ? "YES" : "NO")
              << " | MayaTalk=" << (m_storyState.hasFlag("talked_to_maya") ? "YES" : "NO")
              << " | PhoneSam=" << (m_storyState.hasFlag("replied_phone_sam") ? "YES" : "NO")
              << " | Hallway=" << (m_storyState.hasFlag("hallway_unlocked") ? "OPEN" : "LOCKED");
    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 70),
        flagsLine.str(), GLUT_BITMAP_HELVETICA_12, 0.92f, 0.98f, 0.88f);

    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 92),
        m_storyState.getCurrentObjectiveText(),
        GLUT_BITMAP_HELVETICA_12, 1.0f, 0.88f, 0.50f);

    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 114),
        "Controls: WASD/Arrows Move | [E] Interact | [1/2] Dialogue/Phone Choice | [Tab] Phone",
        GLUT_BITMAP_HELVETICA_12, 0.80f, 0.88f, 0.96f);

    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 136),
        "Save/Load: [F5] Quick Save | [F9] Quick Load | [P] Projection | [F3] Debug Overlay",
        GLUT_BITMAP_HELVETICA_12, 0.76f, 0.86f, 0.80f);

    if (m_storyState.hasActiveToast()) {
        engine::ui::TextRenderer::drawBitmapText2D(
            30.0f, static_cast<float>(h - 158),
            ">> " + m_storyState.getToastMessage(),
            GLUT_BITMAP_9_BY_15, 0.55f, 1.0f, 0.75f);
    } else {
        const auto* nearObj = m_interactions.getActiveInteractable();
        std::string targetText = nearObj ? nearObj->displayName : "Walk near Maya, Sketchbook, Chalkboard, Window, or Door";
        engine::ui::TextRenderer::drawBitmapText2D(
            30.0f, static_cast<float>(h - 158),
            "Nearby: " + targetText,
            GLUT_BITMAP_9_BY_15, 0.68f, 0.92f, 0.85f);
    }

    // Smartphone notification pill & handset UI
    m_phoneSystem.render2D(w, h, m_storyState);

    // Active Dialogue Box OR Proximity Prompt
    if (m_dialogue.isActive()) {
        m_dialogue.render2D(w, h);
    } else if (!m_phoneSystem.isOpen()) {
        m_interactions.render2DPrompt(w, h);
    }

    // Cinematic Scene Transition Overlay
    m_transition.render2D(w, h);

    engine::camera::Camera3D::end2DOverlay();
}

void ClassroomScene::onKeyPress(unsigned char key) {
    if (m_transition.isActive()) {
        return;
    }

    // Toggle Smartphone with Tab (ASCII 9) or 'b'/'B'
    if (key == 9 || key == 'b' || key == 'B') {
        if (!m_dialogue.isActive()) {
            m_phoneSystem.toggleOpen();
            return;
        }
    }

    // When Smartphone is open, handle [1], [2], and [N] thread switching
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

    // When Dialogue is waiting for a branching choice, allow [1] / [2] selection
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
            if (activeObj) {
                m_interactions.incrementInteractionCount(activeObj);
                if (activeObj->id == "classroom_door") {
                    if (m_storyState.hasFlag("hallway_unlocked")) {
                        m_dialogue.startDialogue("inspect_door_unlocked");
                        m_transition.startTransition(
                            "SCHOOL HALLWAY - 3RD FLOOR",
                            [this]() {
                                m_player.setPosition(3.5f, 0.0f, -1.6f);
                            },
                            1.4f);
                    } else {
                        m_dialogue.startDialogue("inspect_door_locked");
                    }
                } else {
                    m_dialogue.startDialogue(activeObj->dialogueNodeId);
                }
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
            "classroom",
            m_player.getX(),
            m_player.getZ());
    } else if (key == GLUT_KEY_F9) {
        std::string sceneId;
        float px = m_player.getX();
        float pz = m_player.getZ();
        if (m_storyState.loadFromFile("data/saves/save_slot_1.sav", sceneId, px, pz)) {
            m_transition.startTransition(
                "LOADING SAVE SLOT 1...",
                [this, px, pz]() {
                    m_player.setPosition(px, 0.0f, pz);
                },
                1.0f);
        }
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
           m_player.isValid() &&
           m_npcMaya.isValid();
}

} // namespace scenes
} // namespace game
