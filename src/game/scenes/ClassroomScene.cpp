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

    // Module 6 Mapped-VBO vertex translation at scene startup
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

    m_dialogue.loadDefaultClassroomDialogues();
    buildCollidersAndInteractables();
    m_lighting.initClassroomLighting();
}

void ClassroomScene::destroy() {
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

    // Room perimeter walls
    m_collisionWorld.addBoxCollider( 0.0f, 1.9f, -3.30f, 9.4f, 3.8f, 0.35f, "BackWall");
    m_collisionWorld.addBoxCollider( 0.0f, 1.9f,  3.35f, 9.4f, 3.8f, 0.25f, "FrontBoundary");
    m_collisionWorld.addBoxCollider(-4.50f, 1.9f, 0.0f,  0.35f, 3.8f, 6.8f, "LeftWindowWall");
    m_collisionWorld.addBoxCollider( 4.50f, 1.9f, 0.0f,  0.35f, 3.8f, 6.8f, "RightHallwayWall");

    // Teacher's Desk & 6 Student Desks (2 rows x 3 columns)
    m_collisionWorld.addBoxCollider( 1.85f, 0.42f, -1.85f, 1.40f, 0.85f, 0.82f, "TeacherDesk");

    m_collisionWorld.addBoxCollider(-1.85f, 0.42f,  0.05f, 1.32f, 0.85f, 0.82f, "StudentDesk_R1C1");
    m_collisionWorld.addBoxCollider( 0.15f, 0.42f,  0.05f, 1.32f, 0.85f, 0.82f, "StudentDesk_R1C2");
    m_collisionWorld.addBoxCollider( 2.15f, 0.42f,  0.05f, 1.32f, 0.85f, 0.82f, "StudentDesk_R1C3");

    m_collisionWorld.addBoxCollider(-1.85f, 0.42f,  1.85f, 1.32f, 0.85f, 0.82f, "StudentDesk_R2C1");
    m_collisionWorld.addBoxCollider( 0.15f, 0.42f,  1.85f, 1.32f, 0.85f, 0.82f, "StudentDesk_R2C2");
    m_collisionWorld.addBoxCollider( 2.15f, 0.42f,  1.85f, 1.32f, 0.85f, 0.82f, "StudentDesk_R2C3");

    // Register NPC Maya collider
    m_npcMaya.registerCollider(m_collisionWorld);

    // Register Interactive Objects & NPC in InteractionManager
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
        "[E] Check Hallway Door",
        "inspect_door",
        4.15f, 1.45f, -1.65f,
        1.35f);
}

void ClassroomScene::update(float dt, engine::camera::Camera3D& camera) {
    m_timeSeconds += dt;

    // Update active dialogue typewriter or player movement
    if (m_dialogue.isActive()) {
        m_dialogue.update(dt);
    } else if (!camera.isFreeCameraEnabled()) {
        m_player.update(dt, m_collisionWorld, true);
    }

    // Update NPC Maya idle animation & facing toward player
    m_npcMaya.update(dt, m_player.getX(), m_player.getZ());

    // Update proximity detection for interactive objects/NPCs
    m_interactions.update(m_player.getX(), m_player.getZ());

    // Update camera
    camera.updateFreeOrbit(dt);
    if (!camera.isFreeCameraEnabled()) {
        camera.setNarrativeTarget(
            m_player.getX() * 0.58f,
            1.20f,
            m_player.getZ() * 0.45f,
            dt);
    }

    // Module 6 Subtopic 1 & 2: Subtle vertical bobbing on pyramid apex vertex via glMapBuffer
    float bobVel = std::cos(m_timeSeconds * 3.0f) * 0.0015f;
    engine::core::Math4x4::translateVertexInMappedVBO(
        m_meshDynamicPyramid.getVBOId(), 0.0f, bobVel, 0.0f, 0);
}

