#include "engine/renderer/LightingSystem.h"
#include <cmath>

namespace engine {
namespace renderer {

LightingSystem::LightingSystem()
    : m_lightingEnabled(true)
    , m_sunLightEnabled(true)
    , m_spotLightEnabled(true)
    , m_specularEnabled(true)
{}

void LightingSystem::initClassroomLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);

    // Enable color material for ambient/diffuse when textures + vertex colors are active
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Module 8 Subtopic 1: Global Ambient Light Model
    GLfloat globalAmbient[4] = { 0.30f, 0.26f, 0.28f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
}

void LightingSystem::applyLights(float timeSeconds) {
    if (!m_lightingEnabled) {
        glDisable(GL_LIGHTING);
        return;
    }
    glEnable(GL_LIGHTING);

    // LIGHT0: Warm Golden Sunset Directional/Positional Light coming through left window
    if (m_sunLightEnabled) {
        glEnable(GL_LIGHT0);
        GLfloat sunAmbient[4]  = { 0.22f, 0.17f, 0.14f, 1.0f };
        GLfloat sunDiffuse[4]  = { 0.98f, 0.76f, 0.50f, 1.0f };
        GLfloat sunSpecular[4] = {
            m_specularEnabled ? 0.90f : 0.0f,
            m_specularEnabled ? 0.80f : 0.0f,
            m_specularEnabled ? 0.65f : 0.0f,
            1.0f
        };
        // Homogeneous w = 1.0f for positional light near window
        GLfloat sunPos[4] = { -4.2f, 3.2f, 1.4f, 1.0f };

        glLightfv(GL_LIGHT0, GL_AMBIENT,  sunAmbient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE,  sunDiffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpecular);
        glLightfv(GL_LIGHT0, GL_POSITION, sunPos);
        glLightf(GL_LIGHT0,  GL_SPOT_CUTOFF, 180.0f); // Omnidirectional point light
    } else {
        glDisable(GL_LIGHT0);
    }

    // LIGHT1: Classroom Ceiling Spotlight (Module 8 Spotlight: GL_SPOT_DIRECTION + GL_SPOT_CUTOFF)
    if (m_spotLightEnabled) {
        glEnable(GL_LIGHT1);
        GLfloat spotAmbient[4]  = { 0.05f, 0.05f, 0.07f, 1.0f };
        GLfloat spotDiffuse[4]  = { 0.85f, 0.88f, 0.98f, 1.0f };
        GLfloat spotSpecular[4] = {
            m_specularEnabled ? 1.0f : 0.0f,
            m_specularEnabled ? 1.0f : 0.0f,
            m_specularEnabled ? 1.0f : 0.0f,
            1.0f
        };
        // Gently swaying ceiling lamp spotlight
        float swayX = std::sin(timeSeconds * 0.9f) * 0.25f;
        GLfloat spotPos[4] = { 1.5f, 3.6f, -0.8f, 1.0f };
        GLfloat spotDir[3] = { swayX, -1.0f, 0.0f };

        glLightfv(GL_LIGHT1, GL_AMBIENT,        spotAmbient);
        glLightfv(GL_LIGHT1, GL_DIFFUSE,        spotDiffuse);
        glLightfv(GL_LIGHT1, GL_SPECULAR,       spotSpecular);
        glLightfv(GL_LIGHT1, GL_POSITION,       spotPos);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
        glLightf(GL_LIGHT1,  GL_SPOT_CUTOFF,    38.0f);
    } else {
        glDisable(GL_LIGHT1);
    }
}

void LightingSystem::applyMaterial(const MaterialPreset& mat, GLenum face) {
    // Module 8 Subtopic 1: Material properties via glMaterialfv
    glMaterialfv(face, GL_AMBIENT,   mat.ambient);
    glMaterialfv(face, GL_DIFFUSE,   mat.diffuse);
    glMaterialfv(face, GL_SPECULAR,  mat.specular);
    glMaterialfv(face, GL_EMISSION,  mat.emission);
    glMaterialfv(face, GL_SHININESS, &mat.shininess);
}

MaterialPreset LightingSystem::matteWoodMaterial() {
    MaterialPreset m = {
        { 0.25f, 0.20f, 0.16f, 1.0f },
        { 0.85f, 0.78f, 0.70f, 1.0f },
        { 0.18f, 0.15f, 0.12f, 1.0f },
        { 0.00f, 0.00f, 0.00f, 1.0f },
        16.0f
    };
    return m;
}

MaterialPreset LightingSystem::polishedCeramicMaterial() {
    MaterialPreset m = {
        { 0.25f, 0.22f, 0.18f, 1.0f },
        { 0.92f, 0.82f, 0.55f, 1.0f },
        { 0.95f, 0.95f, 0.90f, 1.0f },
        { 0.02f, 0.02f, 0.01f, 1.0f },
        85.0f
    };
    return m;
}

MaterialPreset LightingSystem::glowingLampMaterial() {
    MaterialPreset m = {
        { 0.30f, 0.28f, 0.20f, 1.0f },
        { 1.00f, 0.95f, 0.75f, 1.0f },
        { 0.50f, 0.50f, 0.50f, 1.0f },
        { 0.65f, 0.55f, 0.30f, 1.0f },
        32.0f
    };
    return m;
}

} // namespace renderer
} // namespace engine
