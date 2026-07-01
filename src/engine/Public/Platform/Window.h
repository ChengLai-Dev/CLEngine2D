#pragma once

#include <string>

struct GLFWwindow;

class Window {
public:
    Window(const std::string& title, int width, int height);
    ~Window();

    void OnUpdate();
    bool ShouldClose() const;

    int GetWidth() const;
    int GetHeight() const;
    GLFWwindow* GetNativeWindow() const;

    static void* GetProcAddress(const char* name);
    static double GetTime();

private:
    void Init();
    void Shutdown();

    static void WindowResizeCallback(GLFWwindow* window, int width, int height);
    static void WindowCloseCallback(GLFWwindow* window);
    static void WindowFocusCallback(GLFWwindow* window, int focused);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double x, double y);
    static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

    GLFWwindow* m_window = nullptr;
    std::string m_title;
    int m_width;
    int m_height;
};
