#pragma once

#include "IEditorPanel.h"
#include <memory>
#include <functional>

class Node;
class Widget;
class Renderer;
class OrthographicCamera;

class PropertyPanel : public IEditorPanel {
public:
    PropertyPanel();
    ~PropertyPanel();

    void SetTarget(Node* target);
    Node* GetTarget() const;

    void SetRect(float x, float y, float width, float height) override;
    void SetWindowHeight(int height) override;

    void OnRender(Renderer& renderer) override;

    HitRect GetHitRect() const override;

    using PropertyChangedCallback = std::function<void()>;
    void OnPropertyChanged(PropertyChangedCallback cb);

private:
    void DrawProperty(const char* label, float value, float minVal, float maxVal,
                      Renderer& renderer, float& y, std::function<void(float)> setter);

    Node* m_target = nullptr;

    float m_rectX = 0.0f;
    float m_rectY = 0.0f;
    float m_rectWidth = 300.0f;
    float m_rectHeight = 720.0f;

    int m_windowHeight = 0;

    PropertyChangedCallback m_onPropertyChanged;

    std::unique_ptr<OrthographicCamera> m_camera;
};
