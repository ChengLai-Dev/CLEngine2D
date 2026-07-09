#pragma once

#include <Input/MouseEvent.h>
#include <typeinfo>
#include "Logger.h"

class Renderer;

class IEditorPanel {
public:
    virtual ~IEditorPanel() = default;

    virtual bool OnMouseEvent(const MouseEvent& event) {
        if (event.button == MouseEvent::None) return false;
        Logger::Debug("[{}] OnMouseEvent type={} pos=({},{}) btn={} scroll={}",
            typeid(*this).name(),
            event.GetTypeString(),
            event.screenPos.x, event.screenPos.y,
            event.GetButtonString(), event.scrollDelta);
        return false;
    }

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
