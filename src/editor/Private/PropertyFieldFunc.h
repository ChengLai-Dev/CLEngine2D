#pragma once

#include "PropertyFieldRegistry.h"
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <SceneGraph/Label.h>
#include <SceneGraph/Sprite.h>
#include <algorithm>
#include <cstdlib>
#include <format>

// ======================================================================
// Helper Utilities
// ======================================================================

inline std::string FormatFloat(float v) {
    return std::format("{:.2f}", v);
}

inline float ParseFloat(const std::string& s, float min, float max) {
    char* end;
    float v = std::strtof(s.c_str(), &end);
    if (end != s.c_str() && *end == '\0') {
        v = (std::max)(v, min);
        v = (std::min)(v, max);
    }
    return v;
}

inline int ParseInt(const std::string& s, int min, int max) {
    char* end;
    long v = std::strtol(s.c_str(), &end, 10);
    if (end != s.c_str() && *end == '\0') {
        v = (std::max)(v, static_cast<long>(min));
        v = (std::min)(v, static_cast<long>(max));
    }
    return static_cast<int>(v);
}

// ======================================================================
// Name
// ======================================================================

inline std::string GetName(Node* n) { return n->GetName(); }
inline void SetName(Node* n, const std::string& v) { n->SetName(v); }

// ======================================================================
// Transform
// ======================================================================

inline std::string GetPosX(Node* n) { return FormatFloat(n->GetPosition().x); }
inline void SetPosX(Node* n, const std::string& s) {
    Vec3 p = n->GetPosition(); p.x = ParseFloat(s, -10000.0f, 10000.0f); n->SetPosition(p);
}

inline std::string GetPosY(Node* n) { return FormatFloat(n->GetPosition().y); }
inline void SetPosY(Node* n, const std::string& s) {
    Vec3 p = n->GetPosition(); p.y = ParseFloat(s, -10000.0f, 10000.0f); n->SetPosition(p);
}

inline std::string GetRotation(Node* n) { return FormatFloat(n->GetRotationZ()); }
inline void SetRotation(Node* n, const std::string& s) {
    n->SetRotationZ(ParseFloat(s, -10000.0f, 10000.0f));
}

inline std::string GetScaleX(Node* n) { return FormatFloat(n->GetScale().x); }
inline void SetScaleX(Node* n, const std::string& s) {
    Vec3 sc = n->GetScale(); sc.x = ParseFloat(s, -10000.0f, 10000.0f); n->SetScale(sc);
}

inline std::string GetScaleY(Node* n) { return FormatFloat(n->GetScale().y); }
inline void SetScaleY(Node* n, const std::string& s) {
    Vec3 sc = n->GetScale(); sc.y = ParseFloat(s, -10000.0f, 10000.0f); n->SetScale(sc);
}

inline std::string GetAnchorX(Node* n) { return FormatFloat(n->GetAnchor().x); }
inline void SetAnchorX(Node* n, const std::string& s) {
    Vec2 a = n->GetAnchor(); a.x = ParseFloat(s, 0.0f, 1.0f); n->SetAnchor(a);
}

inline std::string GetAnchorY(Node* n) { return FormatFloat(n->GetAnchor().y); }
inline void SetAnchorY(Node* n, const std::string& s) {
    Vec2 a = n->GetAnchor(); a.y = ParseFloat(s, 0.0f, 1.0f); n->SetAnchor(a);
}

// ======================================================================
// Size
// ======================================================================

inline std::string GetWidth(Node* n) { return FormatFloat(n->GetContentSize().x); }
inline void SetWidth(Node* n, const std::string& s) {
    Vec2 sz = n->GetContentSize(); sz.x = ParseFloat(s, 0.0f, 10000.0f); n->SetContentSize(sz);
}

inline std::string GetHeight(Node* n) { return FormatFloat(n->GetContentSize().y); }
inline void SetHeight(Node* n, const std::string& s) {
    Vec2 sz = n->GetContentSize(); sz.y = ParseFloat(s, 0.0f, 10000.0f); n->SetContentSize(sz);
}

// ======================================================================
// Appearance
// ======================================================================

inline std::string GetOpacity(Node* n) { return FormatFloat(n->GetOpacity()); }
inline void SetOpacity(Node* n, const std::string& s) {
    n->SetOpacity(ParseFloat(s, 0.0f, 1.0f));
}

inline std::string GetRed(Node* n) { return FormatFloat(n->GetColor().x); }
inline void SetRed(Node* n, const std::string& s) {
    Vec4 c = n->GetColor(); c.x = ParseFloat(s, 0.0f, 1.0f); n->SetColor(c);
}

