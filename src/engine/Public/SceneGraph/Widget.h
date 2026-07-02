#pragma once

#include "SceneGraph/Node.h"
#include <functional>

class Widget : public Node {
    friend class UISystem;

public:
    Widget();
    virtual ~Widget();

    enum class SizePolicy { FIXED, FILL, SHRINK };

    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    void SetTouchEnabled(bool enabled);
    bool IsTouchEnabled() const;

    void SetFocusable(bool focusable);
    bool IsFocusable() const;

    void SetFocused(bool focused);
    bool IsFocused() const;

    void SetSizePolicy(SizePolicy policy);
    SizePolicy GetSizePolicy() const;

    using TouchCallback = std::function<void(Widget*, const Vec2&)>;

    void OnTouchStarted(TouchCallback cb);
    void OnTouchMoved(TouchCallback cb);
    void OnTouchEnded(TouchCallback cb);

    virtual bool HitTest(const Vec3& worldPoint);

    virtual void OnTouchStartedEvent(const Vec2& pos);
    virtual void OnTouchMovedEvent(const Vec2& pos);
    virtual void OnTouchEndedEvent(const Vec2& pos);

protected:
    bool m_enabled = true;
    bool m_touchEnabled = true;
    bool m_focusable = false;
    bool m_focused = false;

    SizePolicy m_sizePolicy = SizePolicy::FIXED;

    TouchCallback m_onTouchStarted;
    TouchCallback m_onTouchMoved;
    TouchCallback m_onTouchEnded;
};