void ClassroomScene::runPhase3VerificationStep(float dt, engine::camera::Camera3D& camera) {
    m_timeSeconds += dt;

    // Step 1: Walk toward Maya & Sketchbook until an interactable is in range, then trigger & complete dialogue
    m_interactions.update(m_player.getX(), m_player.getZ());
    if (!m_dialogue.isActive() && m_dialogue.getCompletedDialoguesCount() == 0) {
        auto* activeObj = m_interactions.getActiveInteractableMutable();
        if (activeObj) {
            m_interactions.incrementInteractionCount(activeObj);
            m_dialogue.startDialogue(activeObj->dialogueNodeId);
        } else {
            // Walk toward Maya at (-2.75, -0.55) / Sketchbook at (-1.85, 0.05)
            m_player.simulateMoveStep(-0.85f, -0.65f, 0.09f, m_collisionWorld);
        }
    } else if (m_dialogue.isActive()) {
        // Rapidly advance through all lines of the active dialogue to verify full completion
        m_dialogue.advanceOrCompleteLine();
        m_dialogue.advanceOrCompleteLine();
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

    // Desk tabletop (VBO+EBO glDrawElements)
    glPushMatrix();
    glTranslatef(0.0f, 0.74f, 0.0f);
    m_meshDeskTop.drawVBOElements();
    glPopMatrix();

    // 4 Desk legs (VBO glDrawArrays)
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

    // Student Chair tucked slightly behind desk (+Z)
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

    // 1. Classroom Wood Floor (VBO + EBO glDrawElements)
    floorTex.bind();
    glPushMatrix();
    m_meshFloor.drawVBOElements();
    glPopMatrix();

    // 2. Back Wall (VBO glDrawArrays) & Left/Right Walls (Module 4 Client-Side Vertex Arrays)
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

    // 3. Front Chalkboard (Module 6 glArrayElement inside VBO)
    boardTex.bind();
    glPushMatrix();
    glTranslatef(0.0f, 1.95f, -3.22f);
    m_meshChalkboard.drawVBOArrayElement();
    glPopMatrix();

    // 4. Right Wall Sliding Wooden Door to Hallway
    deskTex.bind();
    glPushMatrix();
    glTranslatef(4.44f, 1.18f, -1.65f);
    m_meshDoorPanel.drawVBOElements();
    glPopMatrix();
    engine::assets::Texture2D::unbind();

    // 5. Teacher's Desk + 6 Student Desks & Chairs
    drawDeskWithChair3D( 1.85f, -1.85f, 172.0f, deskTex);

    drawDeskWithChair3D(-1.85f,  0.05f,   0.0f, deskTex);
    drawDeskWithChair3D( 0.15f,  0.05f,   0.0f, deskTex);
    drawDeskWithChair3D( 2.15f,  0.05f,   0.0f, deskTex);

    drawDeskWithChair3D(-1.85f,  1.85f,   0.0f, deskTex);
    drawDeskWithChair3D( 0.15f,  1.85f,   0.0f, deskTex);
    drawDeskWithChair3D( 2.15f,  1.85f,   0.0f, deskTex);

    // 6. Interactive Prop: Elijah's Sketchbook on Front-Left Desk (-1.85, 0.05)
    glPushMatrix();
    glTranslatef(-1.72f, 0.81f, 0.08f);
    glRotatef(14.0f, 0.0f, 1.0f, 0.0f);
    m_texCheckerDebug.bind();
    m_meshSketchbookProp.drawVBOElements();
    engine::assets::Texture2D::unbind();
    glPopMatrix();

    // 7. Module 6 Mapped-VBO 3D Pyramid Prop on Center Desk (0.15, 0.05)
    glPushMatrix();
    glTranslatef(0.15f, 0.80f, 0.05f);
    glRotatef(m_timeSeconds * 38.0f, 0.0f, 1.0f, 0.0f);
    m_meshDynamicPyramid.drawVBOArrays();
    glPopMatrix();

    // 8. Module 8 GLUT 3D Props on Teacher's Desk (Polished Ceramic Teapot & Globe Sphere)
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

    // 9. Ceiling Lamp Fixture (Emissive Material)
    engine::renderer::LightingSystem::applyMaterial(
        engine::renderer::LightingSystem::glowingLampMaterial());
    glPushMatrix();
    glTranslatef(1.5f, 3.62f, -0.8f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 0.95f, 0.75f);
    glutSolidCone(0.22, 0.18, 16, 4);
    glPopMatrix();

    // 10. Render 2.5D Characters (Depth-ordered by Z coordinate for clean alpha edges)
    engine::renderer::LightingSystem::applyMaterial(
        engine::renderer::LightingSystem::matteWoodMaterial());
    if (m_npcMaya.getZ() < m_player.getZ()) {
        m_npcMaya.render(camera.getYawDegrees(), m_showDebugOverlay);
        m_player.render(camera.getYawDegrees(), m_showDebugOverlay);
    } else {
        m_player.render(camera.getYawDegrees(), m_showDebugOverlay);
        m_npcMaya.render(camera.getYawDegrees(), m_showDebugOverlay);
    }

    // 11. Render 3D Interaction Indicators (Bobbing Amber Diamond + Stippled Ring)
    m_interactions.render3DIndicators(m_timeSeconds, m_showDebugOverlay);

    // 12. Atmospheric Dust Motes (Module 2 GL_POINTS) & Sunset Windows
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

    // 13. Debug Overlay (F3): Reference Grid, Axes, Stippled AABB Colliders & Module 2 Showcase
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

    // Top-left Scene & Objective HUD Panel
    engine::ui::TextRenderer::drawPanel2D(16.0f, static_cast<float>(h - 172), 560.0f, 156.0f);

    engine::ui::TextRenderer::drawStrokeText2D(
        30.0f, static_cast<float>(h - 44),
        "ROOM 304 - NOVEMBER 14 (LATE AFTERNOON)",
        0.13f, 2.0f, GLUT_STROKE_ROMAN,
        1.0f, 0.86f, 0.48f);

    std::ostringstream statusLine;
    statusLine << std::fixed << std::setprecision(2)
               << "Elijah Pos: (" << m_player.getX() << ", " << m_player.getZ() << ")"
               << " | Interactions: " << m_interactions.getTotalInteractionsTriggered()
               << " | Dialogues Completed: " << m_dialogue.getCompletedDialoguesCount();
    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 70),
        statusLine.str(), GLUT_BITMAP_HELVETICA_12, 0.92f, 0.98f, 0.88f);

    const auto* nearObj = m_interactions.getActiveInteractable();
    std::string targetText = nearObj ? nearObj->displayName : "None (Walk near Maya, Sketchbook, Chalkboard, Window, or Door)";
    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 92),
        "Nearby Target: " + targetText,
        GLUT_BITMAP_HELVETICA_12, 1.0f, 0.86f, 0.55f);

    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 114),
        "Controls: WASD/Arrows Move | [E]/[Enter] Interact & Advance Dialogue | [C] Cam Mode",
        GLUT_BITMAP_HELVETICA_12, 0.80f, 0.88f, 0.96f);

    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 136),
        "OpenGL Keys: [P] Projection | [1-4] Phong Lights | [F3] Debug Colliders | [T] UV Checker",
        GLUT_BITMAP_HELVETICA_12, 0.76f, 0.84f, 0.78f);

    engine::ui::TextRenderer::drawBitmapText2D(
        30.0f, static_cast<float>(h - 158),
        "Objective: Talk to Maya by the window and inspect your Sketchbook on the front-left desk.",
        GLUT_BITMAP_9_BY_15, 0.68f, 0.95f, 0.82f);

    // Render either the active Dialogue Box OR the Proximity Interaction Prompt
    if (m_dialogue.isActive()) {
        m_dialogue.render2D(w, h);
    } else {
        m_interactions.render2DPrompt(w, h);
    }

    engine::camera::Camera3D::end2DOverlay();
}

void ClassroomScene::onKeyPress(unsigned char key) {
    if (key == 'e' || key == 'E' || key == 13 || key == ' ') {
        if (m_dialogue.isActive()) {
            m_dialogue.advanceOrCompleteLine();
            return;
        }
        if (key == 'e' || key == 'E' || key == 13) {
            auto* activeObj = m_interactions.getActiveInteractableMutable();
            if (activeObj) {
                m_interactions.incrementInteractionCount(activeObj);
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
    if (key == GLUT_KEY_F3) {
        m_showDebugOverlay = !m_showDebugOverlay;
    }
}

void ClassroomScene::onMouseClick(int button, int state) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (m_dialogue.isActive()) {
            m_dialogue.advanceOrCompleteLine();
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
