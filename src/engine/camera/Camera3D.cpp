#include "engine/camera/Camera3D.h"
#include "engine/input/InputManager.h"
#include <GL/freeglut.h>
#include <cmath>

namespace engine {
namespace camera {

static constexpr float kPi = 3.14159265358979323846f;

Camera3D::Camera3D()
    : m_viewportWidth(1280)
    , m_viewportHeight(720)
    , m_projMode(ProjectionMode::PerspectiveGLU)
    , m_freeCamera(false)
    , m_fovDegrees(42.0f)
    , m_nearPlane(0.1f)
    , m_farPlane(100.0f)
    , m_orthoScale(3.6f)
    , m_eyeX(0.0f), m_eyeY(2.3f), m_eyeZ(6.8f)
    , m_targetX(0.0f), m_targetY(1.2f), m_targetZ(0.0f)
    , m_upX(0.0f), m_upY(1.0f), m_upZ(0.0f)
    , m_yawDegrees(0.0f)
    , m_pitchDegrees(14.0f)
    , m_distance(6.8f)
{
    recomputeEyeFromSpherical();
}

void Camera3D::resetToDefault25D() {
    m_targetX      = 0.0f;
    m_targetY      = 1.25f;
    m_targetZ      = 0.0f;
    m_yawDegrees   = 0.0f;
    m_pitchDegrees = 13.5f;
    m_distance     = 6.8f;
    recomputeEyeFromSpherical();
}

void Camera3D::recomputeEyeFromSpherical() {
    float yawRad   = m_yawDegrees   * (kPi / 180.0f);
    float pitchRad = m_pitchDegrees * (kPi / 180.0f);

    float horizDist = m_distance * std::cos(pitchRad);
    m_eyeX = m_targetX + horizDist * std::sin(yawRad);
    m_eyeY = m_targetY + m_distance * std::sin(pitchRad);
    m_eyeZ = m_targetZ + horizDist * std::cos(yawRad);
}

void Camera3D::setViewport(int width, int height) {
    if (height <= 0) {
        height = 1;
    }
    m_viewportWidth  = width;
    m_viewportHeight = height;
    // Module 7 Subtopic 2: Viewport control
    glViewport(0, 0, m_viewportWidth, m_viewportHeight);
}

void Camera3D::applyProjection() const {
    float aspect = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (m_projMode == ProjectionMode::PerspectiveGLU) {
        // Module 7 Subtopic 2: Perspective projection via gluPerspective
        gluPerspective(m_fovDegrees, aspect, m_nearPlane, m_farPlane);
    } else if (m_projMode == ProjectionMode::PerspectiveFrustum) {
        // Module 7 Subtopic 2: Perspective projection via glFrustum
        float top    = m_nearPlane * std::tan((m_fovDegrees * 0.5f) * (kPi / 180.0f));
        float bottom = -top;
        float right  = top * aspect;
        float left   = -right;
        glFrustum(left, right, bottom, top, m_nearPlane, m_farPlane);
    } else {
        // Module 7 Subtopic 2: 3D Orthographic projection via glOrtho
        float halfH = m_orthoScale;
        float halfW = halfH * aspect;
        glOrtho(-halfW, halfW, -halfH, halfH, -25.0, 100.0);
    }

    glMatrixMode(GL_MODELVIEW);
}

void Camera3D::applyView() const {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Module 7 Subtopic 3: Camera positioning and orientation via gluLookAt
    gluLookAt(
        m_eyeX,    m_eyeY,    m_eyeZ,
        m_targetX, m_targetY, m_targetZ,
        m_upX,     m_upY,     m_upZ);
}

void Camera3D::begin2DOverlay(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // Module 7 Subtopic 2: 2D Orthographic projection via gluOrtho2D
    gluOrtho2D(0.0, static_cast<GLdouble>(width), 0.0, static_cast<GLdouble>(height));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void Camera3D::end2DOverlay() {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}

void Camera3D::updateFreeOrbit(float dt) {
    using input::InputManager;

    const float panSpeed   = 3.2f * dt;
    const float orbitSpeed = 65.0f * dt;
    const float zoomSpeed  = 4.5f * dt;

    // In Free Camera mode, Arrow keys orbit camera yaw and pitch
    if (m_freeCamera) {
        if (InputManager::isSpecialDown(GLUT_KEY_LEFT)) {
            m_yawDegrees -= orbitSpeed;
        }
        if (InputManager::isSpecialDown(GLUT_KEY_RIGHT)) {
            m_yawDegrees += orbitSpeed;
        }
        if (InputManager::isSpecialDown(GLUT_KEY_UP)) {
            m_pitchDegrees += orbitSpeed * 0.7f;
        }
        if (InputManager::isSpecialDown(GLUT_KEY_DOWN)) {
            m_pitchDegrees -= orbitSpeed * 0.7f;
        }
    }

    // Right mouse drag orbits camera smoothly in any mode
    if (InputManager::isMouseButtonDown(GLUT_RIGHT_BUTTON)) {
        m_yawDegrees   -= static_cast<float>(InputManager::getMouseDeltaX()) * 0.35f;
        m_pitchDegrees += static_cast<float>(InputManager::getMouseDeltaY()) * 0.25f;
    }

    if (m_pitchDegrees < 3.0f)  m_pitchDegrees = 3.0f;
    if (m_pitchDegrees > 75.0f) m_pitchDegrees = 75.0f;

    // Q/E zooms in/out in any mode; WASD pans target only in free camera mode
    if (InputManager::isKeyDown('q')) m_distance -= zoomSpeed;
    if (InputManager::isKeyDown('e')) m_distance += zoomSpeed;

    if (m_freeCamera) {
        if (InputManager::isKeyDown('a')) m_targetX -= panSpeed;
        if (InputManager::isKeyDown('d')) m_targetX += panSpeed;
        if (InputManager::isKeyDown('w')) m_targetZ -= panSpeed;
        if (InputManager::isKeyDown('s')) m_targetZ += panSpeed;
    }

    if (m_distance < 2.2f)  m_distance = 2.2f;
    if (m_distance > 16.0f) m_distance = 16.0f;

    recomputeEyeFromSpherical();
}

void Camera3D::setNarrativeTarget(float targetX, float targetY, float targetZ, float dt) {
    float smooth = (dt * 5.0f > 1.0f) ? 1.0f : (dt * 5.0f);
    m_targetX += (targetX - m_targetX) * smooth;
    m_targetY += (targetY - m_targetY) * smooth;
    m_targetZ += (targetZ - m_targetZ) * smooth;
    recomputeEyeFromSpherical();
}

void Camera3D::cycleProjectionMode() {
    int next = (static_cast<int>(m_projMode) + 1) % 3;
    m_projMode = static_cast<ProjectionMode>(next);
}

std::string Camera3D::getProjectionModeName() const {
    switch (m_projMode) {
        case ProjectionMode::PerspectiveGLU:
            return "Perspective (gluPerspective)";
        case ProjectionMode::PerspectiveFrustum:
            return "Perspective (glFrustum)";
        case ProjectionMode::Orthographic3D:
            return "Orthographic (glOrtho)";
    }
    return "Unknown";
}

} // namespace camera
} // namespace engine
