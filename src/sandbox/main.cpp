#include <Application.h>
#include <Input/RawInput.h>
#include <Input/InputAction.h>
#include <Input/InputMappingContext.h>
#include <Input/InputSystem.h>
#include <Input/InputCodes.h>
#include <Platform/Window.h>
#include <Logger.h>
#include <Timer.h>
#include <AssetManager.h>
#include <Scene.h>
#include <SceneGraph/Sprite.h>
#include <SceneGraph/Widget.h>
#include <SceneGraph/Button.h>
#include <SceneGraph/Label.h>
#include <SceneGraph/CanvasPanel.h>
#include <SceneGraph/UISystem.h>
#include <Render/Renderer.h>
#include <Render/RenderCommand.h>
#include <Render/OrthographicCamera.h>
#include <Render/Texture.h>
#include <Audio/AudioEngine.h>

#include "UIDemoScene.h"

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

static const char* TITLE = "CLEngine2D - Sandbox";

class GameScene : public Scene {
public:
    GameScene() {
        AssetManager& assets = AssetManager::GetInstance();
        std::shared_ptr<Texture> texture = assets.LoadTexture("assets/textures/checkerboard.png");

        float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        float red[4] = { 1.0f, 0.2f, 0.2f, 1.0f };
        float blue[4] = { 0.2f, 0.4f, 1.0f, 1.0f };

        m_playerSprite = CreateSprite(Vec3(0.0f, 0.0f, 0.0f), Vec2(2.0f, 2.0f), texture, white);
        m_redSprite = CreateSprite(Vec3(3.0f, 0.0f, 0.0f), Vec2(2.0f, 2.0f), nullptr, red);
        m_blueSprite = CreateSprite(Vec3(-3.0f, 0.0f, 0.0f), Vec2(2.0f, 2.0f), nullptr, blue);
    }

    void OnUpdate(float deltaTime) override {
        m_elapsed += deltaTime;

        if (m_rotateRed && m_redSprite) {
            m_redSprite->SetRotationZ(m_redSprite->GetRotationZ() + deltaTime * 60.0f);
        }

        if (m_playerSprite && m_moveDir.LengthSq() > 0.0f) {
            Vec3 newPos = m_playerSprite->GetPosition() + m_moveDir * m_speed * deltaTime;
            m_playerSprite->SetPosition(newPos);
        }
    }

    void SetMoveDir(const Vec3& dir) { m_moveDir = dir; }
    void ToggleRotation() { m_rotateRed = !m_rotateRed; }

private:
    float m_elapsed = 0.0f;
    Vec3 m_moveDir = Vec3(0.0f, 0.0f, 0.0f);
    float m_speed = 5.0f;
    bool m_rotateRed = true;

    Sprite* m_playerSprite = nullptr;
    Sprite* m_redSprite = nullptr;
    Sprite* m_blueSprite = nullptr;
};

