#ifndef ENGINE_RENDERER_LIGHTINGSYSTEM_H
#define ENGINE_RENDERER_LIGHTINGSYSTEM_H

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>

namespace engine {
namespace renderer {

struct MaterialPreset {
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat emission[4];
    GLfloat shininess;
};

class LightingSystem {
public:
    LightingSystem();

    void initClassroomLighting();
    void applyLights(float timeSeconds);

    // Module 8 Subtopic 1: Apply material properties via glMaterialfv
    static void applyMaterial(const MaterialPreset& mat, GLenum face = GL_FRONT_AND_BACK);
    static MaterialPreset matteWoodMaterial();
    static MaterialPreset polishedCeramicMaterial();
    static MaterialPreset glowingLampMaterial();

    // Runtime component toggles for academic inspection
    void toggleLightingEnabled() { m_lightingEnabled = !m_lightingEnabled; }
    void toggleSunLight()        { m_sunLightEnabled = !m_sunLightEnabled; }
    void toggleSpotLight()       { m_spotLightEnabled = !m_spotLightEnabled; }
    void toggleSpecular()        { m_specularEnabled = !m_specularEnabled; }

    bool isLightingEnabled() const { return m_lightingEnabled; }
    bool isSunLightEnabled() const { return m_sunLightEnabled; }
    bool isSpotLightEnabled() const { return m_spotLightEnabled; }
    bool isSpecularEnabled() const { return m_specularEnabled; }

private:
    bool m_lightingEnabled;
    bool m_sunLightEnabled;
    bool m_spotLightEnabled;
    bool m_specularEnabled;
};

} // namespace renderer
} // namespace engine

#endif // ENGINE_RENDERER_LIGHTINGSYSTEM_H
