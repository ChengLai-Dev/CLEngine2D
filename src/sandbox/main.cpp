#include <Application.h>
#include <Input.h>
#include <InputCodes.h>
#include <Platform/Window.h>
#include <Logger.h>
#include <Timer.h>
#include <AssetManager.h>
#include <Scene.h>
#include <Render/Renderer.h>
#include <Render/RenderCommand.h>
#include <Render/OrthographicCamera.h>
#include <Render/Texture.h>
#include <Audio/AudioEngine.h>

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

        if (m_rotateRed) {
            auto* redSprite = GetSprite(1);
            if (redSprite) {
                redSprite->Rotation += deltaTime * 60.0f;
            }
        }

        auto* playerSprite = GetSprite(0);
        if (playerSprite && m_moveDir.LengthSq() > 0.0f) {
            Vec3 move = m_moveDir * m_speed * deltaTime;
            playerSprite->Position = playerSprite->Position + move;
        }
    }

    void SetMoveDir(const Vec3& dir) { m_moveDir = dir; }
    void ToggleRotation() { m_rotateRed = !m_rotateRed; }

private:
    float m_elapsed = 0.0f;
    Vec3 m_moveDir = Vec3(0.0f, 0.0f, 0.0f);
    float m_speed = 5.0f;
    bool m_rotateRed = true;
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
        m_scene = new GameScene();
        sceneManager.PushScene(std::unique_ptr<Scene>(m_scene));

        Logger::Info("Controls: WASD/Arrows to move, Space to toggle rotation, Esc to quit");
        Logger::Info("Audio engine ready - place .wav files in assets/audio/ and use AudioEngine::GetInstance().LoadSound()");
    }

    void OnUpdate(float deltaTime) override {
        m_timer.TickFrame();

        HandleInput();

        auto* scene = SceneManager::GetInstance().GetCurrentScene();
        if (scene) {
            scene->OnUpdate(deltaTime);
        }

        if (m_fpsUpdateTimer >= 0.25f) {
            auto mousePos = Input::GetMousePosition();
            std::string title = std::string(TITLE)
                + " [FPS: " + std::to_string(static_cast<int>(m_timer.GetFPS()))
                + ", Mouse: " + std::to_string(static_cast<int>(mousePos.first))
                + "," + std::to_string(static_cast<int>(mousePos.second))
                + "]";
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
    void HandleInput() {
        if (Input::IsKeyPressed(KeyCode::Escape)) {
            glfwSetWindowShouldClose(GetWindow()->GetNativeWindow(), GLFW_TRUE);
        }

        if (Input::IsMouseButtonPressed(MouseCode::ButtonLeft)) {
            m_scene->ToggleRotation();
        }

        Vec3 moveDir(0.0f, 0.0f, 0.0f);
        if (Input::IsKeyDown(KeyCode::W) || Input::IsKeyDown(KeyCode::Up)) {
            moveDir.y += 1.0f;
        }
        if (Input::IsKeyDown(KeyCode::S) || Input::IsKeyDown(KeyCode::Down)) {
            moveDir.y -= 1.0f;
        }
        if (Input::IsKeyDown(KeyCode::A) || Input::IsKeyDown(KeyCode::Left)) {
            moveDir.x -= 1.0f;
        }
        if (Input::IsKeyDown(KeyCode::D) || Input::IsKeyDown(KeyCode::Right)) {
            moveDir.x += 1.0f;
        }

        if (moveDir.LengthSq() > 0.0f) {
            m_scene->SetMoveDir(moveDir.Normalized());
        } else {
            m_scene->SetMoveDir(Vec3(0.0f, 0.0f, 0.0f));
        }
    }

    Timer m_timer;
    std::unique_ptr<OrthographicCamera> m_camera;
    std::unique_ptr<Renderer> m_renderer;
    GameScene* m_scene = nullptr;
    float m_fpsUpdateTimer = 0.0f;
};

int main() {
    Sandbox app;
    app.Run();
    return 0;
}
