#include "engine/renderer/MeshVBO.h"

namespace engine {
namespace renderer {

MeshVBO::MeshVBO()
    : m_vboId(0)
    , m_eboId(0)
    , m_primitiveMode(GL_QUADS)
    , m_usage(GL_STATIC_DRAW)
{}

MeshVBO::~MeshVBO() {
    destroy();
}

void MeshVBO::destroy() {
    if (m_vboId != 0) {
        glDeleteBuffers(1, &m_vboId);
        m_vboId = 0;
    }
    if (m_eboId != 0) {
        glDeleteBuffers(1, &m_eboId);
        m_eboId = 0;
    }
}

void MeshVBO::uploadToGPU(
    const std::vector<Vertex3D>& vertices,
    const std::vector<GLushort>& indices,
    GLenum primitiveMode,
    GLenum usage)
{
    destroy();

    m_cpuVertices   = vertices;
    m_cpuIndices    = indices;
    m_primitiveMode = primitiveMode;
    m_usage         = usage;

    glGenBuffers(1, &m_vboId);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(m_cpuVertices.size() * sizeof(Vertex3D)),
        m_cpuVertices.data(),
        m_usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!m_cpuIndices.empty()) {
        glGenBuffers(1, &m_eboId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eboId);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(m_cpuIndices.size() * sizeof(GLushort)),
            m_cpuIndices.data(),
            m_usage);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void MeshVBO::updateSubData(int startVertex, const std::vector<Vertex3D>& updatedVertices) {
    if (m_vboId == 0 || updatedVertices.empty()) {
        return;
    }
    for (size_t i = 0; i < updatedVertices.size(); ++i) {
        size_t dst = static_cast<size_t>(startVertex) + i;
        if (dst < m_cpuVertices.size()) {
            m_cpuVertices[dst] = updatedVertices[i];
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        static_cast<GLintptr>(startVertex * sizeof(Vertex3D)),
        static_cast<GLsizeiptr>(updatedVertices.size() * sizeof(Vertex3D)),
        updatedVertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshVBO::buildTexturedBox(
    float width, float height, float depth,
    float uvScaleU, float uvScaleV,
    GLenum usage)
{
    float hx = width  * 0.5f;
    float hy = height * 0.5f;
    float hz = depth  * 0.5f;

    // Module 6 Subtopic 2: 24 vertices (6 faces x 4 vertices) with normals, colors, and UVs
    std::vector<Vertex3D> verts = {
        // Front Face (+Z)
        { -hx, -hy,  hz,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f,     0.0f     },
        {  hx, -hy,  hz,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f,  uvScaleU, 0.0f     },
        {  hx,  hy,  hz,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f,  uvScaleU, uvScaleV },
        { -hx,  hy,  hz,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f,     uvScaleV },

        // Back Face (-Z)
        {  hx, -hy, -hz,   0.0f,  0.0f, -1.0f,  0.95f, 0.95f, 0.95f, 0.0f,     0.0f     },
        { -hx, -hy, -hz,   0.0f,  0.0f, -1.0f,  0.95f, 0.95f, 0.95f, uvScaleU, 0.0f     },
        { -hx,  hy, -hz,   0.0f,  0.0f, -1.0f,  0.95f, 0.95f, 0.95f, uvScaleU, uvScaleV },
        {  hx,  hy, -hz,   0.0f,  0.0f, -1.0f,  0.95f, 0.95f, 0.95f, 0.0f,     uvScaleV },

        // Left Face (-X)
        { -hx, -hy, -hz,  -1.0f,  0.0f,  0.0f,  0.92f, 0.92f, 0.92f, 0.0f,     0.0f     },
        { -hx, -hy,  hz,  -1.0f,  0.0f,  0.0f,  0.92f, 0.92f, 0.92f, uvScaleU, 0.0f     },
        { -hx,  hy,  hz,  -1.0f,  0.0f,  0.0f,  0.92f, 0.92f, 0.92f, uvScaleU, uvScaleV },
        { -hx,  hy, -hz,  -1.0f,  0.0f,  0.0f,  0.92f, 0.92f, 0.92f, 0.0f,     uvScaleV },

        // Right Face (+X)
        {  hx, -hy,  hz,   1.0f,  0.0f,  0.0f,  0.92f, 0.92f, 0.92f, 0.0f,     0.0f     },
        {  hx, -hy, -hz,   1.0f,  0.0f,  0.0f,  0.92f, 0.92f, 0.92f, uvScaleU, 0.0f     },
        {  hx,  hy, -hz,   1.0f,  0.0f,  0.0f,  0.92f, 0.92f, 0.92f, uvScaleU, uvScaleV },
        {  hx,  hy,  hz,   1.0f,  0.0f,  0.0f,  0.92f, 0.92f, 0.92f, 0.0f,     uvScaleV },

        // Top Face (+Y)
        { -hx,  hy,  hz,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 1.0f,  0.0f,     0.0f     },
        {  hx,  hy,  hz,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 1.0f,  uvScaleU, 0.0f     },
        {  hx,  hy, -hz,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 1.0f,  uvScaleU, uvScaleV },
        { -hx,  hy, -hz,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 1.0f,  0.0f,     uvScaleV },

        // Bottom Face (-Y)
        { -hx, -hy, -hz,   0.0f, -1.0f,  0.0f,  0.85f, 0.85f, 0.85f, 0.0f,     0.0f     },
        {  hx, -hy, -hz,   0.0f, -1.0f,  0.0f,  0.85f, 0.85f, 0.85f, uvScaleU, 0.0f     },
        {  hx, -hy,  hz,   0.0f, -1.0f,  0.0f,  0.85f, 0.85f, 0.85f, uvScaleU, uvScaleV },
        { -hx, -hy,  hz,   0.0f, -1.0f,  0.0f,  0.85f, 0.85f, 0.85f, 0.0f,     uvScaleV }
    };

    // 24 indices for GL_QUADS
    std::vector<GLushort> indices(24);
    for (GLushort i = 0; i < 24; ++i) {
        indices[i] = i;
    }

    uploadToGPU(verts, indices, GL_QUADS, usage);
}

void MeshVBO::buildPyramid(float baseWidth, float height, GLenum usage) {
    float h = baseWidth * 0.5f;

    // Module 6 Subtopic 2: 3D Pyramid using GL_TRIANGLES (12 vertices for 4 triangular faces)
    std::vector<Vertex3D> verts = {
        // Front Triangle
        {  0.0f, height,  0.0f,   0.0f, 0.55f,  0.83f,   0.98f, 0.78f, 0.35f,  0.5f, 1.0f },
        { -h,    0.0f,    h,      0.0f, 0.55f,  0.83f,   0.92f, 0.45f, 0.25f,  0.0f, 0.0f },
        {  h,    0.0f,    h,      0.0f, 0.55f,  0.83f,   0.92f, 0.45f, 0.25f,  1.0f, 0.0f },

        // Right Triangle
        {  0.0f, height,  0.0f,   0.83f, 0.55f,  0.0f,   0.98f, 0.78f, 0.35f,  0.5f, 1.0f },
        {  h,    0.0f,    h,      0.83f, 0.55f,  0.0f,   0.85f, 0.38f, 0.22f,  0.0f, 0.0f },
        {  h,    0.0f,   -h,      0.83f, 0.55f,  0.0f,   0.85f, 0.38f, 0.22f,  1.0f, 0.0f },

        // Back Triangle
        {  0.0f, height,  0.0f,   0.0f, 0.55f, -0.83f,   0.98f, 0.78f, 0.35f,  0.5f, 1.0f },
        {  h,    0.0f,   -h,      0.0f, 0.55f, -0.83f,   0.92f, 0.45f, 0.25f,  0.0f, 0.0f },
        { -h,    0.0f,   -h,      0.0f, 0.55f, -0.83f,   0.92f, 0.45f, 0.25f,  1.0f, 0.0f },

        // Left Triangle
        {  0.0f, height,  0.0f,  -0.83f, 0.55f,  0.0f,   0.98f, 0.78f, 0.35f,  0.5f, 1.0f },
        { -h,    0.0f,   -h,     -0.83f, 0.55f,  0.0f,   0.85f, 0.38f, 0.22f,  0.0f, 0.0f },
        { -h,    0.0f,    h,     -0.83f, 0.55f,  0.0f,   0.85f, 0.38f, 0.22f,  1.0f, 0.0f }
    };

    std::vector<GLushort> indices(12);
    for (GLushort i = 0; i < 12; ++i) {
        indices[i] = i;
    }

    uploadToGPU(verts, indices, GL_TRIANGLES, usage);
}

void MeshVBO::buildQuadXZ(float width, float depth, float uvRepeatU, float uvRepeatV, GLenum usage) {
    float hx = width * 0.5f;
    float hz = depth * 0.5f;

    std::vector<Vertex3D> verts = {
        { -hx, 0.0f,  hz,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f,      0.0f      },
        {  hx, 0.0f,  hz,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  uvRepeatU, 0.0f      },
        {  hx, 0.0f, -hz,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  uvRepeatU, uvRepeatV },
        { -hx, 0.0f, -hz,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f,      uvRepeatV }
    };

    std::vector<GLushort> indices = { 0, 1, 2, 3 };
    uploadToGPU(verts, indices, GL_QUADS, usage);
}

static void enableInterleavedPointersVBO() {
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    constexpr GLsizei stride = static_cast<GLsizei>(sizeof(Vertex3D));
    glVertexPointer(3,   GL_FLOAT, stride, BUFFER_OFFSET(0));
    glNormalPointer(     GL_FLOAT, stride, BUFFER_OFFSET(3 * sizeof(GLfloat)));
    glColorPointer(3,    GL_FLOAT, stride, BUFFER_OFFSET(6 * sizeof(GLfloat)));
    glTexCoordPointer(2, GL_FLOAT, stride, BUFFER_OFFSET(9 * sizeof(GLfloat)));
}

static void disableInterleavedPointers() {
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void MeshVBO::drawVBOArrays() const {
    if (m_vboId == 0 || m_cpuVertices.empty()) {
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);
    enableInterleavedPointersVBO();

    glDrawArrays(m_primitiveMode, 0, static_cast<GLsizei>(m_cpuVertices.size()));

    disableInterleavedPointers();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshVBO::drawVBOElements() const {
    if (m_vboId == 0 || m_eboId == 0 || m_cpuIndices.empty()) {
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eboId);
    enableInterleavedPointersVBO();

    glDrawElements(
        m_primitiveMode,
        static_cast<GLsizei>(m_cpuIndices.size()),
        GL_UNSIGNED_SHORT,
        BUFFER_OFFSET(0));

    disableInterleavedPointers();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshVBO::drawVBOArrayElement() const {
    if (m_vboId == 0 || m_cpuVertices.empty()) {
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vboId);
    enableInterleavedPointersVBO();

    // Module 6 Subtopic 2: Drawing VBO vertices using glBegin + glArrayElement + glEnd
    glBegin(m_primitiveMode);
    for (size_t i = 0; i < m_cpuVertices.size(); ++i) {
        glArrayElement(static_cast<GLint>(i));
    }
    glEnd();

    disableInterleavedPointers();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshVBO::drawClientSideElements() const {
    if (m_cpuVertices.empty() || m_cpuIndices.empty()) {
        return;
    }
    // Module 4 Subtopic 1: Client-side vertex arrays (unbinding any VBO first)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    constexpr GLsizei stride = static_cast<GLsizei>(sizeof(Vertex3D));
    const char* base = reinterpret_cast<const char*>(m_cpuVertices.data());
    glVertexPointer(3,   GL_FLOAT, stride, base + 0);
    glNormalPointer(     GL_FLOAT, stride, base + 3 * sizeof(GLfloat));
    glColorPointer(3,    GL_FLOAT, stride, base + 6 * sizeof(GLfloat));
    glTexCoordPointer(2, GL_FLOAT, stride, base + 9 * sizeof(GLfloat));

    glDrawElements(
        m_primitiveMode,
        static_cast<GLsizei>(m_cpuIndices.size()),
        GL_UNSIGNED_SHORT,
        m_cpuIndices.data());

    disableInterleavedPointers();
}

} // namespace renderer
} // namespace engine
