#include "engine/core/Math4x4.h"
#include <cmath>

namespace engine {
namespace core {

static constexpr float kDegToRad = 3.14159265358979323846f / 180.0f;

Mat4::Mat4() {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            m[r][c] = (r == c) ? 1.0f : 0.0f;
        }
    }
}

Mat4 Mat4::identity() {
    return Mat4();
}

Mat4 Mat4::zero() {
    Mat4 res;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            res.m[r][c] = 0.0f;
        }
    }
    return res;
}

Mat4 Mat4::add(const Mat4& a, const Mat4& b) {
    Mat4 res;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            res.m[r][c] = a.m[r][c] + b.m[r][c];
        }
    }
    return res;
}

Mat4 Mat4::subtract(const Mat4& a, const Mat4& b) {
    Mat4 res;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            res.m[r][c] = a.m[r][c] - b.m[r][c];
        }
    }
    return res;
}

Mat4 Mat4::multiplyScalar(const Mat4& a, float scalar) {
    Mat4 res;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            res.m[r][c] = a.m[r][c] * scalar;
        }
    }
    return res;
}

Vec4 Mat4::multiplyVec4(const Mat4& mat, const Vec4& vec) {
    float in[4]  = { vec.x, vec.y, vec.z, vec.w };
    float out[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            out[r] += mat.m[r][c] * in[c];
        }
    }
    return Vec4(out[0], out[1], out[2], out[3]);
}

Mat4 Mat4::multiply(const Mat4& a, const Mat4& b) {
    Mat4 res = Mat4::zero();
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            for (int k = 0; k < 4; ++k) {
                res.m[r][c] += a.m[r][k] * b.m[k][c];
            }
        }
    }
    return res;
}

Mat4 Mat4::translation(float tx, float ty, float tz) {
    Mat4 res = Mat4::identity();
    res.m[0][3] = tx;
    res.m[1][3] = ty;
    res.m[2][3] = tz;
    return res;
}

Mat4 Mat4::scaling(float sx, float sy, float sz) {
    Mat4 res = Mat4::identity();
    res.m[0][0] = sx;
    res.m[1][1] = sy;
    res.m[2][2] = sz;
    return res;
}

Mat4 Mat4::rotationX(float degrees) {
    float rad = degrees * kDegToRad;
    float cs  = std::cos(rad);
    float sn  = std::sin(rad);
    Mat4 res  = Mat4::identity();
    res.m[1][1] =  cs;
    res.m[1][2] = -sn;
    res.m[2][1] =  sn;
    res.m[2][2] =  cs;
    return res;
}

Mat4 Mat4::rotationY(float degrees) {
    float rad = degrees * kDegToRad;
    float cs  = std::cos(rad);
    float sn  = std::sin(rad);
    Mat4 res  = Mat4::identity();
    res.m[0][0] =  cs;
    res.m[0][2] =  sn;
    res.m[2][0] = -sn;
    res.m[2][2] =  cs;
    return res;
}

Mat4 Mat4::rotationZ(float degrees) {
    float rad = degrees * kDegToRad;
    float cs  = std::cos(rad);
    float sn  = std::sin(rad);
    Mat4 res  = Mat4::identity();
    res.m[0][0] =  cs;
    res.m[0][1] = -sn;
    res.m[1][0] =  sn;
    res.m[1][1] =  cs;
    return res;
}

void Mat4::toColumnMajor(float out16[16]) const {
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            out16[c * 4 + r] = m[r][c];
        }
    }
}

bool Math4x4::translateVertexInMappedVBO(
    GLuint vboId,
    float tx,
    float ty,
    float tz,
    int startFloatIndex)
{
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    GLfloat* mapped = static_cast<GLfloat*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    if (!mapped) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return false;
    }

    // Module 6 explicit 4x4 translation matrix multiplication on 4x1 homogeneous vertex
    Mat4 trans = Mat4::translation(tx, ty, tz);
    Vec4 vIn(mapped[startFloatIndex + 0],
             mapped[startFloatIndex + 1],
             mapped[startFloatIndex + 2],
             1.0f);
    Vec4 vOut = Mat4::multiplyVec4(trans, vIn);

    mapped[startFloatIndex + 0] = vOut.x;
    mapped[startFloatIndex + 1] = vOut.y;
    mapped[startFloatIndex + 2] = vOut.z;

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

bool Math4x4::transformMeshVerticesInMappedVBO(
    GLuint vboId,
    const Mat4& transform,
    int vertexCount,
    int strideFloats,
    int posOffsetFloats)
{
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    GLfloat* mapped = static_cast<GLfloat*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    if (!mapped) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return false;
    }

    for (int i = 0; i < vertexCount; ++i) {
        int base = i * strideFloats + posOffsetFloats;
        Vec4 vIn(mapped[base + 0], mapped[base + 1], mapped[base + 2], 1.0f);
        Vec4 vOut = Mat4::multiplyVec4(transform, vIn);
        mapped[base + 0] = vOut.x;
        mapped[base + 1] = vOut.y;
        mapped[base + 2] = vOut.z;
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

} // namespace core
} // namespace engine
