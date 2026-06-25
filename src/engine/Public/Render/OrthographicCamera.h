#pragma once

#include "Render/Mat4.h"

class OrthographicCamera {
public:
    OrthographicCamera(float left, float right, float bottom, float top);

    void SetProjection(float left, float right, float bottom, float top);

    void SetPosition(const Vec3& position);
    void SetRotation(float rotationRad);

    const Vec3& GetPosition() const { return m_position; }
    float GetRotation() const { return m_rotation; }

    const Mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
    const Mat4& GetViewMatrix() const { return m_viewMatrix; }
    Mat4 GetViewProjectionMatrix() const { return m_projectionMatrix * m_viewMatrix; }

private:
    void RecalculateViewMatrix();

    Mat4 m_projectionMatrix;
    Mat4 m_viewMatrix;
    Vec3 m_position;
    float m_rotation = 0.0f;
};
