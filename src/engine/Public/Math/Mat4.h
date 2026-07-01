#pragma once

#include "Math/Vec3.h"
#include <cmath>

struct Mat4 {
    float m[4][4] = {};

    Mat4()
    {
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row)
                m[col][row] = 0.0f;
    }

    static Mat4 Identity()
    {
        Mat4 result;
        result.m[0][0] = 1.0f; result.m[1][1] = 1.0f;
        result.m[2][2] = 1.0f; result.m[3][3] = 1.0f;
        return result;
    }

    Mat4 operator*(const Mat4& rhs) const
    {
        Mat4 result;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += m[k][row] * rhs.m[col][k];
                }
                result.m[col][row] = sum;
            }
        }
        return result;
    }

    static Mat4 Ortho(float left, float right, float bottom, float top, float nearZ, float farZ)
    {
        Mat4 result = Identity();
        result.m[0][0] = 2.0f / (right - left);
        result.m[1][1] = 2.0f / (top - bottom);
        result.m[2][2] = -2.0f / (farZ - nearZ);
        result.m[3][0] = -(right + left) / (right - left);
        result.m[3][1] = -(top + bottom) / (top - bottom);
        result.m[3][2] = -(farZ + nearZ) / (farZ - nearZ);
        return result;
    }

    static Mat4 Translate(const Vec3& t)
    {
        Mat4 result = Identity();
        result.m[3][0] = t.x;
        result.m[3][1] = t.y;
        result.m[3][2] = t.z;
        return result;
    }

    static Mat4 Scale(const Vec3& s)
    {
        Mat4 result = Identity();
        result.m[0][0] = s.x;
        result.m[1][1] = s.y;
        result.m[2][2] = s.z;
        return result;
    }

    static Mat4 RotateZ(float angleRad)
    {
        Mat4 result = Identity();
        float c = cosf(angleRad);
        float s = sinf(angleRad);
        result.m[0][0] = c;   result.m[1][0] = -s;
        result.m[0][1] = s;   result.m[1][1] = c;
        return result;
    }

    static Mat4 RotateX(float angleRad)
    {
        Mat4 result = Identity();
        float c = cosf(angleRad);
        float s = sinf(angleRad);
        result.m[1][1] = c;   result.m[2][1] = -s;
        result.m[1][2] = s;   result.m[2][2] = c;
        return result;
    }

    static Mat4 RotateY(float angleRad)
    {
        Mat4 result = Identity();
        float c = cosf(angleRad);
        float s = sinf(angleRad);
        result.m[0][0] = c;   result.m[2][0] = s;
        result.m[0][2] = -s;  result.m[2][2] = c;
        return result;
    }

    Mat4 Transposed() const
    {
        Mat4 result;
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row)
                result.m[col][row] = m[row][col];
        return result;
    }

    static Mat4 Inverse(const Mat4& mat)
    {
        float cof[4][4];

        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sub[3][3];
                int si = 0;
                for (int c = 0; c < 4; ++c) {
                    if (c == col) continue;
                    int sj = 0;
                    for (int r = 0; r < 4; ++r) {
                        if (r == row) continue;
                        sub[si][sj] = mat.m[c][r];
                        ++sj;
                    }
                    ++si;
                }

                float det3 = sub[0][0] * (sub[1][1] * sub[2][2] - sub[1][2] * sub[2][1])
                           - sub[0][1] * (sub[1][0] * sub[2][2] - sub[1][2] * sub[2][0])
                           + sub[0][2] * (sub[1][0] * sub[2][1] - sub[1][1] * sub[2][0]);

                cof[col][row] = ((col + row) & 1) ? -det3 : det3;
            }
        }

        float det = 0.0f;
        for (int col = 0; col < 4; ++col) {
            det += mat.m[col][0] * cof[col][0];
        }

        if (std::abs(det) < 1e-6f) return Identity();

        Mat4 result;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                result.m[col][row] = cof[row][col] / det;
            }
        }
        return result;
    }

    static Mat4 Perspective(float fovYRad, float aspect, float nearZ, float farZ)
    {
        float tanHalfFov = std::tan(fovYRad / 2.0f);
        Mat4 result = {};
        result.m[0][0] = 1.0f / (aspect * tanHalfFov);
        result.m[1][1] = 1.0f / tanHalfFov;
        result.m[2][2] = -(farZ + nearZ) / (farZ - nearZ);
        result.m[2][3] = -1.0f;
        result.m[3][2] = -2.0f * farZ * nearZ / (farZ - nearZ);
        return result;
    }

    static Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
    {
        Vec3 f = (target - eye).Normalized();
        Vec3 s = f.Cross(up).Normalized();
        Vec3 u = s.Cross(f);

        Mat4 result = Identity();
        result.m[0][0] = s.x;  result.m[1][0] = s.y;  result.m[2][0] = s.z;
        result.m[0][1] = u.x;  result.m[1][1] = u.y;  result.m[2][1] = u.z;
        result.m[0][2] = -f.x; result.m[1][2] = -f.y; result.m[2][2] = -f.z;
        result.m[3][0] = -s.Dot(eye);
        result.m[3][1] = -u.Dot(eye);
        result.m[3][2] = f.Dot(eye);
        return result;
    }

    Vec3 TransformPoint(const Vec3& p) const
    {
        float x = m[0][0] * p.x + m[1][0] * p.y + m[2][0] * p.z + m[3][0];
        float y = m[0][1] * p.x + m[1][1] * p.y + m[2][1] * p.z + m[3][1];
        float z = m[0][2] * p.x + m[1][2] * p.y + m[2][2] * p.z + m[3][2];
        float w = m[0][3] * p.x + m[1][3] * p.y + m[2][3] * p.z + m[3][3];
        if (std::abs(w) > 1e-6f) {
            return Vec3(x / w, y / w, z / w);
        }
        return Vec3(x, y, z);
    }

    Vec3 TransformVector(const Vec3& v) const
    {
        float x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z;
        float y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z;
        float z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z;
        return Vec3(x, y, z);
    }

    const float* Data() const
    {
        return &m[0][0];
    }
};
