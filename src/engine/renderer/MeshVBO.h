#ifndef ENGINE_RENDERER_MESHVBO_H
#define ENGINE_RENDERER_MESHVBO_H

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <vector>

// Module 6 Subtopic 2: Required byte offset macro for VBO pointers
#ifndef BUFFER_OFFSET
#define BUFFER_OFFSET(i) ((void*)(i))
#endif

namespace engine {
namespace renderer {

/**
 * Interleaved Vertex format (11 floats = 44 bytes):
 * - Position: (x, y, z)      [3 floats, offset 0]
 * - Normal:   (nx, ny, nz)   [3 floats, offset 3*sizeof(GLfloat)]
 * - Color:    (r, g, b)      [3 floats, offset 6*sizeof(GLfloat)]
 * - TexCoord: (u, v)         [2 floats, offset 9*sizeof(GLfloat)]
 */
struct Vertex3D {
    GLfloat x, y, z;
    GLfloat nx, ny, nz;
    GLfloat r, g, b;
    GLfloat u, v;
};

class MeshVBO {
public:
    MeshVBO();
    ~MeshVBO();

    // Build reusable 3D Box (24 vertices, 6 faces, GL_QUADS + 36 indices for GL_TRIANGLES)
    void buildTexturedBox(
        float width, float height, float depth,
        float uvScaleU = 1.0f, float uvScaleV = 1.0f,
        GLenum usage = GL_STATIC_DRAW);

    // Build Module 6 3D Pyramid (12 side vertices + base, GL_TRIANGLES)
    void buildPyramid(
        float baseWidth, float height,
        GLenum usage = GL_STATIC_DRAW);

    // Build flat 3D Quad (e.g., floor, wall panel, window, or character billboard)
    void buildQuadXZ(
        float width, float depth,
        float uvRepeatU = 1.0f, float uvRepeatV = 1.0f,
        GLenum usage = GL_STATIC_DRAW);

    void uploadToGPU(
        const std::vector<Vertex3D>& vertices,
        const std::vector<GLushort>& indices,
        GLenum primitiveMode,
        GLenum usage = GL_STATIC_DRAW);

    // Module 5 & 6: Update a range of vertices using glBufferSubData
    void updateSubData(int startVertex, const std::vector<Vertex3D>& updatedVertices);

    // Module 5 & 6: Draw using bound VBO + glDrawArrays
    void drawVBOArrays() const;

    // Module 4, 5 & 6: Draw using bound VBO + EBO + glDrawElements
    void drawVBOElements() const;

    // Module 6 Subtopic 2: Draw using bound VBO + glBegin/glArrayElement/glEnd
    void drawVBOArrayElement() const;

    // Module 4 Subtopic 1: Draw directly from CPU Client-Side Vertex Arrays (no VBO bound)
    void drawClientSideElements() const;

    void destroy();

    GLuint getVBOId() const { return m_vboId; }
    GLuint getEBOId() const { return m_eboId; }
    bool isValid() const { return m_vboId != 0 && glIsBuffer(m_vboId) == GL_TRUE; }
    int getVertexCount() const { return static_cast<int>(m_cpuVertices.size()); }

private:
    GLuint                m_vboId;
    GLuint                m_eboId;
    GLenum                m_primitiveMode;
    GLenum                m_usage;
    std::vector<Vertex3D> m_cpuVertices;
    std::vector<GLushort> m_cpuIndices;
};

} // namespace renderer
} // namespace engine

#endif // ENGINE_RENDERER_MESHVBO_H
