cmake_minimum_required(VERSION 3.16)
project(NarrativeGame VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure GLEW static linking works cleanly alongside FreeGLUT on MSYS2 UCRT64
add_compile_definitions(GLEW_STATIC)

# Collect source files (Phase 0 starts with src/main.cpp; later phases add subsystem files)
set(GAME_SOURCES
    src/main.cpp
)

add_executable(NarrativeGame ${GAME_SOURCES})

target_include_directories(NarrativeGame PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    "C:/msys64/ucrt64/include"
)

target_link_directories(NarrativeGame PRIVATE
    "C:/msys64/ucrt64/lib"
)

# Link against MSYS2 UCRT64 static/import libraries for GLEW, FreeGLUT, GLU, and OpenGL
target_link_libraries(NarrativeGame PRIVATE
    glew32
    freeglut
    glu32
    opengl32
    gdi32
    winmm
)
