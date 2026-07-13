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
            { "Tex Offset X", FieldType::Float, GetTexOffsetX, SetTexOffsetX },
            { "Tex Offset Y", FieldType::Float, GetTexOffsetY, SetTexOffsetY },
            { "Tex Scale X", FieldType::Float, GetTexScaleX, SetTexScaleX },
            { "Tex Scale Y", FieldType::Float, GetTexScaleY, SetTexScaleY },
        }
    },
};

const std::vector<PropertySectionDef>& PropertyFieldRegistry::GetAll() {
    return s_sections;
}
