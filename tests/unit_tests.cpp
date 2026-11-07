/**
 * CS0045 Computer Graphics and Visual Computing
 * "Echoes of November" - Automated Unit Test Suite (Phase 8 Verification)
 *
 * Tests:
 * 1. Matrix and transformation calculations (Module 6 Mat4 & homogeneous Vec4)
 * 2. Bounding-box collision & sliding resolution logic (AABB3D & CollisionWorld)
 * 3. Sprite-sheet UV frame indexing & horizontal flipping (SpriteAnimator)
 * 4. Dialogue state transitions & branching choices from external data (DialogueSystem)
 * 5. Story flag persistence & Save/Load round-trip (StoryState & PhoneSystem)
 * 6. Asset file generation & path verification (Texture2D & AudioSystem)
 */

#include "engine/core/Math4x4.h"
#include "engine/collision/CollisionSystem.h"
#include "engine/animation/SpriteAnimator.h"
#include "engine/assets/Texture2D.h"
#include "engine/audio/AudioSystem.h"
#include "game/narrative/StoryState.h"
#include "game/narrative/PhoneSystem.h"
#include "game/dialogue/DialogueSystem.h"
#include "game/scenes/ClassroomScene.h"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

int g_testsPassed = 0;
int g_testsFailed = 0;

void checkTrue(bool condition, const std::string& testName) {
    if (condition) {
        ++g_testsPassed;
        std::cout << "  [PASS] " << testName << std::endl;
    } else {
        ++g_testsFailed;
        std::cerr << "  [FAIL] " << testName << std::endl;
    }
}

bool nearEq(float a, float b, float eps = 1e-4f) {
    return std::fabs(a - b) <= eps;
}

// ============================================================================
// 1. Matrix & Homogeneous Coordinate Tests (Module 6)
// ============================================================================
void testMatrixAndTransformations() {
    using engine::core::Mat4;
    using engine::core::Vec4;

    std::cout << "[Suite 1] Matrix & Homogeneous Coordinate Operations (Module 6)" << std::endl;

    Mat4 I = Mat4::identity();
    Mat4 T = Mat4::translation(3.0f, -2.0f, 5.5f);

    // Position (w=1) is affected by translation
    Vec4 pos = Vec4::point(1.0f, 2.0f, 0.5f);
    Vec4 posOut = Mat4::multiplyVec4(T, pos);
    checkTrue(nearEq(posOut.x, 4.0f) && nearEq(posOut.y, 0.0f) && nearEq(posOut.z, 6.0f) && nearEq(posOut.w, 1.0f),
              "Mat4::translation translates homogeneous point (w=1)");

    // Direction (w=0) is unaffected by translation
    Vec4 dir = Vec4::direction(1.0f, 2.0f, 0.5f);
    Vec4 dirOut = Mat4::multiplyVec4(T, dir);
    checkTrue(nearEq(dirOut.x, 1.0f) && nearEq(dirOut.y, 2.0f) && nearEq(dirOut.z, 0.5f) && nearEq(dirOut.w, 0.0f),
              "Mat4::translation preserves homogeneous direction vector (w=0)");

    // Matrix Addition, Subtraction, and Scalar Multiplication
    Mat4 sum = Mat4::add(I, I);
    Mat4 scaled = Mat4::multiplyScalar(I, 2.0f);
    Mat4 diff = Mat4::subtract(sum, scaled);
    checkTrue(nearEq(diff.m[0][0], 0.0f) && nearEq(diff.m[1][1], 0.0f) && nearEq(diff.m[3][3], 0.0f),
              "Mat4::add, subtract, and multiplyScalar consistency");

    // Matrix-Matrix Multiplication (Scaling followed by Translation)
    Mat4 S = Mat4::scaling(2.0f, 3.0f, 4.0f);
    Mat4 TS = Mat4::multiply(T, S);
    Vec4 pScaledTrans = Mat4::multiplyVec4(TS, Vec4::point(1.0f, 1.0f, 1.0f));
    checkTrue(nearEq(pScaledTrans.x, 5.0f) && nearEq(pScaledTrans.y, 1.0f) && nearEq(pScaledTrans.z, 9.5f),
              "Mat4::multiply combines Scaling and Translation matrices");

    // 90-degree Z Rotation
    Mat4 Rz = Mat4::rotationZ(90.0f);
    Vec4 rotOut = Mat4::multiplyVec4(Rz, Vec4::point(1.0f, 0.0f, 0.0f));
    checkTrue(nearEq(rotOut.x, 0.0f) && nearEq(rotOut.y, 1.0f),
              "Mat4::rotationZ(90 deg) rotates (1,0,0) to (0,1,0)");
}

