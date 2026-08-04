#pragma once

#include "PropertyFieldRegistry.h"
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <SceneGraph/Label.h>
#include <SceneGraph/Sprite.h>
#include <SceneGraph/Image.h>
#include <SceneGraph/Button.h>
#include <SceneGraph/Layout.h>
#include <SceneGraph/CanvasPanel.h>
#include <AssetManager.h>
#include <Render/Texture.h>
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
// Widget Type
// ======================================================================

inline std::string GetWidgetTypeName(Node* n) {
    if (dynamic_cast<Button*>(n)) return "Button";
    if (dynamic_cast<Label*>(n)) return "Label";
    if (dynamic_cast<Layout*>(n)) return "Layout";
    if (dynamic_cast<Image*>(n)) return "Image";
    if (dynamic_cast<Sprite*>(n)) return "Sprite";
    if (dynamic_cast<CanvasPanel*>(n)) return "Panel";
    if (dynamic_cast<Widget*>(n)) return "Widget";
    return "Node";
}

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

// ======================================================================
// Sprite — Texture
// ======================================================================

inline std::string GetSpriteTexture(Node* n) {
    auto* sp = static_cast<Sprite*>(n);
    auto tex = sp->GetTexture();
    return tex ? tex->GetFilePath() : "";
}

inline void SetSpriteTexture(Node* n, const std::string& v) {
    if (v.empty()) {
        static_cast<Sprite*>(n)->SetTexture(nullptr);
    } else {
        auto tex = AssetManager::GetInstance().LoadTexture(v);
        static_cast<Sprite*>(n)->SetTexture(tex);
    }
}

// ======================================================================
// Image
// ======================================================================

inline std::string GetScale9Enabled(Node* n) {
    return static_cast<Image*>(n)->IsScale9Enabled() ? "True" : "False";
}

inline void SetScale9Enabled(Node* n, const std::string& v) {
    static_cast<Image*>(n)->SetScale9Enabled(v == "True");
}

inline std::string GetCapInsetL(Node* n) { return FormatFloat(static_cast<Image*>(n)->GetCapInsets().x); }
inline void SetCapInsetL(Node* n, const std::string& s) {
    auto* img = static_cast<Image*>(n);
    Vec4 v = img->GetCapInsets(); v.x = ParseFloat(s, 0.0f, 10000.0f); img->SetCapInsets(v);
}

inline std::string GetCapInsetT(Node* n) { return FormatFloat(static_cast<Image*>(n)->GetCapInsets().y); }
inline void SetCapInsetT(Node* n, const std::string& s) {
    auto* img = static_cast<Image*>(n);
    Vec4 v = img->GetCapInsets(); v.y = ParseFloat(s, 0.0f, 10000.0f); img->SetCapInsets(v);
}

inline std::string GetCapInsetR(Node* n) { return FormatFloat(static_cast<Image*>(n)->GetCapInsets().z); }
inline void SetCapInsetR(Node* n, const std::string& s) {
    auto* img = static_cast<Image*>(n);
    Vec4 v = img->GetCapInsets(); v.z = ParseFloat(s, 0.0f, 10000.0f); img->SetCapInsets(v);
}

inline std::string GetCapInsetB(Node* n) { return FormatFloat(static_cast<Image*>(n)->GetCapInsets().w); }
inline void SetCapInsetB(Node* n, const std::string& s) {
    auto* img = static_cast<Image*>(n);
    Vec4 v = img->GetCapInsets(); v.w = ParseFloat(s, 0.0f, 10000.0f); img->SetCapInsets(v);
}

// ======================================================================
// Button — Textures
// ======================================================================

inline std::string GetBtnNormalTex(Node* n) {
    auto* btn = static_cast<Button*>(n);
    auto tex = btn->GetNormalImage();
    return tex ? tex->GetFilePath() : "";
}

inline void SetBtnNormalTex(Node* n, const std::string& v) {
    if (v.empty()) {
        static_cast<Button*>(n)->SetNormalImage(nullptr);
    } else {
        auto tex = AssetManager::GetInstance().LoadTexture(v);
        static_cast<Button*>(n)->SetNormalImage(tex);
    }
}

inline std::string GetBtnPressedTex(Node* n) {
    auto* btn = static_cast<Button*>(n);
    auto tex = btn->GetPressedImage();
    return tex ? tex->GetFilePath() : "";
}

inline void SetBtnPressedTex(Node* n, const std::string& v) {
    if (v.empty()) {
        static_cast<Button*>(n)->SetPressedImage(nullptr);
    } else {
        auto tex = AssetManager::GetInstance().LoadTexture(v);
        static_cast<Button*>(n)->SetPressedImage(tex);
    }
}

inline std::string GetBtnDisabledTex(Node* n) {
    auto* btn = static_cast<Button*>(n);
    auto tex = btn->GetDisabledImage();
    return tex ? tex->GetFilePath() : "";
}

inline void SetBtnDisabledTex(Node* n, const std::string& v) {
    if (v.empty()) {
        static_cast<Button*>(n)->SetDisabledImage(nullptr);
    } else {
        auto tex = AssetManager::GetInstance().LoadTexture(v);
        static_cast<Button*>(n)->SetDisabledImage(tex);
    }
}

// ======================================================================
// Button — Text / Font / Color
// ======================================================================

