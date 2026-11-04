#ifndef ENGINE_CORE_MATH4X4_H
#define ENGINE_CORE_MATH4X4_H

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>

namespace engine {
namespace core {

/**
 * Module 6: Homogeneous 4D Vector (x, y, z, w)
 * - w = 1.0f indicates a 3D position in space (affected by translation)
 * - w = 0.0f indicates a 3D direction vector (unaffected by translation)
 */
struct Vec4 {
    float x;
    float y;
    float z;
    float w;

    Vec4() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    Vec4(float px, float py, float pz, float pw = 1.0f)
        : x(px), y(py), z(pz), w(pw) {}

    static Vec4 point(float px, float py, float pz) {
        return Vec4(px, py, pz, 1.0f);
    }

    static Vec4 direction(float dx, float dy, float dz) {
        return Vec4(dx, dy, dz, 0.0f);
    }
};

/**
 * Module 6: Explicit 4x4 Matrix (Row-major for pedagogical clarity in CPU-side
 * homogeneous transformations, with conversion helper to OpenGL column-major).
 */
struct Mat4 {
    float m[4][4];

    Mat4(); // Initializes to Identity matrix

    static Mat4 identity();
    static Mat4 zero();

    // Module 6 Subtopic 1: Matrix Operations
    static Mat4 add(const Mat4& a, const Mat4& b);
    static Mat4 subtract(const Mat4& a, const Mat4& b);
    static Mat4 multiplyScalar(const Mat4& a, float scalar);
    static Vec4 multiplyVec4(const Mat4& mat, const Vec4& vec);
    static Mat4 multiply(const Mat4& a, const Mat4& b);

    // Module 6 & 7: Transformation Matrices
    static Mat4 translation(float tx, float ty, float tz);
    static Mat4 scaling(float sx, float sy, float sz);
    static Mat4 rotationX(float degrees);
    static Mat4 rotationY(float degrees);
    static Mat4 rotationZ(float degrees);

    // Convert to OpenGL column-major float[16] array for glMultMatrixf / glLoadMatrixf
    void toColumnMajor(float out16[16]) const;
};

/**
 * Module 6 Subtopic 1 & 2: CPU-side VBO Vertex Transformation via glMapBuffer.
 * Replicates the exact Module 6 lecture algorithm:
 *   translate(float x, float y, float z, const int startindex)
 * Maps GL_ARRAY_BUFFER with GL_READ_WRITE, multiplies the 4x4 translation matrix
 * against the 4x1 homogeneous vertex [vx, vy, vz, 1.0f]^T, writes back the result,
 * and unmaps the buffer with glUnmapBuffer(GL_ARRAY_BUFFER).
 */
class Math4x4 {
public:
    static bool translateVertexInMappedVBO(
        GLuint vboId,
        float tx,
        float ty,
        float tz,
        int startFloatIndex);

    static bool transformMeshVerticesInMappedVBO(
        GLuint vboId,
        const Mat4& transform,
        int vertexCount,
        int strideFloats,
        int posOffsetFloats = 0);
};

} // namespace core
} // namespace engine

#endif // ENGINE_CORE_MATH4X4_H
