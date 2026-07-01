#include "Platform/Window.h"
#include "Input.h"
#include "Utils.h"

#include "Logger.h"

#include <GLFW/glfw3.h>
#include <format>

Window::Window(const std::string& title, int width, int height)
    : m_title(title)
    , m_width(width)
    , m_height(height)
{
    Init();
}

Window::~Window() {
    Shutdown();
}

void Window::Init() {
    Logger::Info(std::format("Creating window: {} ({}x{})", m_title, m_width, m_height));

    if (!glfwInit()) {
        Logger::Fatal("Failed to initialize GLFW");
        return;
    }

    glfwSetErrorCallback([](int error, const char* desc) {
        Logger::Error(std::format("GLFW Error ({}): {}", error, desc));
    });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    if (IS_DEBUG) {
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    }

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        Logger::Fatal("Failed to create GLFW window");
        return;
    }

    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);
    glfwSwapInterval(1);

    glfwSetWindowSizeCallback(m_window, WindowResizeCallback);
    glfwSetWindowCloseCallback(m_window, WindowCloseCallback);
    glfwSetWindowFocusCallback(m_window, WindowFocusCallback);
    glfwSetKeyCallback(m_window, KeyCallback);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetCursorPosCallback(m_window, CursorPosCallback);
    glfwSetScrollCallback(m_window, ScrollCallback);
}

void Window::Shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void Window::OnUpdate() {
    glfwPollEvents();
    glfwSwapBuffers(m_window);
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_window);
}

int Window::GetWidth() const { return m_width; }
int Window::GetHeight() const { return m_height; }
GLFWwindow* Window::GetNativeWindow() const { return m_window; }

void* Window::GetProcAddress(const char* name) {
    return reinterpret_cast<void*>(glfwGetProcAddress(name));
}

double Window::GetTime() {
    return glfwGetTime();
}

void Window::WindowResizeCallback(GLFWwindow* window, int width, int height) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->m_width = width;
        self->m_height = height;
    }
}

void Window::WindowCloseCallback(GLFWwindow* /*window*/) {
    Logger::Info("Window close requested");
}

void Window::WindowFocusCallback(GLFWwindow* /*window*/, int focused) {
    if (focused)
        Logger::Debug("Window focused");
    else
        Logger::Debug("Window unfocused");
}

void Window::KeyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
    Input::OnKeyEvent(key, action);
}

void Window::MouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
    Input::OnMouseButtonEvent(button, action);
}

void Window::CursorPosCallback(GLFWwindow* /*window*/, double x, double y) {
    Input::OnMouseMoveEvent(x, y);
}

void Window::ScrollCallback(GLFWwindow* /*window*/, double xOffset, double yOffset) {
    Input::OnScrollEvent(xOffset, yOffset);
}
