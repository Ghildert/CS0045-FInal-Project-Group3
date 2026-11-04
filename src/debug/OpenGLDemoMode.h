#ifndef DEBUG_OPENGLDEMOMODE_H
#define DEBUG_OPENGLDEMOMODE_H

#include "engine/assets/Texture2D.h"
#include "engine/renderer/MeshVBO.h"
#include "engine/renderer/LightingSystem.h"
#include "engine/camera/Camera3D.h"
#include <string>

namespace debug {

class OpenGLDemoMode {
public:
    OpenGLDemoMode();
    ~OpenGLDemoMode();

    void init();
    void destroy();

    void toggleActive();
    void setActive(bool active) { m_active = active; }
    bool isActive() const { return m_active; }

    void selectModule(int moduleNumber1To8);
    void nextModule();
    void prevModule();
    int  getCurrentModuleNumber() const { return m_currentModule; } // 1..8

    void update(float dt);
    void render3D(const engine::camera::Camera3D& camera);
    void render2D(const engine::camera::Camera3D& camera);

    void onKeyPress(unsigned char key, engine::camera::Camera3D& camera);
    void onSpecialKeyPress(int key);

    int getModulesVisitedMask() const { return m_visitedMask; }

private:
    void renderModule1_3D();
    void renderModule2_3D();
    void renderModule3_3D();
    void renderModule4_3D();
    void renderModule5_3D();
    void renderModule6_3D();
    void renderModule7_3D();
    void renderModule8_3D();

    bool                              m_active;
    int                               m_currentModule; // 1..8
    int                               m_visitedMask;
    float                             m_timeSeconds;
    bool                              m_wireframeMode;
    bool                              m_useCheckerTex;
    int                               m_materialPresetIdx;

    engine::renderer::LightingSystem  m_demoLighting;
    engine::assets::Texture2D         m_texWood;
    engine::assets::Texture2D         m_texChecker;
    engine::assets::Texture2D         m_texSpriteSheet;

    engine::renderer::MeshVBO         m_meshBoxStatic;
    engine::renderer::MeshVBO         m_meshPyramidDynamic;
};

} // namespace debug

#endif // DEBUG_OPENGLDEMOMODE_H
