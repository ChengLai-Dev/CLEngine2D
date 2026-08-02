#pragma once

#include "HitRect.h"
#include <Input/MouseEvent.h>
#include <typeinfo>
#include "Logger.h"

class Renderer;
class TextRenderer;

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

    void SetRect(float x, float y, float w, float h) {
        m_rectLeft = x; m_rectTop = y;
        m_rectWidth = w; m_rectHeight = h;
    }
    void SetWindowHeight(int h) { m_windowHeight = h; }

    virtual HitRect GetHitRect() const {
        return { m_rectLeft, m_rectTop, m_rectWidth, m_rectHeight };
    }

    // 输入参与资格：不可见的面板不参与事件分发（与几何独立，Ref: UE SlateCore Layout/Visibility.h）
    virtual bool IsVisible() const { return true; }

    virtual bool IsCapturing() const { return false; }

    virtual void OnUpdate(float deltaTime) { (void)deltaTime; }
    virtual void OnRender(Renderer& renderer) = 0;

    void SetFontRenderer(TextRenderer* tr) { m_fontRenderer = tr; }

protected:
    float m_rectLeft = 0.0f;
    float m_rectTop = 0.0f;
    float m_rectWidth = 250.0f;
    float m_rectHeight = 50.0f;
    int m_windowHeight = 0;

    TextRenderer* m_fontRenderer = nullptr;
};
