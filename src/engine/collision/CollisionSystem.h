#ifndef ENGINE_COLLISION_COLLISIONSYSTEM_H
#define ENGINE_COLLISION_COLLISIONSYSTEM_H

#include <string>
#include <vector>

namespace engine {
namespace collision {

struct AABB3D {
    float       minX, minY, minZ;
    float       maxX, maxY, maxZ;
    std::string tag;
    bool        isTrigger;

    static AABB3D fromCenterSize(
        float cx, float cy, float cz,
        float sx, float sy, float sz,
        const std::string& label = "",
        bool trigger = false);

    bool intersects(const AABB3D& other) const;
    bool intersectsXZ(const AABB3D& other) const;
};

class CollisionWorld {
public:
    CollisionWorld();

    void clear();
    void addBoxCollider(
        float cx, float cy, float cz,
        float sx, float sy, float sz,
        const std::string& tag = "obstacle");

    /**
     * Resolves 2.5D movement along X and Z axes independently so the player
     * slides smoothly along walls and furniture edges instead of snagging.
     * Returns true if any solid obstacle was contacted during this step.
     */
    bool resolveMovementXZ(
        float& posX,
        float& posZ,
        float deltaX,
        float deltaZ,
        float halfWidthX,
        float halfDepthZ,
        std::string* outHitTag = nullptr) const;

    void drawDebugColliders(const std::string& highlightTag = "") const;

    const std::vector<AABB3D>& getColliders() const { return m_colliders; }

private:
    std::vector<AABB3D> m_colliders;
};

} // namespace collision
} // namespace engine

#endif // ENGINE_COLLISION_COLLISIONSYSTEM_H
