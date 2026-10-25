#pragma once

#include "core/IScene.hpp"
#include "core/GameContext.hpp"

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

private:
    core::GameContext& m_ctx;

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
    float m_titleTimer{0.0f};
    bool m_cursorLocked{false};
};

} // namespace scenes
