#pragma once

#include <cmath>

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;

    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& rhs) const
    {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec2 operator-(const Vec2& rhs) const
    {
        return Vec2(x - rhs.x, y - rhs.y);
    }

    Vec2 operator*(float s) const
    {
        return Vec2(x * s, y * s);
    }

    Vec2 operator/(float s) const
    {
        return Vec2(x / s, y / s);
    }

    Vec2 operator-() const
    {
        return Vec2(-x, -y);
    }

    Vec2& operator+=(const Vec2& rhs)
    {
        x += rhs.x; y += rhs.y;
        return *this;
    }

    Vec2& operator-=(const Vec2& rhs)
    {
        x -= rhs.x; y -= rhs.y;
        return *this;
    }

    Vec2& operator*=(float s)
    {
        x *= s; y *= s;
        return *this;
    }

    Vec2& operator/=(float s)
    {
        x /= s; y /= s;
        return *this;
    }

    float Dot(const Vec2& rhs) const
    {
        return x * rhs.x + y * rhs.y;
    }

    float Cross(const Vec2& rhs) const
    {
        return x * rhs.y - y * rhs.x;
    }

    float Length() const
    {
        return std::sqrt(x * x + y * y);
    }

    float LengthSq() const
    {
        return x * x + y * y;
    }

    Vec2 Normalized() const
    {
        float len = Length();
        if (len < 1e-6f) return Vec2(0.0f, 0.0f);
        return Vec2(x / len, y / len);
    }

    void Normalize()
    {
        float len = Length();
        if (len < 1e-6f) return;
        x /= len; y /= len;
    }

    const float* Data() const
    {
        return &x;
    }
};
