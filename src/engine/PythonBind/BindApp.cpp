#include "BindApp.h"
#include <Render/Renderer.h>
#include <Render/OrthographicCamera.h>
#include <Render/RenderCommand.h>
#include <Scene.h>
#include <SceneGraph/UISystem.h>
#include <SceneGraph/Widget.h>
#include <Logger.h>
#include <Timer.h>
#include <Audio/AudioEngine.h>
#include <Input/InputSystem.h>
#include <Platform/Window.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

PythonScriptApp* PythonScriptApp::s_current = nullptr;

PythonScriptApp::PythonScriptApp(const std::string& scriptDir,
                                 const std::string& moduleName)
    : Application()
    , m_scriptDir(scriptDir)
    , m_moduleName(moduleName)
{
    s_current = this;
}

PythonScriptApp::~PythonScriptApp()
{
    s_current = nullptr;
}

void PythonScriptApp::ReloadScripts()
{
    Logger::Info("Hot-reloading Python scripts...");
    py::module_ hotreload = py::module_::import("engine_hotreload");
    hotreload.attr("perform_reload")(m_scriptDir, m_moduleName);
    loadScriptFunctions();
    Logger::Info("Script hot-reload completed");
}

void PythonScriptApp::loadScriptFunctions()
{
    py::module_ sys = py::module_::import("sys");
    sys.attr("path").attr("insert")(0, m_scriptDir);

    m_pyModule = py::module_::import(m_moduleName.c_str());

    m_pyOnInit = py::none();
    m_pyOnUpdate = py::none();
    m_pyOnRender = py::none();
    m_pyOnShutdown = py::none();

    if (py::hasattr(m_pyModule, "on_init"))
        m_pyOnInit = m_pyModule.attr("on_init");
    if (py::hasattr(m_pyModule, "on_update"))
        m_pyOnUpdate = m_pyModule.attr("on_update");
    if (py::hasattr(m_pyModule, "on_render"))
        m_pyOnRender = m_pyModule.attr("on_render");
    if (py::hasattr(m_pyModule, "on_shutdown"))
        m_pyOnShutdown = m_pyModule.attr("on_shutdown");
}

bool PythonScriptApp::tryCall(const py::object& func)
{
    if (func.is_none()) return false;
    try {
        func();
        return true;
    } catch (py::error_already_set& e) {
        Logger::Error("Python error: {}", e.what());
        return false;
    }
}

void PythonScriptApp::OnInit()
{
    Logger::Info("PythonScriptApp initializing...");

    RenderCommand::SetClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    RenderCommand::SetBlend(true);
    RenderCommand::SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_gameCamera = std::unique_ptr<OrthographicCamera>(
        new OrthographicCamera(-16.0f, 16.0f, -9.0f, 9.0f));
    m_uiCamera = std::unique_ptr<OrthographicCamera>(
        new OrthographicCamera(0.0f, 1280.0f, 720.0f, 0.0f));

    m_renderer = std::unique_ptr<Renderer>(new Renderer());
    m_renderer->Init();

    try {
        loadScriptFunctions();
        tryCall(m_pyOnInit);
    } catch (py::error_already_set& e) {
        Logger::Error("Python error: {}", e.what());
    }
}

void PythonScriptApp::OnUpdate(float deltaTime)
{
    m_deltaTime = deltaTime;

    Scene* scene = SceneManager::GetInstance().GetCurrentScene();
    if (scene) {
        scene->OnUpdate(deltaTime);
    }

    if (!m_pyOnUpdate.is_none()) {
        try {
            m_pyOnUpdate(deltaTime);
        } catch (py::error_already_set& e) {
            Logger::Error("Python on_update error: {}", e.what());
        }
    }
}

void PythonScriptApp::OnRender()
{
    RenderCommand::Clear();

    m_renderer->BeginScene(*m_gameCamera);

    Scene* scene = SceneManager::GetInstance().GetCurrentScene();
    if (scene) {
        scene->OnRender(*m_renderer);
    }

    if (!m_pyOnRender.is_none()) {
        try {
            m_pyOnRender();
        } catch (py::error_already_set& e) {
            Logger::Error("Python on_render error: {}", e.what());
        }
    }

    m_renderer->EndScene();

    m_renderer->BeginScene(*m_uiCamera);

    UISystem& ui = UISystem::GetInstance();
    Widget* uiRoot = ui.GetUIRoot();
    if (uiRoot) {
        Mat4 identity = Mat4::Identity();
        static_cast<Node*>(uiRoot)->Visit(*m_renderer, identity, 1.0f);
    }

    m_renderer->EndScene();
}

void PythonScriptApp::OnShutdown()
{
    tryCall(m_pyOnShutdown);

    SceneManager::GetInstance().PopScene();
    InputSystem::GetInstance().Clear();
    m_renderer->Shutdown();
    m_renderer.reset();

    Logger::Info("PythonScriptApp shutting down");
}
