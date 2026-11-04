#include "engine/collision/CollisionSystem.h"
#include "engine/renderer/PrimitiveRenderer.h"

namespace engine {
namespace collision {

AABB3D AABB3D::fromCenterSize(
    float cx, float cy, float cz,
    float sx, float sy, float sz,
    const std::string& label,
    bool trigger)
{
    float hx = sx * 0.5f;
    float hy = sy * 0.5f;
    float hz = sz * 0.5f;
    return AABB3D{
        cx - hx, cy - hy, cz - hz,
        cx + hx, cy + hy, cz + hz,
        label,
        trigger
    };
}

bool AABB3D::intersects(const AABB3D& other) const {
    return (minX <= other.maxX && maxX >= other.minX) &&
           (minY <= other.maxY && maxY >= other.minY) &&
           (minZ <= other.maxZ && maxZ >= other.minZ);
}

bool AABB3D::intersectsXZ(const AABB3D& other) const {
    return (minX < other.maxX && maxX > other.minX) &&
           (minZ < other.maxZ && maxZ > other.minZ);
}

CollisionWorld::CollisionWorld() {}

void CollisionWorld::clear() {
    m_colliders.clear();
}

void CollisionWorld::addBoxCollider(
    float cx, float cy, float cz,
    float sx, float sy, float sz,
    const std::string& tag)
{
    m_colliders.push_back(AABB3D::fromCenterSize(cx, cy, cz, sx, sy, sz, tag, false));
}

bool CollisionWorld::resolveMovementXZ(
    float& posX,
    float& posZ,
    float deltaX,
    float deltaZ,
    float halfWidthX,
    float halfDepthZ,
    std::string* outHitTag) const
{
    bool hitAny = false;
    if (outHitTag) {
        outHitTag->clear();
    }

    // 1. Try moving along X axis first
    float candX = posX + deltaX;
    AABB3D boxX{
        candX - halfWidthX, 0.0f, posZ - halfDepthZ,
        candX + halfWidthX, 1.3f, posZ + halfDepthZ,
        "player", false
    };

    bool blockedX = false;
    for (const auto& col : m_colliders) {
        if (!col.isTrigger && boxX.intersectsXZ(col)) {
            blockedX = true;
            hitAny   = true;
            if (outHitTag && outHitTag->empty()) {
                *outHitTag = col.tag;
            }
            break;
        }
    }
    if (!blockedX) {
        posX = candX;
    }

    // 2. Try moving along Z axis next (allows smooth wall/desk sliding)
    float candZ = posZ + deltaZ;
    AABB3D boxZ{
        posX - halfWidthX, 0.0f, candZ - halfDepthZ,
        posX + halfWidthX, 1.3f, candZ + halfDepthZ,
        "player", false
    };

    bool blockedZ = false;
    for (const auto& col : m_colliders) {
        if (!col.isTrigger && boxZ.intersectsXZ(col)) {
            blockedZ = true;
            hitAny   = true;
            if (outHitTag && outHitTag->empty()) {
                *outHitTag = col.tag;
            }
            break;
        }
    }
    if (!blockedZ) {
        posZ = candZ;
    }

    return hitAny;
}

void CollisionWorld::drawDebugColliders(const std::string& highlightTag) const {
    for (const auto& col : m_colliders) {
        float cx = (col.minX + col.maxX) * 0.5f;
        float cy = (col.minY + col.maxY) * 0.5f;
        float cz = (col.minZ + col.maxZ) * 0.5f;
        float sx = (col.maxX - col.minX);
        float sy = (col.maxY - col.minY);
        float sz = (col.maxZ - col.minZ);

        bool isHighlighted = (!highlightTag.empty() && col.tag == highlightTag);
        if (isHighlighted) {
            renderer::PrimitiveRenderer::drawStippledWireBox(
                cx, cy, cz, sx, sy, sz,
                1.0f, 0.28f, 0.28f,
                1, renderer::PrimitiveRenderer::kStippleDashed, 2.5f);
        } else {
            renderer::PrimitiveRenderer::drawStippledWireBox(
                cx, cy, cz, sx, sy, sz,
                0.35f, 0.88f, 0.65f,
                2, renderer::PrimitiveRenderer::kStippleDashDot, 1.5f);
        }
    }
}

} // namespace collision
} // namespace engine
