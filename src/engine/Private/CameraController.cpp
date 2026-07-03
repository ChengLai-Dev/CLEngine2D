#include "CameraController.h"
#include "Render/OrthographicCamera.h"

#include <algorithm>
#include <cmath>

CameraController::CameraController(OrthographicCamera& camera)
    : m_camera(camera)
    , m_desiredPosition(camera.GetPosition())
{
}

void CameraController::SetFollowTarget(const Vec3* target) {
    m_target = target;
}

const Vec3* CameraController::GetFollowTarget() const {
    return m_target;
}

void CameraController::SetSmoothFactor(float factor) {
    m_smoothFactor = std::max(0.0f, factor);
}

float CameraController::GetSmoothFactor() const {
    return m_smoothFactor;
}

void CameraController::SetDeadZone(const Vec2& deadZone) {
    m_deadZone = Vec2(std::max(0.0f, deadZone.x), std::max(0.0f, deadZone.y));
}

const Vec2& CameraController::GetDeadZone() const {
    return m_deadZone;
}

void CameraController::SetZoomTarget(float zoom) {
    m_targetZoom = std::max(0.01f, zoom);
}

float CameraController::GetZoomTarget() const {
    return m_targetZoom;
}

float CameraController::GetCurrentZoom() const {
    return m_currentZoom;
}

void CameraController::SetBounds(const Vec2& min, const Vec2& max) {
    m_hasBounds = true;
    m_boundsMin = min;
    m_boundsMax = max;
}

void CameraController::ClearBounds() {
    m_hasBounds = false;
}

bool CameraController::HasBounds() const {
    return m_hasBounds;
}

void CameraController::SetPosition(const Vec3& pos) {
    m_desiredPosition = pos;
    m_camera.SetPosition(pos);
}

const Vec3& CameraController::GetPosition() const {
    return m_camera.GetPosition();
}

void CameraController::Update(float deltaTime) {
    if (deltaTime > 0.05f) {
        deltaTime = 0.05f;
    }

    if (m_target) {
        Vec3 targetPos = *m_target;

        if (m_deadZone.x > 0.0f || m_deadZone.y > 0.0f) {
            Vec3 diff = targetPos - m_camera.GetPosition();
            float dx = std::abs(diff.x) - m_deadZone.x;
            float dy = std::abs(diff.y) - m_deadZone.y;

            if (dx > 0.0f || dy > 0.0f) {
                Vec3 offset;
                offset.x = (dx > 0.0f) ? (diff.x > 0.0f ? dx : -dx) : 0.0f;
                offset.y = (dy > 0.0f) ? (diff.y > 0.0f ? dy : -dy) : 0.0f;
                m_desiredPosition = m_camera.GetPosition() + offset;
            }
        } else {
            m_desiredPosition = targetPos;
        }
    }

    Vec3 currentPos = m_camera.GetPosition();
    if (m_smoothFactor > 0.0f) {
        float t = std::min(1.0f, m_smoothFactor * deltaTime);
        currentPos.x += (m_desiredPosition.x - currentPos.x) * t;
        currentPos.y += (m_desiredPosition.y - currentPos.y) * t;
        currentPos.z += (m_desiredPosition.z - currentPos.z) * t;
    } else {
        currentPos = m_desiredPosition;
    }

    if (m_currentZoom != m_targetZoom) {
        float t = std::min(1.0f, m_zoomSmoothFactor * deltaTime);
        m_currentZoom += (m_targetZoom - m_currentZoom) * t;
    }

    if (m_hasBounds) {
        float halfW = 16.0f / m_currentZoom;
        float halfH = 9.0f / m_currentZoom;
        currentPos.x = std::max(m_boundsMin.x + halfW, std::min(currentPos.x, m_boundsMax.x - halfW));
        currentPos.y = std::max(m_boundsMin.y + halfH, std::min(currentPos.y, m_boundsMax.y - halfH));
    }

    m_camera.SetPosition(currentPos);
}
