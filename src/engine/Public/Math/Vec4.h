#pragma once

struct Vec4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    Vec4() = default;

    Vec4(float s) : x(s), y(s), z(s), w(s) {}

    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    Vec4 operator+(const Vec4& rhs) const
    {
        return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
    }

    Vec4 operator-(const Vec4& rhs) const
    {
        return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
    }

    Vec4 operator*(float s) const
    {
        return Vec4(x * s, y * s, z * s, w * s);
    }

    Vec4 operator/(float s) const
    {
        return Vec4(x / s, y / s, z / s, w / s);
    }

    Vec4 operator-() const
    {
        return Vec4(-x, -y, -z, -w);
    }

    Vec4& operator+=(const Vec4& rhs)
    {
        x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w;
        return *this;
    }

    Vec4& operator-=(const Vec4& rhs)
    {
        x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w;
        return *this;
    }

    Vec4& operator*=(float s)
    {
        x *= s; y *= s; z *= s; w *= s;
        return *this;
    }

    Vec4& operator/=(float s)
    {
        x /= s; y /= s; z /= s; w /= s;
        return *this;
    }

    float Dot(const Vec4& rhs) const
    {
        return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
    }

    float Length() const
    {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    float LengthSq() const
    {
        return x * x + y * y + z * z + w * w;
    }

    const float* Data() const
    {
        return &x;
    }
};
