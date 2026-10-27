#pragma once

#include "core/IScene.hpp"
#include "core/GameContext.hpp"
#include "renderer/SceneRenderer.hpp"
#include "ui/UIRenderer.hpp"
#include "ui/MainMenuUI.hpp"

namespace scenes {

class MainMenuScene : public core::IScene {
public:
    explicit MainMenuScene(core::GameContext& ctx);
    ~MainMenuScene() override = default;

    void onEnter() override;
    void onExit() override;
    void handleInput(float deltaTime) override;
    void update(float deltaTime) override;
    void render(int windowWidth, int windowHeight) override;

private:
    core::GameContext& m_ctx;
    renderer::SceneRenderer m_sceneRenderer;
    ui::UIRenderer m_uiRenderer;
    ui::MainMenuUI m_menuUI;

    float m_menuOrbitAngle{0.0f};
    float m_elapsedTime{0.0f};
    float m_titleTimer{0.0f};
};

} // namespace scenes
