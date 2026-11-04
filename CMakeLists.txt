cmake_minimum_required(VERSION 3.16)
project(EchoesOfNovember VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for IDE intelligence
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Required preprocessor definitions for static GLEW and FreeGLUT on Windows
add_compile_definitions(GLEW_STATIC)

# Include project headers
include_directories(
    ${CMAKE_SOURCE_DIR}/src
)

set(ENGINE_SOURCES
    src/engine/core/Logger.cpp
    src/engine/core/Math4x4.cpp
    src/engine/input/InputManager.cpp
    src/engine/assets/Texture2D.cpp
    src/engine/animation/SpriteAnimator.cpp
    src/engine/renderer/PrimitiveRenderer.cpp
    src/engine/renderer/MeshVBO.cpp
    src/engine/renderer/SpriteQuad3D.cpp
    src/engine/renderer/LightingSystem.cpp
    src/engine/collision/CollisionSystem.cpp
    src/engine/camera/Camera3D.cpp
    src/engine/scene/SceneTransition.cpp
    src/engine/ui/TextRenderer.cpp
)

set(GAME_SOURCES
    src/game/player/PlayerController.cpp
    src/game/characters/NPCCharacter.cpp
    src/game/interactions/InteractionSystem.cpp
    src/game/dialogue/DialogueSystem.cpp
    src/game/narrative/StoryState.cpp
    src/game/narrative/PhoneSystem.cpp
    src/game/minigames/RhythmMinigame.cpp
    src/game/scenes/ClassroomScene.cpp
    src/debug/OpenGLDemoMode.cpp
)

add_executable(NarrativeGame
    src/main.cpp
    ${ENGINE_SOURCES}
    ${GAME_SOURCES}
)

if(MINGW)
    target_compile_options(NarrativeGame PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Link OpenGL compatibility libraries, GLEW, FreeGLUT, GLU, GDI32, and WinMM (audio/timers)
target_link_libraries(NarrativeGame PRIVATE
    glew32
    freeglut
    glu32
    opengl32
    gdi32
    winmm
)
