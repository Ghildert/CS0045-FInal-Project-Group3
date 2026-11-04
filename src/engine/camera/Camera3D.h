#ifndef ENGINE_CAMERA_CAMERA3D_H
#define ENGINE_CAMERA_CAMERA3D_H

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <GL/glu.h>
#include <string>

namespace engine {
namespace camera {

enum class ProjectionMode {
    PerspectiveGLU = 0,     // Module 7: gluPerspective
    PerspectiveFrustum = 1, // Module 7: glFrustum
    Orthographic3D = 2      // Module 7: glOrtho
};

class Camera3D {
public:
    Camera3D();

    void setViewport(int width, int height);
    void applyProjection() const;
    void applyView() const;

    // Module 7: 2D Orthographic UI Overlay using gluOrtho2D
    static void begin2DOverlay(int width, int height);
    static void end2DOverlay();

    // Camera controls
    void updateFreeOrbit(float dt);
    void setNarrativeTarget(float targetX, float targetY, float targetZ, float dt);

    void cycleProjectionMode();
    ProjectionMode getProjectionMode() const { return m_projMode; }
    std::string getProjectionModeName() const;

    void setFreeCameraEnabled(bool enabled) { m_freeCamera = enabled; }
    bool isFreeCameraEnabled() const { return m_freeCamera; }
    void resetToDefault25D();

    float getEyeX() const { return m_eyeX; }
    float getEyeY() const { return m_eyeY; }
    float getEyeZ() const { return m_eyeZ; }
    float getTargetX() const { return m_targetX; }
    float getTargetY() const { return m_targetY; }
    float getTargetZ() const { return m_targetZ; }
    float getYawDegrees() const { return m_yawDegrees; }
    int getViewportWidth() const { return m_viewportWidth; }
    int getViewportHeight() const { return m_viewportHeight; }

private:
    void recomputeEyeFromSpherical();

    int            m_viewportWidth;
    int            m_viewportHeight;
    ProjectionMode m_projMode;
    bool           m_freeCamera;

    float m_fovDegrees;
    float m_nearPlane;
    float m_farPlane;
    float m_orthoScale;

    // LookAt state
    float m_eyeX, m_eyeY, m_eyeZ;
    float m_targetX, m_targetY, m_targetZ;
    float m_upX, m_upY, m_upZ;

    // Orbit parameters (Yaw, Pitch, Distance)
    float m_yawDegrees;
    float m_pitchDegrees;
    float m_distance;
};

} // namespace camera
} // namespace engine

#endif // ENGINE_CAMERA_CAMERA3D_H
