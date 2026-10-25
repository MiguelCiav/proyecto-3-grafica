#pragma once

#include "core/IScene.hpp"
#include "core/GameContext.hpp"

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
    float m_menuOrbitAngle{0.0f};
    float m_elapsedTime{0.0f};
    float m_titleTimer{0.0f};
};

} // namespace scenes
