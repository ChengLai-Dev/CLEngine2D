#include "Application.h"
#include "Input/InputSystem.h"
#include "Logger.h"
#include "Platform/Window.h"
#include "Render/Renderer.h"
#include "Audio/AudioEngine.h"
#include "SceneGraph/UISystem.h"

Application::Application() {
    Logger::Init();
    Logger::Info("CLEngine2D v0.1.0 initializing...");

    m_window = std::unique_ptr<Window>(new Window("CLEngine2D", 1280, 720));

    if (!Renderer::InitGL()) {
        Logger::Fatal("Failed to initialize OpenGL");
        return;
    }

    AudioEngine::GetInstance().Init();
}

Application::~Application() {
}

void Application::Run() {
    Logger::Info("Engine started");

    m_window->SetResizeCallback([this](int w, int h) {
        OnWindowResize(w, h);
    });

    OnInit();

    double lastTime = Window::GetTime();

    while (!m_window->ShouldClose()) {
        double currentTime = Window::GetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);

        InputSystem::GetInstance().PollEvents();

        EInputMode mode = InputSystem::GetInstance().GetInputMode();
        if (mode == EInputMode::GameAndUI || mode == EInputMode::UIOnly) {
            InputSystem::GetInstance().ResetUIConsumedFlags();
            UISystem::GetInstance().ProcessEvents();
        }

        OnUpdate(deltaTime);

        if (mode != EInputMode::UIOnly) {
            InputSystem::GetInstance().Advance();
        }

        OnRender();
        m_window->SwapBuffers();
        lastTime = currentTime;
    }

    OnShutdown();

    Logger::Info("Engine shutting down");
}
