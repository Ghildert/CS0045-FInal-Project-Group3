#ifndef ENGINE_RENDERER_PRIMITIVERENDERER_H
#define ENGINE_RENDERER_PRIMITIVERENDERER_H

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>

namespace engine {
namespace renderer {

/**
 * Module 1 & Module 2: Geometric Primitives, Colors, and Line Stippling
 */
class PrimitiveRenderer {
public:
    // Module 2 Subtopic 2: Line Stipple Patterns from Lecture Slides
    static constexpr GLushort kStippleDashed  = 0x00FF;
    static constexpr GLushort kStippleDashDot = 0x0C0F;
    static constexpr GLushort kStippleDotted  = 0xAAAA;

    // 3D Reference Grid & Coordinate Axes (GL_LINES + Line Stippling)
    static void drawReferenceGrid(float halfExtent = 6.0f, float step = 1.0f);
    static void drawCoordinateAxes(float length = 2.0f);

    // Module 2: Stippled 3D Wireframe Box (for collision/interaction highlight outlines)
    static void drawStippledWireBox(
        float cx, float cy, float cz,
        float sx, float sy, float sz,
        GLfloat r, GLfloat g, GLfloat b,
        GLint factor = 2,
        GLushort pattern = kStippleDashed,
        float lineWidth = 2.0f);

    // Module 2: Sunbeam / Dust Mote Particles using GL_POINTS & glPointSize
    static void drawDustParticles(float timeSeconds, int count = 28);

    // Module 2: Showcase all 10 OpenGL primitive types in one row/panel
    static void drawModule2PrimitiveShowcase(float originX, float originY, float originZ);
};

} // namespace renderer
} // namespace engine

#endif // ENGINE_RENDERER_PRIMITIVERENDERER_H