class Sandbox : public Application {
protected:
    void OnInit() override {
        Logger::Info("Sandbox initialized");

        RenderCommand::SetClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        RenderCommand::SetBlend(true);
        RenderCommand::SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_camera = std::unique_ptr<OrthographicCamera>(new OrthographicCamera(-16.0f, 16.0f, 9.0f, -9.0f));
        m_uiCamera = std::unique_ptr<OrthographicCamera>(
            new OrthographicCamera(0.0f, static_cast<float>(GetWindow()->GetWidth()),
                                   static_cast<float>(GetWindow()->GetHeight()), 0.0f)
        );
        m_renderer = std::unique_ptr<Renderer>(new Renderer());
        m_renderer->Init();

        SceneManager& sceneManager = SceneManager::GetInstance();
        m_scene = new GameScene();
        sceneManager.PushScene(std::shared_ptr<Scene>(m_scene));

        m_uiScene = std::make_unique<UIDemoScene>();

        m_defaultContext = std::make_shared<InputMappingContext>();
        m_defaultContext->MapKey(m_quitAction, KeyCode::Escape);
        m_quitAction->OnStarted([this](const InputActionValue&) {
            glfwSetWindowShouldClose(GetWindow()->GetNativeWindow(), GLFW_TRUE);
        });

        m_defaultContext->MapMouse(m_toggleAction, MouseCode::ButtonLeft);
        m_toggleAction->OnStarted([this](const InputActionValue&) {
            m_scene->ToggleRotation();
        });

        m_defaultContext->MapKey(m_moveAction, KeyCode::W, Vec2(0.0f, -1.0f));
        m_defaultContext->MapKey(m_moveAction, KeyCode::S, Vec2(0.0f, 1.0f));
        m_defaultContext->MapKey(m_moveAction, KeyCode::D, Vec2(1.0f, 0.0f));
        m_defaultContext->MapKey(m_moveAction, KeyCode::A, Vec2(-1.0f, 0.0f));
        m_defaultContext->MapKey(m_moveAction, KeyCode::Up, Vec2(0.0f, -1.0f));
        m_defaultContext->MapKey(m_moveAction, KeyCode::Down, Vec2(0.0f, 1.0f));
        m_defaultContext->MapKey(m_moveAction, KeyCode::Right, Vec2(1.0f, 0.0f));
        m_defaultContext->MapKey(m_moveAction, KeyCode::Left, Vec2(-1.0f, 0.0f));

        InputSystem::GetInstance().AddContext(m_defaultContext, 0);
        m_moveAction->AccumulationBehavior = EInputActionAccumulationBehavior::Cumulative;
        m_moveAction->OnTriggered([this](const InputActionValue& val) {
            Vec2 dir = val.GetVec2();
            m_scene->SetMoveDir(dir.LengthSq() > 0.0f
                ? Vec3(dir.Normalized().x, dir.Normalized().y, 0.0f)
                : Vec3(0.0f, 0.0f, 0.0f));
        });
        m_moveAction->OnCompleted([this](const InputActionValue&) {
            m_scene->SetMoveDir(Vec3(0.0f, 0.0f, 0.0f));
        });

        Logger::Info("Controls: WASD/Arrows to move, Left Click to toggle rotation, Esc to quit");
        Logger::Info("Audio engine ready - place .wav files in assets/audio/ and use AudioEngine::GetInstance().LoadSound()");
    }

    void OnUpdate(float deltaTime) override {
        m_timer.TickFrame();

        Scene* scene = SceneManager::GetInstance().GetCurrentScene();
        if (scene) {
            scene->OnUpdate(deltaTime);
        }

        if (m_fpsUpdateTimer >= 0.25f) {
            Vec2 mousePos = RawInput::GetMousePosition();
            std::string title = std::string(TITLE)
                + " [FPS: " + std::to_string(static_cast<int>(m_timer.GetFPS()))
                + ", Mouse: " + std::to_string(static_cast<int>(mousePos.x))
                + "," + std::to_string(static_cast<int>(mousePos.y))
                + "]";
            glfwSetWindowTitle(GetWindow()->GetNativeWindow(), title.c_str());
            m_fpsUpdateTimer = 0.0f;
        }
        m_fpsUpdateTimer += deltaTime;
    }

    void OnRender() override {
        RenderCommand::Clear();

        m_renderer->BeginScene(*m_camera);

        Scene* scene = SceneManager::GetInstance().GetCurrentScene();
        if (scene) {
            scene->OnRender(*m_renderer);
        }

        m_renderer->EndScene();

        m_renderer->BeginScene(*m_uiCamera);
        if (m_uiScene) {
            m_uiScene->OnRender(*m_renderer);
        }
        m_renderer->EndScene();
    }

    void OnWindowResize(int width, int height) override {
        m_uiCamera->SetProjection(0.0f, static_cast<float>(width),
                                  static_cast<float>(height), 0.0f);
        if (m_uiScene) {
            m_uiScene->SetWindowSize(width, height);
        }
    }

    void OnShutdown() override {
        Logger::Info("Sandbox shutting down");
        SceneManager::GetInstance().PopScene();
        m_uiScene.reset();
        m_renderer.reset();
        m_camera.reset();
        m_uiCamera.reset();
    }

private:
    std::shared_ptr<InputAction> m_quitAction = std::make_shared<InputAction>();
    std::shared_ptr<InputAction> m_toggleAction = std::make_shared<InputAction>();
    std::shared_ptr<InputAction> m_moveAction = std::make_shared<InputAction>();
    std::shared_ptr<InputMappingContext> m_defaultContext;

    Timer m_timer;
    std::unique_ptr<OrthographicCamera> m_camera;
    std::unique_ptr<OrthographicCamera> m_uiCamera;
    std::unique_ptr<Renderer> m_renderer;
    GameScene* m_scene = nullptr;
    std::unique_ptr<UIDemoScene> m_uiScene;
    float m_fpsUpdateTimer = 0.0f;
};

int main() {
    Sandbox app;
    app.Run();
    return 0;
}
