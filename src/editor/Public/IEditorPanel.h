#pragma once

#include <Math/Vec2.h>

class Renderer;

struct MouseEvent {
    enum Type { Down, Move, Up, Scroll };
    Type type;
    Vec2 screenPos;
    int button = 0;
    float scrollDelta = 0.0f;
};

class IEditorPanel {
public:
    virtual ~IEditorPanel() = default;

    virtual bool OnMouseEvent(const MouseEvent& event) { (void)event; return false; }

    struct HitRect {
        float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
        bool Contains(float px, float py) const {
            return px >= x && px < x + w && py >= y && py < y + h;
        }
    };
    virtual HitRect GetHitRect() const = 0;

    virtual bool IsCapturing() const { return false; }

    virtual void OnUpdate(float deltaTime) { (void)deltaTime; }
    virtual void OnRender(Renderer& renderer) = 0;

    virtual void SetRect(float x, float y, float w, float h) = 0;
    virtual void SetWindowHeight(int h) { (void)h; }
};