// ============================================================================
// 2. Bounding-Box Collision & Sliding Resolution Tests
// ============================================================================
void testCollisionSystem() {
    using engine::collision::AABB3D;
    using engine::collision::CollisionWorld;

    std::cout << "[Suite 2] 3D AABB Collision & Axis-Separated Sliding" << std::endl;

    AABB3D desk = AABB3D::fromCenterSize(0.0f, 0.5f, 0.0f, 2.0f, 1.0f, 1.0f, "Desk");
    AABB3D overlapBox = AABB3D::fromCenterSize(0.5f, 0.5f, 0.2f, 0.5f, 1.0f, 0.5f, "Player");
    AABB3D farBox = AABB3D::fromCenterSize(3.0f, 0.5f, 3.0f, 0.5f, 1.0f, 0.5f, "Far");

    checkTrue(desk.intersects(overlapBox) && desk.intersectsXZ(overlapBox),
              "AABB3D::intersects detects overlapping 3D boxes");
    checkTrue(!desk.intersects(farBox) && !desk.intersectsXZ(farBox),
              "AABB3D::intersects rejects separated 3D boxes");

    CollisionWorld world;
    world.addBoxCollider(0.0f, 0.5f, 0.0f, 2.0f, 1.0f, 1.0f, "CenterDesk");

    // Place player just in front of desk (+Z = 0.75) and move diagonally (-Z into desk, +X along desk)
    float px = 0.0f;
    float pz = 0.75f;
    std::string hitTag;
    bool hit = world.resolveMovementXZ(px, pz, 0.35f, -0.35f, 0.20f, 0.20f, &hitTag);
    checkTrue(hit && hitTag == "CenterDesk" && nearEq(px, 0.35f) && nearEq(pz, 0.75f),
              "CollisionWorld::resolveMovementXZ blocks Z penetration while sliding smoothly along X");
}

// ============================================================================
// 3. Sprite-Sheet UV Frame Indexing Tests
// ============================================================================
void testSpriteAnimator() {
    using engine::animation::AnimState;
    using engine::animation::SpriteAnimator;

    std::cout << "[Suite 3] Sprite-Sheet UV Frame Indexing & Orientation" << std::endl;

    SpriteAnimator anim(4, 2);
    auto uvIdle0 = anim.getCurrentUVRect();
    checkTrue(nearEq(uvIdle0.u0, 0.0f) && nearEq(uvIdle0.u1, 0.25f) &&
              nearEq(uvIdle0.v0, 0.5f) && nearEq(uvIdle0.v1, 1.0f),
              "Idle Frame 0 maps to top row [u: 0..0.25, v: 0.5..1.0]");

    anim.setState(AnimState::Walk);
    anim.update(0.15f); // Advance 1 walk frame (walk frame duration = 0.14s)
    checkTrue(anim.getCurrentFrame() == 1, "Walk state advances frame index after 0.15s");

    anim.setFlipHorizontal(true);
    auto uvWalk1Flip = anim.getCurrentUVRect();
    checkTrue(nearEq(uvWalk1Flip.u0, 0.50f) && nearEq(uvWalk1Flip.u1, 0.25f) &&
              nearEq(uvWalk1Flip.v0, 0.0f)  && nearEq(uvWalk1Flip.v1, 0.5f),
              "Horizontal flip swaps u0/u1 on Walk Row [u: 0.50..0.25, v: 0.0..0.5]");
}

