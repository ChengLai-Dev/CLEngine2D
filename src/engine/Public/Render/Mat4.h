#pragma once

#include "Render/Vec3.h"
#include <cmath>

struct Mat4 {
    float m[4][4];

    Mat4() {
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row)
                m[col][row] = 0.0f;
    }

    static Mat4 Identity() {
        Mat4 result;
        result.m[0][0] = 1.0f; result.m[1][1] = 1.0f;
        result.m[2][2] = 1.0f; result.m[3][3] = 1.0f;
        return result;
    }

    Mat4 operator*(const Mat4& rhs) const {
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

    static Mat4 Ortho(float left, float right, float bottom, float top, float nearZ, float farZ) {
        Mat4 result = Identity();
        result.m[0][0] = 2.0f / (right - left);
        result.m[1][1] = 2.0f / (top - bottom);
        result.m[2][2] = -2.0f / (farZ - nearZ);
        result.m[3][0] = -(right + left) / (right - left);
        result.m[3][1] = -(top + bottom) / (top - bottom);
        result.m[3][2] = -(farZ + nearZ) / (farZ - nearZ);
        return result;
    }

    static Mat4 Translate(const Vec3& t) {
        Mat4 result = Identity();
        result.m[3][0] = t.x;
        result.m[3][1] = t.y;
        result.m[3][2] = t.z;
        return result;
    }

    static Mat4 Scale(const Vec3& s) {
        Mat4 result = Identity();
        result.m[0][0] = s.x;
        result.m[1][1] = s.y;
        result.m[2][2] = s.z;
        return result;
    }

    static Mat4 RotateZ(float angleRad) {
        Mat4 result = Identity();
        float c = cosf(angleRad);
        float s = sinf(angleRad);
        result.m[0][0] = c;   result.m[1][0] = -s;
        result.m[0][1] = s;   result.m[1][1] = c;
        return result;
    }

    const float* Data() const {
        return &m[0][0];
    }
};
