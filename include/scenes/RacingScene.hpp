#pragma once

#include "core/IScene.hpp"
#include "core/GameContext.hpp"
#include "renderer/SceneRenderer.hpp"
#include "ui/UIRenderer.hpp"
#include "ui/PauseMenuUI.hpp"

namespace scenes {

class RacingScene : public core::IScene {
public:
    explicit RacingScene(core::GameContext& ctx);
    ~RacingScene() override = default;

    void onEnter() override;
    void onExit() override;
    void handleInput(float deltaTime) override;
    void update(float deltaTime) override;
    void render(int windowWidth, int windowHeight) override;

    [[nodiscard]] int getLapsCompleted() const { return m_lapsCompleted; }
    [[nodiscard]] float getCurrentLapTime() const { return m_currentLapTime; }
    [[nodiscard]] float getBestLapTime() const { return m_bestLapTime; }
    [[nodiscard]] bool isPaused() const { return m_isPaused; }
    void setPaused(bool paused) {
        m_isPaused = paused;
        if (paused) {
            m_ctx.getAudio().stopAllGameplaySounds();
            m_pauseMenu.open();
        } else {
            m_pauseMenu.close();
        }
    }
    [[nodiscard]] int getPauseSelectedOption() const { return m_pauseMenu.getSelectedOption(); }
    void setPauseSelectedOption(int opt) { m_pauseMenu.setSelectedOption(opt); }

private:
    void resetRace();

    core::GameContext& m_ctx;
    renderer::SceneRenderer m_sceneRenderer;
    ui::UIRenderer m_uiRenderer;
    ui::PauseMenuUI m_pauseMenu;

    bool m_isPaused{false};

    // Telemetría y vueltas
    float m_finishLineCooldown{8.0f};
    int m_lapsCompleted{0};
    int m_totalLaps{3};
    float m_currentLapTime{0.0f};
    float m_bestLapTime{0.0f};
    float m_lastLapTime{0.0f};
    float m_totalRaceTime{0.0f};
    float m_lapNoticeTimer{0.0f};
    bool m_isNewRecord{false};

    float m_elapsedTime{0.0f};
    float m_impactAudioCooldown{0.0f};
    bool m_wasBoostingLastFrame{false};
    bool m_cursorLocked{false};
};

} // namespace scenes
