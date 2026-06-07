#include <Application.h>
#include <Logger.h>
#include <Render/Renderer.h>
#include <Render/RenderCommand.h>
#include <Render/OrthographicCamera.h>
#include <Render/Texture.h>

#include <memory>
#include <glad/glad.h>

class Sandbox : public Application {
protected:
    void OnInit() override {
        Logger::Info("Sandbox initialized");

        RenderCommand::SetClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        RenderCommand::SetBlend(true);
        RenderCommand::SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_camera = std::unique_ptr<OrthographicCamera>(new OrthographicCamera(-16.0f, 16.0f, -9.0f, 9.0f));
        m_renderer = std::unique_ptr<Renderer>(new Renderer());
        m_renderer->Init();
        m_texture = std::unique_ptr<Texture>(new Texture("assets/textures/checkerboard.png"));
    }

    void OnUpdate(float deltaTime) override {
        (void)deltaTime;
    }

    void OnRender() override {
        RenderCommand::Clear();

        m_renderer->BeginScene(*m_camera);

        float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        float red[4] = { 1.0f, 0.2f, 0.2f, 1.0f };

        m_renderer->DrawQuad(Vec3(0.0f, 0.0f, 0.0f), Vec3(2.0f, 2.0f, 1.0f), 0.0f, m_texture.get(), white);
        m_renderer->DrawQuad(Vec3(3.0f, 0.0f, 0.0f), Vec3(2.0f, 2.0f, 1.0f), 0.0f, nullptr, red);

        m_renderer->EndScene();
    }

    void OnShutdown() override {
        Logger::Info("Sandbox shutting down");
        m_texture.reset();
        m_renderer.reset();
        m_camera.reset();
    }

private:
    std::unique_ptr<OrthographicCamera> m_camera;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Texture> m_texture;
};

int main() {
    Sandbox app;
    app.Run();
    return 0;
}
