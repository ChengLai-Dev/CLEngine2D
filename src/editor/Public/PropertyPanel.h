#pragma once

#include "IEditorPanel.h"
#include <memory>
#include <functional>

class Node;
class Widget;
class Renderer;

class PropertyPanel : public IEditorPanel {
public:
    PropertyPanel();
    ~PropertyPanel();

    void SetTarget(Node* target);
    Node* GetTarget() const;

    void OnRender(Renderer& renderer) override;

    using PropertyChangedCallback = std::function<void()>;
    void OnPropertyChanged(PropertyChangedCallback cb);

private:
    void DrawProperty(const char* label, float value, float minVal, float maxVal,
                      Renderer& renderer, float& y, std::function<void(float)> setter);

    Node* m_target = nullptr;

    PropertyChangedCallback m_onPropertyChanged;
};
