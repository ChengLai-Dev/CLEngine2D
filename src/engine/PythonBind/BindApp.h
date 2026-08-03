#pragma once

#include <Application.h>
#include <pybind11/embed.h>
#include <memory>
#include <string>

namespace py = pybind11;

class Renderer;
class OrthographicCamera;
class TextRenderer;

class PythonScriptApp : public Application {
public:
    PythonScriptApp(const std::string& scriptDir,
                    const std::string& moduleName);
    ~PythonScriptApp() override;

    Renderer* GetRenderer() const { return m_renderer.get(); }
    OrthographicCamera* GetGameCamera() const { return m_gameCamera.get(); }
    OrthographicCamera* GetUICamera() const { return m_uiCamera.get(); }

    float GetDeltaTime() const { return m_deltaTime; }
    float GetFPS() const { return m_fps; }

    static PythonScriptApp* GetCurrent() { return s_current; }

    void ReloadScripts();

protected:
    void OnInit() override;
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void OnShutdown() override;
    void OnWindowResize(int width, int height) override;

private:
    void loadScriptFunctions();
    bool tryCall(const py::object& func);

    py::scoped_interpreter m_guard;
    py::object m_pyModule;
    std::string m_scriptDir;
    std::string m_moduleName;
    py::object m_pyOnInit = py::none();
    py::object m_pyOnUpdate = py::none();
    py::object m_pyOnRender = py::none();
    py::object m_pyOnShutdown = py::none();

    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<TextRenderer> m_fontRenderer;
    std::unique_ptr<OrthographicCamera> m_gameCamera;
    std::unique_ptr<OrthographicCamera> m_uiCamera;
    float m_deltaTime = 0.0f;
    float m_fps = 0.0f;

    static PythonScriptApp* s_current;
};
