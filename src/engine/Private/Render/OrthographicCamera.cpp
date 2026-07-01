#include "Render/OrthographicCamera.h"

OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top) {
    SetProjection(left, right, bottom, top);
    RecalculateViewMatrix();
}

void OrthographicCamera::SetProjection(float left, float right, float bottom, float top) {
    float n = -1.0f;
    float f = 1.0f;
    m_projectionMatrix = Mat4::Ortho(left, right, bottom, top, n, f);
}

void OrthographicCamera::SetPosition(const Vec3& position) {
    m_position = position;
    RecalculateViewMatrix();
}

void OrthographicCamera::SetRotation(float rotationRad) {
    m_rotation = rotationRad;
    RecalculateViewMatrix();
}

void OrthographicCamera::RecalculateViewMatrix() {
    Mat4 t = Mat4::Translate(Vec3(-m_position.x, -m_position.y, -m_position.z));
    Mat4 r = Mat4::RotateZ(-m_rotation);
    m_viewMatrix = r * t;
}
