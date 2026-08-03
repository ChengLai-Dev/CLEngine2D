#include "PropertyFieldRegistry.h"
#include "PropertyFieldFunc.h"

static const std::vector<PropertySectionDef> s_sections = {
    {
        "Name", nullptr,
        {
            { "Name", FieldType::String, GetName, SetName },
        }
    },
    {
        "Transform", nullptr,
        {
            { "Pos X", FieldType::Float, GetPosX, SetPosX, -10000.0f, 10000.0f },
            { "Pos Y", FieldType::Float, GetPosY, SetPosY, -10000.0f, 10000.0f },
            { "Rotation", FieldType::Float, GetRotation, SetRotation, -10000.0f, 10000.0f },
            { "Scale X", FieldType::Float, GetScaleX, SetScaleX, -10000.0f, 10000.0f },
            { "Scale Y", FieldType::Float, GetScaleY, SetScaleY, -10000.0f, 10000.0f },
            { "Anchor X", FieldType::Float, GetAnchorX, SetAnchorX, 0.0f, 1.0f },
            { "Anchor Y", FieldType::Float, GetAnchorY, SetAnchorY, 0.0f, 1.0f },
        }
    },
    {
        "Size", nullptr,
        {
            { "Width", FieldType::Float, GetWidth, SetWidth, 0.0f, 10000.0f },
            { "Height", FieldType::Float, GetHeight, SetHeight, 0.0f, 10000.0f },
        }
    },
    {
        "Appearance", nullptr,
        {
            { "Opacity", FieldType::Float, GetOpacity, SetOpacity, 0.0f, 1.0f },
            { "Red", FieldType::Float, GetRed, SetRed, 0.0f, 1.0f },
            { "Green", FieldType::Float, GetGreen, SetGreen, 0.0f, 1.0f },
            { "Blue", FieldType::Float, GetBlue, SetBlue, 0.0f, 1.0f },
            { "Alpha", FieldType::Float, GetAlpha, SetAlpha, 0.0f, 1.0f },
            { "Visible", FieldType::Bool, GetVisible, SetVisible },
            { "Z Order", FieldType::Int, GetZOrder, SetZOrder, -10000.0f, 10000.0f },
        }
    },
    {
        "Widget",
        [](Node* n) { return dynamic_cast<Widget*>(n) != nullptr; },
        {
            { "Enabled", FieldType::Bool, GetEnabled, SetEnabled },
            { "Touch Enabled", FieldType::Bool, GetTouchEnabled, SetTouchEnabled },
            { "Focusable", FieldType::Bool, GetFocusable, SetFocusable },
        }
    },
    {
        "Label",
        [](Node* n) { return dynamic_cast<Label*>(n) != nullptr; },
        {
            { "Text", FieldType::String, GetLabelText, SetLabelText },
            { "Font Size", FieldType::Float, GetFontSize, SetFontSize, 1.0f, 200.0f },
        }
    },
    {
        "Sprite",
        [](Node* n) { return dynamic_cast<Sprite*>(n) != nullptr; },
        {
            { "Texture", FieldType::TextureAsset, GetSpriteTexture, SetSpriteTexture },
            { "Tex Offset X", FieldType::Float, GetTexOffsetX, SetTexOffsetX },
            { "Tex Offset Y", FieldType::Float, GetTexOffsetY, SetTexOffsetY },
            { "Tex Scale X", FieldType::Float, GetTexScaleX, SetTexScaleX },
            { "Tex Scale Y", FieldType::Float, GetTexScaleY, SetTexScaleY },
        }
    },
    {
        "Image",
        [](Node* n) { return dynamic_cast<Image*>(n) != nullptr; },
        {
            { "Scale9", FieldType::Bool, GetScale9Enabled, SetScale9Enabled },
            { "Cap Inset L", FieldType::Float, GetCapInsetL, SetCapInsetL, 0.0f, 10000.0f },
            { "Cap Inset T", FieldType::Float, GetCapInsetT, SetCapInsetT, 0.0f, 10000.0f },
            { "Cap Inset R", FieldType::Float, GetCapInsetR, SetCapInsetR, 0.0f, 10000.0f },
            { "Cap Inset B", FieldType::Float, GetCapInsetB, SetCapInsetB, 0.0f, 10000.0f },
        }
    },
    {
        "Button",
        [](Node* n) { return dynamic_cast<Button*>(n) != nullptr; },
        {
            { "Normal Tex", FieldType::TextureAsset, GetBtnNormalTex, SetBtnNormalTex },
            { "Pressed Tex", FieldType::TextureAsset, GetBtnPressedTex, SetBtnPressedTex },
            { "Disabled Tex", FieldType::TextureAsset, GetBtnDisabledTex, SetBtnDisabledTex },
            { "Text", FieldType::String, GetBtnText, SetBtnText },
            { "Font Size", FieldType::Float, GetBtnFontSize, SetBtnFontSize, 1.0f, 200.0f },
            { "Text Color R", FieldType::Float, GetBtnTextColorR, SetBtnTextColorR, 0.0f, 1.0f },
            { "Text Color G", FieldType::Float, GetBtnTextColorG, SetBtnTextColorG, 0.0f, 1.0f },
            { "Text Color B", FieldType::Float, GetBtnTextColorB, SetBtnTextColorB, 0.0f, 1.0f },
            { "Text Color A", FieldType::Float, GetBtnTextColorA, SetBtnTextColorA, 0.0f, 1.0f },
        }
    },
    {
        "Layout",
        [](Node* n) { return dynamic_cast<Layout*>(n) != nullptr; },
        {
            { "Type", FieldType::Enum, GetLayoutType, SetLayoutType, 0.0f, 0.0f, { "VERTICAL", "HORIZONTAL", "GRID" } },
            { "Spacing", FieldType::Float, GetLayoutSpacing, SetLayoutSpacing, 0.0f, 1000.0f },
            { "Grid Columns", FieldType::Int, GetLayoutGridColumns, SetLayoutGridColumns, 1.0f, 16.0f },
            { "Padding L", FieldType::Float, GetLayoutPaddingL, SetLayoutPaddingL, 0.0f, 10000.0f },
            { "Padding T", FieldType::Float, GetLayoutPaddingT, SetLayoutPaddingT, 0.0f, 10000.0f },
            { "Padding R", FieldType::Float, GetLayoutPaddingR, SetLayoutPaddingR, 0.0f, 10000.0f },
            { "Padding B", FieldType::Float, GetLayoutPaddingB, SetLayoutPaddingB, 0.0f, 10000.0f },
        }
    },
};

const std::vector<PropertySectionDef>& PropertyFieldRegistry::GetAll() {
    return s_sections;
}