inline std::string GetGreen(Node* n) { return FormatFloat(n->GetColor().y); }
inline void SetGreen(Node* n, const std::string& s) {
    Vec4 c = n->GetColor(); c.y = ParseFloat(s, 0.0f, 1.0f); n->SetColor(c);
}

inline std::string GetBlue(Node* n) { return FormatFloat(n->GetColor().z); }
inline void SetBlue(Node* n, const std::string& s) {
    Vec4 c = n->GetColor(); c.z = ParseFloat(s, 0.0f, 1.0f); n->SetColor(c);
}

inline std::string GetAlpha(Node* n) { return FormatFloat(n->GetColor().w); }
inline void SetAlpha(Node* n, const std::string& s) {
    Vec4 c = n->GetColor(); c.w = ParseFloat(s, 0.0f, 1.0f); n->SetColor(c);
}

inline std::string GetVisible(Node* n) { return n->IsVisible() ? "True" : "False"; }
inline void SetVisible(Node* n, const std::string& v) { n->SetVisible(v == "True"); }

inline std::string GetZOrder(Node* n) { return std::format("{}", n->GetZOrder()); }
inline void SetZOrder(Node* n, const std::string& s) {
    n->SetZOrder(ParseInt(s, -10000, 10000));
}

// ======================================================================
// Widget
// ======================================================================

inline std::string GetEnabled(Node* n) { return static_cast<Widget*>(n)->IsEnabled() ? "True" : "False"; }
inline void SetEnabled(Node* n, const std::string& v) { static_cast<Widget*>(n)->SetEnabled(v == "True"); }

inline std::string GetTouchEnabled(Node* n) { return static_cast<Widget*>(n)->IsTouchEnabled() ? "True" : "False"; }
inline void SetTouchEnabled(Node* n, const std::string& v) { static_cast<Widget*>(n)->SetTouchEnabled(v == "True"); }

inline std::string GetFocusable(Node* n) { return static_cast<Widget*>(n)->IsFocusable() ? "True" : "False"; }
inline void SetFocusable(Node* n, const std::string& v) { static_cast<Widget*>(n)->SetFocusable(v == "True"); }

// ======================================================================
// Label
// ======================================================================

inline std::string GetLabelText(Node* n) { return static_cast<Label*>(n)->GetText(); }
inline void SetLabelText(Node* n, const std::string& v) { static_cast<Label*>(n)->SetText(v); }

inline std::string GetFontSize(Node* n) { return FormatFloat(static_cast<Label*>(n)->GetFontSize()); }
inline void SetFontSize(Node* n, const std::string& s) {
    static_cast<Label*>(n)->SetFontSize(ParseFloat(s, 1.0f, 200.0f));
}

// ======================================================================
// Sprite
// ======================================================================

inline std::string GetTexOffsetX(Node* n) { return FormatFloat(static_cast<Sprite*>(n)->GetTexOffsetX()); }
inline void SetTexOffsetX(Node* n, const std::string& s) {
    auto* sp = static_cast<Sprite*>(n);
    float v = ParseFloat(s, -10000.0f, 10000.0f);
    sp->SetTexOffset(v, sp->GetTexOffsetY());
}

inline std::string GetTexOffsetY(Node* n) { return FormatFloat(static_cast<Sprite*>(n)->GetTexOffsetY()); }
inline void SetTexOffsetY(Node* n, const std::string& s) {
    auto* sp = static_cast<Sprite*>(n);
    float v = ParseFloat(s, -10000.0f, 10000.0f);
    sp->SetTexOffset(sp->GetTexOffsetX(), v);
}

inline std::string GetTexScaleX(Node* n) { return FormatFloat(static_cast<Sprite*>(n)->GetTexScaleX()); }
inline void SetTexScaleX(Node* n, const std::string& s) {
    auto* sp = static_cast<Sprite*>(n);
    float v = ParseFloat(s, -10000.0f, 10000.0f);
    sp->SetTexScale(v, sp->GetTexScaleY());
}

inline std::string GetTexScaleY(Node* n) { return FormatFloat(static_cast<Sprite*>(n)->GetTexScaleY()); }
inline void SetTexScaleY(Node* n, const std::string& s) {
    auto* sp = static_cast<Sprite*>(n);
    float v = ParseFloat(s, -10000.0f, 10000.0f);
    sp->SetTexScale(sp->GetTexScaleX(), v);
}
