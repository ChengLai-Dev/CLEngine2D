#include "Application.h"
#include "Logger.h"
#include "Platform/Window.h"
#include "Render/Renderer.h"

Application::Application() {
    Logger::Init();
    Logger::Info("CLEngine2D v0.1.0 initializing...");

    m_window = std::unique_ptr<Window>(new Window("CLEngine2D", 1280, 720));

    if (!Renderer::InitGL()) {
        Logger::Fatal("Failed to initialize OpenGL");
        return;
    }
}

Application::~Application() {
}

void Application::Run() {
    Logger::Info("Engine started");

    OnInit();

    double lastTime = Window::GetTime();

    while (!m_window->ShouldClose()) {
        double currentTime = Window::GetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);

        OnUpdate(deltaTime);
        OnRender();

        m_window->OnUpdate();
    }

    OnShutdown();

    Logger::Info("Engine shutting down");
}
