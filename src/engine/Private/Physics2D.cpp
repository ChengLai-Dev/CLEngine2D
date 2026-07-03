#include "Physics2D.h"
#include <algorithm>
#include <cmath>

CollisionInfo CheckAABBvsAABB(const AABB& a, const AABB& b) {
    CollisionInfo info;

    if (a.max.x < b.min.x || a.min.x > b.max.x) return info;
    if (a.max.y < b.min.y || a.min.y > b.max.y) return info;

    info.colliding = true;

    float overlapX = std::min(a.max.x, b.max.x) - std::max(a.min.x, b.min.x);
    float overlapY = std::min(a.max.y, b.max.y) - std::max(a.min.y, b.min.y);

    Vec2 aCenter = a.GetCenter();
    Vec2 bCenter = b.GetCenter();
    Vec2 diff = bCenter - aCenter;

    if (overlapX < overlapY) {
        info.normal = (diff.x > 0.0f) ? Vec2(1.0f, 0.0f) : Vec2(-1.0f, 0.0f);
        info.penetration = overlapX;
    } else {
        info.normal = (diff.y > 0.0f) ? Vec2(0.0f, 1.0f) : Vec2(0.0f, -1.0f);
        info.penetration = overlapY;
    }

    info.contactPoint = (aCenter + bCenter) * 0.5f;
    return info;
}

CollisionInfo CheckCirclevsCircle(const Circle& a, const Circle& b) {
    CollisionInfo info;

    Vec2 diff = b.center - a.center;
    float distSq = diff.LengthSq();
    float radiusSum = a.radius + b.radius;

    if (distSq > radiusSum * radiusSum) return info;

    info.colliding = true;
    float dist = std::sqrt(distSq);

    if (dist < 0.0001f) {
        info.normal = Vec2(1.0f, 0.0f);
        info.penetration = radiusSum;
        info.contactPoint = a.center;
        return info;
    }

    info.normal = diff / dist;
    info.penetration = radiusSum - dist;
    info.contactPoint = a.center + info.normal * (a.radius - info.penetration * 0.5f);
    return info;
}

CollisionInfo CheckAABBvsCircle(const AABB& aabb, const Circle& circle) {
    CollisionInfo info;

    Vec2 closest(
        std::max(aabb.min.x, std::min(circle.center.x, aabb.max.x)),
        std::max(aabb.min.y, std::min(circle.center.y, aabb.max.y))
    );

    Vec2 diff = circle.center - closest;
    float distSq = diff.LengthSq();

    if (distSq > circle.radius * circle.radius) return info;

    info.colliding = true;
    float dist = std::sqrt(distSq);

    if (dist < 0.0001f) {
        Vec2 centerDiff = circle.center - aabb.GetCenter();
        if (centerDiff.LengthSq() < 0.0001f) {
            info.normal = Vec2(0.0f, -1.0f);
        } else {
            info.normal = centerDiff.Normalized();
        }
        info.penetration = circle.radius;
        info.contactPoint = circle.center - info.normal * circle.radius;
        return info;
    }

    info.normal = diff / dist;
    info.penetration = circle.radius - dist;
    info.contactPoint = closest;
    return info;
}

CollisionInfo CheckCirclevsAABB(const Circle& circle, const AABB& aabb) {
    CollisionInfo info = CheckAABBvsCircle(aabb, circle);
    if (info.colliding) {
        info.normal = -info.normal;
    }
    return info;
}

void ResolveCollision(Vec2& posA, Vec2& posB, const CollisionInfo& info,
                      float massA, float massB)
{
    if (!info.colliding) return;

    float totalMass = massA + massB;
    if (totalMass < 0.0001f) return;

    float ratioA = massB / totalMass;
    float ratioB = massA / totalMass;

    posA -= info.normal * (info.penetration * ratioA);
    posB += info.normal * (info.penetration * ratioB);
}

bool PointInAABB(const Vec2& point, const AABB& aabb) {
    return point.x >= aabb.min.x && point.x <= aabb.max.x &&
           point.y >= aabb.min.y && point.y <= aabb.max.y;
}

bool PointInCircle(const Vec2& point, const Circle& circle) {
    Vec2 diff = point - circle.center;
    return diff.LengthSq() <= circle.radius * circle.radius;
}
