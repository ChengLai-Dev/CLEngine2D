#pragma once

#include "Math/Mat4.h"

namespace Math {

inline Mat4 Translate(const Vec3& t) { return Mat4::Translate(t); }
inline Mat4 RotateX(float angleRad) { return Mat4::RotateX(angleRad); }
inline Mat4 RotateY(float angleRad) { return Mat4::RotateY(angleRad); }
inline Mat4 RotateZ(float angleRad) { return Mat4::RotateZ(angleRad); }
inline Mat4 Scale(const Vec3& s) { return Mat4::Scale(s); }
inline Mat4 Ortho(float l, float r, float b, float t, float n, float f) { return Mat4::Ortho(l, r, b, t, n, f); }
inline Mat4 Perspective(float fovYRad, float aspect, float nearZ, float farZ) { return Mat4::Perspective(fovYRad, aspect, nearZ, farZ); }
inline Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up) { return Mat4::LookAt(eye, target, up); }

}
