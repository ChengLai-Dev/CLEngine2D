#pragma once

#include <memory>
#include <functional>

class Renderer;
class OrthographicCamera;

enum class ToolbarAction {
    ADD_BUTTON, ADD_LABEL, ADD_IMAGE, ADD_PANEL, ADD_LAYOUT,
    ACTION_SAVE, ACTION_LOAD, ACTION_UNDO, ACTION_REDO, ACTION_DELETE
};

class Toolbar {
public:
    Toolbar();
    ~Toolbar();

    void SetRect(float x, float y, float w, float h);

    void OnRender(Renderer& renderer);

    using ActionCallback = std::function<void(ToolbarAction)>;
    void OnAction(ActionCallback cb);

private:
    float m_rectX = 0.0f;
    float m_rectY = 0.0f;
    float m_rectW = 1280.0f;
    float m_rectH = 36.0f;

    ActionCallback m_onAction;
    std::unique_ptr<OrthographicCamera> m_camera;
};
