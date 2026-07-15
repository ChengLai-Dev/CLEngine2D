#pragma once

#include "IEditorPanel.h"
#include "HitRect.h"
#include "PropertyEditBox.h"
#include "PropertyFieldRegistry.h"
#include <memory>
#include <functional>
#include <string>
#include <vector>

class Node;
class Widget;
class Renderer;

class PropertyPanel : public IEditorPanel {
public:
    PropertyPanel();
    ~PropertyPanel();

    void SetParentHwnd(void* hwnd) { m_parentHwnd = hwnd; }

    void SetTarget(Node* target);
    Node* GetTarget() const;

    void OnRender(Renderer& renderer) override;
    bool OnMouseEvent(const MouseEvent& event) override;
    bool IsCapturing() const override { return m_activeFieldIndex >= 0; }
    void OnUpdate(float deltaTime) override;

    using PropertyChangedCallback = std::function<void()>;
    void OnPropertyChanged(PropertyChangedCallback cb);

    void OnNameChanged(std::function<void()> cb);

private:
    struct FieldInfo {
        FieldType type;
        std::string label;
        float virtualY;
        std::function<std::string()> getter;
        std::function<void(const std::string&)> setter;
        float minVal = 0.0f;
        float maxVal = 0.0f;
        bool isTextureAsset = false;
        PropertyEditBox editBox;
    };

    void BuildFields();
    void DrawFieldLabel(Renderer& renderer, const char* label, float y);
    void DrawFields(Renderer& renderer);
    void DrawSectionHeader(Renderer& renderer, const char* title, float y);
    void DrawFieldBackground(Renderer& renderer, float y, bool isActive, bool isBool);

    void StartEdit(int fieldIndex);
    void CommitEdit();
    void CancelEdit();

    Node* m_target = nullptr;

    std::vector<FieldInfo> m_fields;
    float m_contentHeight = 0.0f;

    int m_activeFieldIndex = -1;

    float m_scrollOffset = 0.0f;

    void* m_parentHwnd = nullptr;

    PropertyChangedCallback m_onPropertyChanged;
    std::function<void()> m_onNameChanged;
};
