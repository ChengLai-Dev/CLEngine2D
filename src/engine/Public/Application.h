#pragma once

#include <memory>

class Window;

class Application {
public:
    Application();
    virtual ~Application();

    void Run();

protected:
    virtual void OnInit() {}
    virtual void OnUpdate(float deltaTime) { (void)deltaTime; }
    virtual void OnRender() {}
    virtual void OnShutdown() {}

    Window* GetWindow() const { return m_window.get(); }

private:
    std::unique_ptr<Window> m_window;
};
