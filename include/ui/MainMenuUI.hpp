#pragma once

#include <memory>
#include <vector>
#include <string>
#include "renderer/Texture.hpp"
#include "ui/UIRenderer.hpp"

namespace core {
class GameContext;
}

namespace ui {

enum class MainMenuOption {
    PLAY = 0,
    SETTINGS = 1,
    QUIT = 2
};

enum class SettingsOption {
    SHADING_MODEL = 0,
    DAY_NIGHT = 1,
    HEADLIGHTS = 2,
    TRACK_LIGHTS = 3,
    INITIAL_CAMERA = 4,
    BACK = 5
};

/**
 * @brief Interfaz gráfica interactiva del Menú Principal y Panel de Configuración
 * utilizando los assets del paquete estándar de Kenney (Kenney UI Pack).
 */
class MainMenuUI {
public:
    MainMenuUI();
    ~MainMenuUI();

    bool init(UIRenderer* renderer, core::GameContext* ctx);

    void handleInput(float deltaTime);
    void update(float deltaTime);
    void render(float screenWidth, float screenHeight);

    bool isSettingsOpen() const { return m_inSettings; }
    void openSettings() { m_inSettings = true; m_selectedSettingsOption = SettingsOption::SHADING_MODEL; }
    void closeSettings() { m_inSettings = false; }

private:
    void renderMainMenu(float screenWidth, float screenHeight);
    void renderSettingsModal(float screenWidth, float screenHeight);

    UIRenderer* m_renderer{nullptr};
    core::GameContext* m_ctx{nullptr};

    // Texturas de Kenney UI Pack estándar
    renderer::Texture m_btnIdle;
    renderer::Texture m_btnHover;
    renderer::Texture m_btnActive;
    renderer::Texture m_arrowPointer;
    renderer::Texture m_panelFrame;
    renderer::Texture m_checkOn;
    renderer::Texture m_checkOff;

    MainMenuOption m_selectedOption{MainMenuOption::PLAY};
    SettingsOption m_selectedSettingsOption{SettingsOption::SHADING_MODEL};

    bool m_inSettings{false};
    float m_animTimer{0.0f};
    bool m_initialized{false};
};

} // namespace ui
