#pragma once

#include "Math/Vec2.h"
#include "Math/Vec3.h"

class OrthographicCamera;

class CameraController {
public:
    CameraController(OrthographicCamera& camera);

    void SetFollowTarget(const Vec3* target);
    const Vec3* GetFollowTarget() const;

    void SetSmoothFactor(float factor);
    float GetSmoothFactor() const;

    void SetDeadZone(const Vec2& deadZone);
    const Vec2& GetDeadZone() const;

    void SetZoomTarget(float zoom);
    float GetZoomTarget() const;
    float GetCurrentZoom() const;

    void SetBounds(const Vec2& min, const Vec2& max);
    void ClearBounds();
    bool HasBounds() const;

    void SetPosition(const Vec3& pos);
    const Vec3& GetPosition() const;

    void Update(float deltaTime);

private:
    OrthographicCamera& m_camera;
    const Vec3* m_target = nullptr;

    float m_smoothFactor = 5.0f;
    Vec2 m_deadZone = Vec2(0.0f, 0.0f);

    float m_currentZoom = 1.0f;
    float m_targetZoom = 1.0f;
    float m_zoomSmoothFactor = 3.0f;

    Vec3 m_desiredPosition;

    bool m_hasBounds = false;
    Vec2 m_boundsMin;
    Vec2 m_boundsMax;
};
