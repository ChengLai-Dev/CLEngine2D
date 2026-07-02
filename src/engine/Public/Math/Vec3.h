#pragma once

#include <cmath>

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;

    Vec3(float s) : x(s), y(s), z(s) {}

    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& rhs) const
    {
        return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vec3 operator-(const Vec3& rhs) const
    {
        return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    Vec3 operator*(float s) const
    {
        return Vec3(x * s, y * s, z * s);
    }

    Vec3 operator/(float s) const
    {
        return Vec3(x / s, y / s, z / s);
    }

    Vec3 operator-() const
    {
        return Vec3(-x, -y, -z);
    }

    Vec3& operator+=(const Vec3& rhs)
    {
        x += rhs.x; y += rhs.y; z += rhs.z;
        return *this;
    }

    Vec3& operator-=(const Vec3& rhs)
    {
        x -= rhs.x; y -= rhs.y; z -= rhs.z;
        return *this;
    }

    Vec3& operator*=(float s)
    {
        x *= s; y *= s; z *= s;
        return *this;
    }

    Vec3& operator/=(float s)
    {
        x /= s; y /= s; z /= s;
        return *this;
    }

    float Dot(const Vec3& rhs) const
    {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    Vec3 Cross(const Vec3& rhs) const
    {
        return Vec3(
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x
        );
    }

    float Length() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    float LengthSq() const
    {
        return x * x + y * y + z * z;
    }

    Vec3 Normalized() const
    {
        float len = Length();
        if (len < 1e-6f) return Vec3(0.0f, 0.0f, 0.0f);
        return Vec3(x / len, y / len, z / len);
    }

    void Normalize()
    {
        float len = Length();
        if (len < 1e-6f) return;
        x /= len; y /= len; z /= len;
    }

    const float* Data() const
    {
        return &x;
    }
};
