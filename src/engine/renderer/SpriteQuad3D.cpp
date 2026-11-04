#include "engine/renderer/SpriteQuad3D.h"
#include <cmath>

namespace engine {
namespace renderer {

SpriteQuad3D::SpriteQuad3D()
    : m_width(0.85f)
    , m_height(1.32f)
{}

SpriteQuad3D::~SpriteQuad3D() {
    destroy();
}

void SpriteQuad3D::destroy() {
    m_quadMesh.destroy();
}

void SpriteQuad3D::init(float width, float height) {
    m_width  = width;
    m_height = height;

    float hx = m_width * 0.5f;
    // Bottom of quad sits at y = 0.0f, top at y = m_height
    std::vector<Vertex3D> verts = {
        { -hx, 0.0f,     0.0f,   0.0f, 0.2f, 0.98f,   1.0f, 1.0f, 1.0f,   0.0f,  0.5f },
        {  hx, 0.0f,     0.0f,   0.0f, 0.2f, 0.98f,   1.0f, 1.0f, 1.0f,   0.25f, 0.5f },
        {  hx, m_height, 0.0f,   0.0f, 0.2f, 0.98f,   1.0f, 1.0f, 1.0f,   0.25f, 1.0f },
        { -hx, m_height, 0.0f,   0.0f, 0.2f, 0.98f,   1.0f, 1.0f, 1.0f,   0.0f,  1.0f }
    };
    std::vector<GLushort> indices = { 0, 1, 2, 3 };
    m_quadMesh.uploadToGPU(verts, indices, GL_QUADS, GL_DYNAMIC_DRAW);
}

void SpriteQuad3D::updateUVs(const animation::UVRect& uv) {
    float hx = m_width * 0.5f;
    std::vector<Vertex3D> verts = {
        { -hx, 0.0f,     0.0f,   0.0f, 0.2f, 0.98f,   1.0f, 1.0f, 1.0f,   uv.u0, uv.v0 },
        {  hx, 0.0f,     0.0f,   0.0f, 0.2f, 0.98f,   1.0f, 1.0f, 1.0f,   uv.u1, uv.v0 },
        {  hx, m_height, 0.0f,   0.0f, 0.2f, 0.98f,   1.0f, 1.0f, 1.0f,   uv.u1, uv.v1 },
        { -hx, m_height, 0.0f,   0.0f, 0.2f, 0.98f,   1.0f, 1.0f, 1.0f,   uv.u0, uv.v1 }
    };
    // Module 6 Subtopic 2: Dynamic VBO partial update via glBufferSubData
    m_quadMesh.updateSubData(0, verts);
}

void SpriteQuad3D::drawContactShadow(float radiusX, float radiusZ) {
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Module 2: GL_TRIANGLE_FAN soft grounded shadow ellipse
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.03f, 0.02f, 0.05f, 0.55f);
    glVertex3f(0.0f, 0.008f, 0.0f);

    glColor4f(0.03f, 0.02f, 0.05f, 0.05f);
    constexpr int kSegments = 16;
    constexpr float kTwoPi  = 6.28318530718f;
    for (int i = 0; i <= kSegments; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(kSegments)) * kTwoPi;
        glVertex3f(std::cos(angle) * radiusX, 0.008f, std::sin(angle) * radiusZ);
    }
    glEnd();

    glPopAttrib();
}

void SpriteQuad3D::draw(
    const assets::Texture2D& spriteSheet,
    float worldX, float worldY, float worldZ,
    float billboardYawDegrees,
    bool drawShadow) const
{
    glPushMatrix();
    glTranslatef(worldX, worldY, worldZ);

    if (drawShadow) {
        drawContactShadow(m_width * 0.36f, m_width * 0.22f);
    }

    glRotatef(billboardYawDegrees, 0.0f, 1.0f, 0.0f);

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    spriteSheet.bind();
    m_quadMesh.drawVBOElements();
    assets::Texture2D::unbind();

    glPopAttrib();
    glPopMatrix();
}

} // namespace renderer
} // namespace engine