// ============================================================================
// 4. External Dialogue & Branching Choice Tests
// ============================================================================
void testDialogueAndStoryData() {
    using game::dialogue::DialogueSystem;
    using game::narrative::StoryState;

    std::cout << "[Suite 4] External Dialogue Parsing & Branching Choices" << std::endl;

    StoryState story;
    DialogueSystem dlg;
    dlg.loadFromExternalData("data/dialogue/story_dialogues.txt");

    bool started = dlg.startDialogue("maya_classroom_talk");
    checkTrue(started && dlg.isActive(), "DialogueSystem::startDialogue('maya_classroom_talk') loads external node");

    // Advance to the choice prompt at the end of maya_classroom_talk
    for (int step = 0; step < 8 && !dlg.isWaitingForChoice(); ++step) {
        dlg.advanceOrCompleteLine(&story);
    }
    checkTrue(dlg.isWaitingForChoice(), "DialogueSystem reaches branching choice prompt");

    bool branched = dlg.selectChoice(0, &story);
    checkTrue(branched && dlg.getActiveSequenceId() == "maya_branch_promise" &&
              story.hasFlag("promised_rooftop_rehearsal"),
              "Selecting Branch Choice 0 transitions to 'maya_branch_promise' and sets story flag");
}

// ============================================================================
// 5. Story Flag Persistence & Save/Load Round-Trip Tests
// ============================================================================
void testSaveLoadAndPhone() {
    using game::narrative::PhoneSystem;
    using game::narrative::StoryState;

    std::cout << "[Suite 5] Story Flag Persistence, Smartphone Replies & Save/Load" << std::endl;

    StoryState stateOut;
    stateOut.setFlag("has_sketchbook", 1);
    stateOut.setFlag("talked_to_maya", 1);
    checkTrue(stateOut.hasFlag("hallway_unlocked"),
              "Setting has_sketchbook + talked_to_maya automatically unlocks hallway_unlocked");

    PhoneSystem phone;
    phone.loadFromExternalData("data/dialogue/phone_messages.txt");
    phone.selectChoice(0, stateOut);
    checkTrue(stateOut.hasFlag("replied_phone_sam"),
              "PhoneSystem::selectChoice(0) sends reply and sets replied_phone_sam flag");

    const std::string testSavePath = "data/saves/unit_test_slot.sav";
    bool saved = stateOut.saveToFile(testSavePath, "hallway", -2.25f, 0.75f);

    StoryState stateIn;
    std::string loadedScene;
    float lx = 0.0f, lz = 0.0f;
    bool loaded = stateIn.loadFromFile(testSavePath, loadedScene, lx, lz);

    checkTrue(saved && loaded && loadedScene == "hallway" &&
              nearEq(lx, -2.25f) && nearEq(lz, 0.75f) &&
              stateIn.hasFlag("hallway_unlocked") && stateIn.hasFlag("replied_phone_sam"),
              "StoryState::saveToFile and loadFromFile round-trip preserves scene, position, and flags");

    std::error_code ec;
    std::filesystem::remove(testSavePath, ec);
}

