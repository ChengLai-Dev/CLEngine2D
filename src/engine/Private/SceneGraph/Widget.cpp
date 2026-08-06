#include "SceneGraph/Widget.h"
#include "SceneGraph/UISystem.h"
#include "Math/Mat4.h"
#include "Logger.h"
#include <typeinfo>

Widget::Widget() = default;

Widget::~Widget() {
    // 回调栈内销毁宿主（点击触发场景切换 → 本层被 RemoveUI）后，UISystem 的
    // pressed/hovered/focused 状态指针可能仍指向本对象：析构时反向通知清理，
    // 防止下一帧/回调返回后的悬垂访问（Access violation - no RTTI data）。
    UISystem::GetInstance().OnWidgetDestroyed(this);
}

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

bool Widget::HitTestGeometry(const Vec3& worldPoint) {
    const Mat4& world = GetWorldTransform();
    Mat4 inv = Mat4::Inverse(world);
    Vec3 local = inv.TransformPoint(worldPoint);

    float halfW = m_contentSize.x * 0.5f;
    float halfH = m_contentSize.y * 0.5f;

    return local.x >= -halfW && local.x <= halfW &&
           local.y >= -halfH && local.y <= halfH;
}

bool Widget::HitTest(const Vec3& worldPoint) {
    if (!m_visible || !m_enabled) return false;
    return HitTestGeometry(worldPoint);
}

void Widget::OnTouchStartedEvent(const Vec2& pos) {
    Logger::Debug("[{}] TouchStarted pos=({}, {})", typeid(*this).name(), pos.x, pos.y);
    // 回调前置拷贝：Python 回调栈内可能销毁本对象（场景切换），
    // 回调返回后不得再访问任何成员
    TouchCallback cb = m_onTouchStarted;
    if (cb) {
        cb(this, pos);
    }
}

void Widget::OnTouchMovedEvent(const Vec2& pos) {
    // Logger::Debug("[{}] TouchMoved pos=({}, {})", typeid(*this).name(), pos.x, pos.y);
    TouchCallback cb = m_onTouchMoved;
    if (cb) {
        cb(this, pos);
    }
}

void Widget::OnTouchEndedEvent(const Vec2& pos) {
    Logger::Debug("[{}] TouchEnded pos=({}, {})", typeid(*this).name(), pos.x, pos.y);
    TouchCallback cb = m_onTouchEnded;
    if (cb) {
        cb(this, pos);
    }
}

void Widget::OnKeyDown(KeyCallback cb) {
    m_onKeyDown = std::move(cb);
}

void Widget::OnKeyUp(KeyCallback cb) {
    m_onKeyUp = std::move(cb);
}

bool Widget::OnKeyDownEvent(KeyCode key) {
    KeyCallback cb = m_onKeyDown;
    if (cb) {
        return cb(this, key);
    }
    return false;
}

bool Widget::OnKeyUpEvent(KeyCode key) {
    KeyCallback cb = m_onKeyUp;
    if (cb) {
        return cb(this, key);
    }
    return false;
}
