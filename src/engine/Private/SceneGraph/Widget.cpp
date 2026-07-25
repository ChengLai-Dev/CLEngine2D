#include "SceneGraph/Widget.h"
#include "Math/Mat4.h"
#include "Logger.h"
#include <typeinfo>

Widget::Widget() = default;

Widget::~Widget() = default;

void Widget::SetEnabled(bool enabled) {
    m_enabled = enabled;
}

bool Widget::IsEnabled() const {
    return m_enabled;
}

void Widget::SetTouchEnabled(bool enabled) {
    m_touchEnabled = enabled;
}

bool Widget::IsTouchEnabled() const {
    return m_touchEnabled;
}

void Widget::SetFocusable(bool focusable) {
    m_focusable = focusable;
}

bool Widget::IsFocusable() const {
    return m_focusable;
}

void Widget::SetFocused(bool focused) {
    m_focused = focused;
}

bool Widget::IsFocused() const {
    return m_focused;
}

void Widget::SetSizePolicy(SizePolicy policy) {
    m_sizePolicy = policy;
}

Widget::SizePolicy Widget::GetSizePolicy() const {
    return m_sizePolicy;
}

void Widget::OnTouchStarted(TouchCallback cb) {
    m_onTouchStarted = std::move(cb);
}

void Widget::OnTouchMoved(TouchCallback cb) {
    m_onTouchMoved = std::move(cb);
}

void Widget::OnTouchEnded(TouchCallback cb) {
    m_onTouchEnded = std::move(cb);
}

bool Widget::HitTest(const Vec3& worldPoint) {
    if (!m_visible || !m_enabled) return false;

    const Mat4& world = GetWorldTransform();
    Mat4 inv = Mat4::Inverse(world);
    Vec3 local = inv.TransformPoint(worldPoint);

    float halfW = m_contentSize.x * 0.5f;
    float halfH = m_contentSize.y * 0.5f;

    return local.x >= -halfW && local.x <= halfW &&
           local.y >= -halfH && local.y <= halfH;
}

void Widget::OnTouchStartedEvent(const Vec2& pos) {
    Logger::Debug("[{}] TouchStarted pos=({}, {})", typeid(*this).name(), pos.x, pos.y);
    if (m_onTouchStarted) {
        m_onTouchStarted(this, pos);
    }
}

void Widget::OnTouchMovedEvent(const Vec2& pos) {
    // Logger::Debug("[{}] TouchMoved pos=({}, {})", typeid(*this).name(), pos.x, pos.y);
    if (m_onTouchMoved) {
        m_onTouchMoved(this, pos);
    }
}

void Widget::OnTouchEndedEvent(const Vec2& pos) {
    Logger::Debug("[{}] TouchEnded pos=({}, {})", typeid(*this).name(), pos.x, pos.y);
    if (m_onTouchEnded) {
        m_onTouchEnded(this, pos);
    }
}

void Widget::OnKeyDown(KeyCallback cb) {
    m_onKeyDown = std::move(cb);
}

void Widget::OnKeyUp(KeyCallback cb) {
    m_onKeyUp = std::move(cb);
}

bool Widget::OnKeyDownEvent(KeyCode key) {
    if (m_onKeyDown) {
        return m_onKeyDown(this, key);
    }
    return false;
}

bool Widget::OnKeyUpEvent(KeyCode key) {
    if (m_onKeyUp) {
        return m_onKeyUp(this, key);
    }
    return false;
}
