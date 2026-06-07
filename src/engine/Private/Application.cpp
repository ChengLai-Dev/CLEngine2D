#include "Application.h"
#include "Logger.h"
#include "Platform/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <format>

Application::Application()
    : m_window(nullptr)
{
    Logger::Init();
    Logger::Info("CLEngine2D v0.1.0 initializing...");

    if (!glfwInit()) {
        Logger::Fatal("Failed to initialize GLFW");
        return;
    }

    m_window = std::unique_ptr<Window>(new Window("CLEngine2D", 1280, 720));

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Logger::Fatal("Failed to initialize Glad");
        return;
    }

    Logger::Info(std::format("OpenGL {}.{} loaded", GLVersion.major, GLVersion.minor));
    Logger::Info("Renderer: " + std::string((const char*)glGetString(GL_RENDERER)));
    Logger::Info("Vendor: " + std::string((const char*)glGetString(GL_VENDOR)));
    Logger::Info("GLSL Version: " + std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)));
}

Application::~Application() {
    glfwTerminate();
}

void Application::Run() {
    Logger::Info("Engine started");

    OnInit();

    double lastTime = glfwGetTime();

    while (!m_window->ShouldClose()) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);

        OnUpdate(deltaTime);
        OnRender();

        m_window->OnUpdate();
    }

    OnShutdown();

    Logger::Info("Engine shutting down");
}
