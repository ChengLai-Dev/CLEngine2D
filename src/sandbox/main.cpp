#include <Application.h>
#include <Platform/Window.h>
#include <Logger.h>
#include <Timer.h>
#include <AssetManager.h>
#include <Scene.h>
#include <Render/Renderer.h>
#include <Render/RenderCommand.h>
#include <Render/OrthographicCamera.h>
#include <Render/Texture.h>

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

static const char* TITLE = "CLEngine2D - Sandbox";

class GameScene : public Scene {
public:
    GameScene() {
        auto& assets = AssetManager::GetInstance();
        auto texture = assets.LoadTexture("assets/textures/checkerboard.png");

        float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        float red[4] = { 1.0f, 0.2f, 0.2f, 1.0f };
        float blue[4] = { 0.2f, 0.4f, 1.0f, 1.0f };

        AddSprite(Vec3(0.0f, 0.0f, 0.0f), Vec3(2.0f, 2.0f, 1.0f),
                  0.0f, texture, white);
        AddSprite(Vec3(3.0f, 0.0f, 0.0f), Vec3(2.0f, 2.0f, 1.0f),
                  0.0f, nullptr, red);
        AddSprite(Vec3(-3.0f, 0.0f, 0.0f), Vec3(2.0f, 2.0f, 1.0f),
                  0.0f, nullptr, blue);
    }

    void OnUpdate(float deltaTime) override {
        m_elapsed += deltaTime;

        auto* sprite = GetSprite(1);
        if (sprite) {
            sprite->Rotation += deltaTime * 60.0f;
        }
    }

private:
    float m_elapsed = 0.0f;
};

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

        auto& sceneManager = SceneManager::GetInstance();
        sceneManager.PushScene(std::unique_ptr<Scene>(new GameScene()));
    }

    void OnUpdate(float deltaTime) override {
        m_timer.TickFrame();

        auto* scene = SceneManager::GetInstance().GetCurrentScene();
        if (scene) {
            scene->OnUpdate(deltaTime);
        }

        if (m_fpsUpdateTimer >= 0.25f) {
            std::string title = std::string(TITLE)
                + " [FPS: " + std::to_string(static_cast<int>(m_timer.GetFPS()))
                + ", Frame: " + std::to_string(m_timer.GetFrameTime() * 1000.0f).substr(0, 5) + "ms]";
            glfwSetWindowTitle(GetWindow()->GetNativeWindow(), title.c_str());
            m_fpsUpdateTimer = 0.0f;
        }
        m_fpsUpdateTimer += deltaTime;
    }

    void OnRender() override {
        RenderCommand::Clear();

        m_renderer->BeginScene(*m_camera);

        auto* scene = SceneManager::GetInstance().GetCurrentScene();
        if (scene) {
            scene->OnRender(*m_renderer);
        }

        m_renderer->EndScene();
    }

    void OnShutdown() override {
        Logger::Info("Sandbox shutting down");
        SceneManager::GetInstance().PopScene();
        m_renderer.reset();
        m_camera.reset();
    }

private:
    Timer m_timer;
    std::unique_ptr<OrthographicCamera> m_camera;
    std::unique_ptr<Renderer> m_renderer;
    float m_fpsUpdateTimer = 0.0f;
};

int main() {
    Sandbox app;
    app.Run();
    return 0;
}
