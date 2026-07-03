#pragma once

#include "Math/Vec2.h"

struct AABB {
    Vec2 min;
    Vec2 max;

    AABB() = default;
    AABB(const Vec2& min, const Vec2& max) : min(min), max(max) {}

    Vec2 GetCenter() const { return (min + max) * 0.5f; }
    Vec2 GetSize() const { return Vec2(max.x - min.x, max.y - min.y); }
    Vec2 GetHalfSize() const { return GetSize() * 0.5f; }

    AABB Translated(const Vec2& offset) const {
        return AABB(min + offset, max + offset);
    }

    AABB Scaled(const Vec2& scale) const {
        Vec2 center = GetCenter();
        Vec2 half = GetHalfSize();
        half.x *= scale.x;
        half.y *= scale.y;
        return AABB(center - half, center + half);
    }
};

struct Circle {
    Vec2 center;
    float radius = 0.0f;

    Circle() = default;
    Circle(const Vec2& center, float radius) : center(center), radius(radius) {}
};

struct CollisionInfo {
    bool colliding = false;
    Vec2 normal;
    float penetration = 0.0f;
    Vec2 contactPoint;
};

CollisionInfo CheckAABBvsAABB(const AABB& a, const AABB& b);
CollisionInfo CheckCirclevsCircle(const Circle& a, const Circle& b);
CollisionInfo CheckAABBvsCircle(const AABB& aabb, const Circle& circle);
CollisionInfo CheckCirclevsAABB(const Circle& circle, const AABB& aabb);

void ResolveCollision(Vec2& posA, Vec2& posB, const CollisionInfo& info,
                      float massA = 1.0f, float massB = 1.0f);

bool PointInAABB(const Vec2& point, const AABB& aabb);
bool PointInCircle(const Vec2& point, const Circle& circle);
