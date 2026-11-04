#ifndef ENGINE_RENDERER_SPRITEQUAD3D_H
#define ENGINE_RENDERER_SPRITEQUAD3D_H

#include "engine/renderer/MeshVBO.h"
#include "engine/animation/SpriteAnimator.h"
#include "engine/assets/Texture2D.h"

namespace engine {
namespace renderer {

class SpriteQuad3D {
public:
    SpriteQuad3D();
    ~SpriteQuad3D();

    void init(float width = 0.85f, float height = 1.32f);
    void updateUVs(const animation::UVRect& uv);

    void draw(
        const assets::Texture2D& spriteSheet,
        float worldX, float worldY, float worldZ,
        float billboardYawDegrees = 0.0f,
        bool drawShadow = true) const;

    void destroy();
    bool isValid() const { return m_quadMesh.isValid(); }

private:
    static void drawContactShadow(float radiusX, float radiusZ);

    float   m_width;
    float   m_height;
    MeshVBO m_quadMesh;
};

} // namespace renderer
} // namespace engine

#endif // ENGINE_RENDERER_SPRITEQUAD3D_H