// ============================================================================
// 6. Asset Generation & Disk File Verification
// ============================================================================
void testAssetFilesOnDisk() {
    std::cout << "[Suite 6] Texture, Sprite-Sheet, Portrait & Audio File Verification" << std::endl;

    engine::assets::Texture2D::ensureDefaultTexturesOnDisk();
    engine::audio::AudioSystem::ensureDefaultSoundsOnDisk();

    const char* requiredFiles[] = {
        "assets/textures/wood_floor.png",
        "assets/textures/classroom_wall.png",
        "assets/textures/desk_wood.png",
        "assets/textures/chalkboard.png",
        "assets/textures/window_glass.png",
        "assets/textures/checker_debug.png",
        "assets/textures/hallway_tile.png",
        "assets/textures/school_lockers.png",
        "assets/textures/wet_asphalt.png",
        "assets/textures/storefront_neon.png",
        "assets/sprites/player_elijah_sheet.png",
        "assets/sprites/npc_maya_sheet.png",
        "assets/sprites/npc_sam_sheet.png",
        "assets/sprites/portrait_elijah.png",
        "assets/sprites/portrait_maya.png",
        "assets/sprites/portrait_sam.png",
        "assets/sounds/voice_maya.wav",
        "assets/sounds/voice_elijah.wav",
        "assets/sounds/voice_sam.wav",
        "assets/sounds/voice_narrator.wav",
        "assets/sounds/ui_blip.wav",
        "assets/sounds/dialogue_tick.wav",
        "assets/sounds/chord_strum.wav",
        "assets/sounds/save_chime.wav",
        "assets/sounds/bgm_theme.mp3",
        "data/dialogue/story_dialogues.txt",
        "data/dialogue/phone_messages.txt"
    };

    bool allExist = true;
    for (const char* path : requiredFiles) {
        if (!std::filesystem::exists(path) || std::filesystem::file_size(path) == 0) {
            allExist = false;
            std::cerr << "  Missing or empty asset file: " << path << std::endl;
        }
    }
    checkTrue(allExist, "All 27 required PNG textures, sprite sheets, portraits, WAV/MP3 audio, and dialogue files exist on disk");
}

// ============================================================================
// 7. Developer Mode Key Combination, Scene Teleport & Task Bypass Tests
// ============================================================================
void testDevModeKeyComboAndBypass() {
    using game::scenes::ClassroomScene;
    using game::scenes::LocationId;

    std::cout << "[Suite 7] Developer Mode Key-Combination, Teleport & Task Bypass" << std::endl;

    ClassroomScene scene;
    checkTrue(!scene.isDevModeUnlocked() && !scene.isDevModeOpen(),
              "Dev Mode starts locked and closed by default");

    // Activate Dev Mode by typing the sequential key combination 'd' -> 'e' -> 'v'
    scene.onKeyPress('d');
    scene.onKeyPress('e');
    scene.onKeyPress('v');
    checkTrue(scene.isDevModeUnlocked() && scene.isDevModeOpen(),
              "Key combination 'D' -> 'E' -> 'V' unlocks and opens Dev Mode Console");

    // Press '4' inside Dev Mode Console to bypass the current stage task (has_sketchbook)
    scene.onKeyPress('4');
    checkTrue(scene.getStoryState().hasFlag("has_sketchbook"),
              "Dev Mode [4] bypasses current stage task (sets has_sketchbook=1)");

    // Press '5' inside Dev Mode Console to bypass ALL tasks & unlock all scenes
    scene.onKeyPress('5');
    checkTrue(scene.getStoryState().hasFlag("talked_to_maya") &&
              scene.getStoryState().hasFlag("talked_to_sam") &&
              scene.getStoryState().hasFlag("completed_rhythm_minigame") &&
              scene.getStoryState().hasFlag("story_act1_complete"),
              "Dev Mode [5] bypasses all tasks across Stages 1-3");

    // Press '3' inside Dev Mode Console to teleport directly to Scene 3 (937 P. Paredes St)
    scene.onKeyPress('3');
    checkTrue(scene.getCurrentLocation() == LocationId::RainyStreet && !scene.isDevModeOpen(),
              "Dev Mode [3] teleports player to LocationId::RainyStreet and closes console");
}

} // anonymous namespace

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << " Echoes of November - Automated Unit Test Suite"            << std::endl;
    std::cout << "==========================================================" << std::endl;

    testMatrixAndTransformations();
    testCollisionSystem();
    testSpriteAnimator();
    testDialogueAndStoryData();
    testSaveLoadAndPhone();
    testAssetFilesOnDisk();
    testDevModeKeyComboAndBypass();

    std::cout << "==========================================================" << std::endl;
    std::cout << " Unit Tests Summary: " << g_testsPassed << " Passed, "
              << g_testsFailed << " Failed" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return (g_testsFailed == 0) ? 0 : 1;
}