inline std::string GetBtnText(Node* n) { return static_cast<Button*>(n)->GetText(); }
inline void SetBtnText(Node* n, const std::string& v) { static_cast<Button*>(n)->SetText(v); }

inline std::string GetBtnFontSize(Node* n) { return FormatFloat(static_cast<Button*>(n)->GetFontSize()); }
inline void SetBtnFontSize(Node* n, const std::string& s) {
    static_cast<Button*>(n)->SetFontSize(ParseFloat(s, 1.0f, 200.0f));
}

inline std::string GetBtnTextColorR(Node* n) { return FormatFloat(static_cast<Button*>(n)->GetTextColor().x); }
inline void SetBtnTextColorR(Node* n, const std::string& s) {
    auto* btn = static_cast<Button*>(n);
    Vec4 c = btn->GetTextColor(); c.x = ParseFloat(s, 0.0f, 1.0f); btn->SetTextColor(c);
}

inline std::string GetBtnTextColorG(Node* n) { return FormatFloat(static_cast<Button*>(n)->GetTextColor().y); }
inline void SetBtnTextColorG(Node* n, const std::string& s) {
    auto* btn = static_cast<Button*>(n);
    Vec4 c = btn->GetTextColor(); c.y = ParseFloat(s, 0.0f, 1.0f); btn->SetTextColor(c);
}

inline std::string GetBtnTextColorB(Node* n) { return FormatFloat(static_cast<Button*>(n)->GetTextColor().z); }
inline void SetBtnTextColorB(Node* n, const std::string& s) {
    auto* btn = static_cast<Button*>(n);
    Vec4 c = btn->GetTextColor(); c.z = ParseFloat(s, 0.0f, 1.0f); btn->SetTextColor(c);
}

inline std::string GetBtnTextColorA(Node* n) { return FormatFloat(static_cast<Button*>(n)->GetTextColor().w); }
inline void SetBtnTextColorA(Node* n, const std::string& s) {
    auto* btn = static_cast<Button*>(n);
    Vec4 c = btn->GetTextColor(); c.w = ParseFloat(s, 0.0f, 1.0f); btn->SetTextColor(c);
}

// ======================================================================
// Layout
// ======================================================================

inline std::string GetLayoutType(Node* n) {
    switch (static_cast<Layout*>(n)->GetLayoutType()) {
        case Layout::Type::VERTICAL: return "VERTICAL";
        case Layout::Type::HORIZONTAL: return "HORIZONTAL";
        case Layout::Type::GRID: return "GRID";
    }
    return "VERTICAL";
}

inline void SetLayoutType(Node* n, const std::string& v) {
    auto* layout = static_cast<Layout*>(n);
    if (v == "HORIZONTAL") layout->SetLayoutType(Layout::Type::HORIZONTAL);
    else if (v == "GRID") layout->SetLayoutType(Layout::Type::GRID);
    else if (v == "VERTICAL") layout->SetLayoutType(Layout::Type::VERTICAL);
    layout->DoLayout();
}

inline std::string GetLayoutSpacing(Node* n) { return FormatFloat(static_cast<Layout*>(n)->GetSpacing()); }
inline void SetLayoutSpacing(Node* n, const std::string& s) {
    static_cast<Layout*>(n)->SetSpacing(ParseFloat(s, 0.0f, 1000.0f));
    static_cast<Layout*>(n)->DoLayout();
}

inline std::string GetLayoutGridColumns(Node* n) { return std::format("{}", static_cast<Layout*>(n)->GetGridColumns()); }
inline void SetLayoutGridColumns(Node* n, const std::string& s) {
    static_cast<Layout*>(n)->SetGridColumns(ParseInt(s, 1, 16));
    static_cast<Layout*>(n)->DoLayout();
}

inline std::string GetLayoutPaddingL(Node* n) { return FormatFloat(static_cast<Layout*>(n)->GetPadding().x); }
inline void SetLayoutPaddingL(Node* n, const std::string& s) {
    auto* layout = static_cast<Layout*>(n);
    Vec4 v = layout->GetPadding(); v.x = ParseFloat(s, 0.0f, 10000.0f); layout->SetPadding(v);
    layout->DoLayout();
}

inline std::string GetLayoutPaddingT(Node* n) { return FormatFloat(static_cast<Layout*>(n)->GetPadding().y); }
inline void SetLayoutPaddingT(Node* n, const std::string& s) {
    auto* layout = static_cast<Layout*>(n);
    Vec4 v = layout->GetPadding(); v.y = ParseFloat(s, 0.0f, 10000.0f); layout->SetPadding(v);
    layout->DoLayout();
}

inline std::string GetLayoutPaddingR(Node* n) { return FormatFloat(static_cast<Layout*>(n)->GetPadding().z); }
inline void SetLayoutPaddingR(Node* n, const std::string& s) {
    auto* layout = static_cast<Layout*>(n);
    Vec4 v = layout->GetPadding(); v.z = ParseFloat(s, 0.0f, 10000.0f); layout->SetPadding(v);
    layout->DoLayout();
}

inline std::string GetLayoutPaddingB(Node* n) { return FormatFloat(static_cast<Layout*>(n)->GetPadding().w); }
inline void SetLayoutPaddingB(Node* n, const std::string& s) {
    auto* layout = static_cast<Layout*>(n);
    Vec4 v = layout->GetPadding(); v.w = ParseFloat(s, 0.0f, 10000.0f); layout->SetPadding(v);
    layout->DoLayout();
}
