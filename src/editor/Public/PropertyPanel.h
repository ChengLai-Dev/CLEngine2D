#pragma once

#include <memory>
#include <functional>

class Node;
class Widget;
class Renderer;
class OrthographicCamera;

class PropertyPanel {
public:
    PropertyPanel();
    ~PropertyPanel();

    void SetTarget(Node* target);
    Node* GetTarget() const;

    void SetRect(float x, float y, float w, float h);

    void OnRender(Renderer& renderer);

    using PropertyChangedCallback = std::function<void()>;
    void OnPropertyChanged(PropertyChangedCallback cb);

private:
    void DrawProperty(const char* label, float value, float minVal, float maxVal,
                      Renderer& renderer, float& y, std::function<void(float)> setter);

    Node* m_target = nullptr;

    float m_rectX = 0.0f;
    float m_rectY = 0.0f;
    float m_rectW = 300.0f;
    float m_rectH = 720.0f;

    PropertyChangedCallback m_onPropertyChanged;

    std::unique_ptr<OrthographicCamera> m_camera;
};
