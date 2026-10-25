#pragma once

#include "core/IScene.hpp"
#include "core/GameContext.hpp"
#include "ui/ShowroomUI.hpp"

namespace scenes {

class ShowroomScene : public core::IScene {
public:
    explicit ShowroomScene(core::GameContext& ctx);
    ~ShowroomScene() override;

    void onEnter() override;
    void onExit() override;
    void handleInput(float deltaTime) override;
    void update(float deltaTime) override;
    void render(int windowWidth, int windowHeight) override;

    [[nodiscard]] ui::ShowroomUI& getUI() { return m_showroomUI; }
    [[nodiscard]] const ui::ShowroomUI& getUI() const { return m_showroomUI; }

private:
    core::GameContext& m_ctx;
    ui::ShowroomUI m_showroomUI;
    float m_showcaseAngle{0.0f};
    float m_titleTimer{0.0f};
};

} // namespace scenes
