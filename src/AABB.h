#pragma once
#include <glm/glm.hpp>

struct AABB
{
    glm::vec3 a, b;

    [[nodiscard]] AABB expanded(const glm::vec3 offset) const
    {
        auto new_aabb = *this;

        if (offset.x < 0) new_aabb.a.x -= offset.x;
        else new_aabb.b.x += offset.x;
        if (offset.y < 0) new_aabb.a.y -= offset.y;
        else new_aabb.b.y += offset.y;
        if (offset.z < 0) new_aabb.a.z -= offset.z;
        else new_aabb.b.z += offset.z;

        return new_aabb;
    }

    [[nodiscard]] AABB grown(const glm::vec3 offset) const
    {
        return AABB { a - offset, b + offset };
    }

    [[nodiscard]] AABB moved(const glm::vec3 offset) const
    {
        return AABB { a + offset, b + offset };
    }

    [[nodiscard]] bool intersects(const AABB& box) const
    {
        if (!(box.b.x <= a.x) && !(box.a.x >= b.x)) {
            if (!(box.b.y <= a.y) && !(box.a.y >= b.y)) {
                return !(box.b.z <= a.z) && !(box.a.z >= b.z);
            }
        }
        return false;
    }

    template<int primaryAxis, int secondaryAxisA, int secondaryAxisB>
    [[nodiscard]] float clipAxisCollide(const AABB& box, float change) const
    {
        if (!overlapsAxis<secondaryAxisA>(box) ||
            !overlapsAxis<secondaryAxisB>(box)) return change;

        float max;
        if (change > 0.0f && box.b[primaryAxis] <= a[primaryAxis]) {
            max = a[primaryAxis] - box.b[primaryAxis] - FLT_EPSILON;
            if (max < change) {
                change = max;
            }
        }

        if (change < 0.0F && box.a[primaryAxis] >= b[primaryAxis]) {
            max = b[primaryAxis] - box.a[primaryAxis] + FLT_EPSILON;
            if (max > change) {
                change = max;
            }
        }

        return change;
    }

    template<int axis>
    [[nodiscard]] bool overlapsAxis(const AABB& other) const
    {
        return other.b[axis] > a[axis] && other.a[axis] < b[axis];
    }
};
