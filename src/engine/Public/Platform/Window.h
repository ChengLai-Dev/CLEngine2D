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

private:
    void Init();
    void Shutdown();

    static void WindowResizeCallback(GLFWwindow* window, int width, int height);
    static void WindowCloseCallback(GLFWwindow* window);
    static void WindowFocusCallback(GLFWwindow* window, int focused);

    GLFWwindow* m_window;
    std::string m_title;
    int m_width;
    int m_height;
};
