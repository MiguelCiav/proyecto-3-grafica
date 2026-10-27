#include "ui/MainMenuUI.hpp"
#include "ui/UIUtils.hpp"
#include "ui/UIWidgets.hpp"
#include "core/GameContext.hpp"
#include "core/Input.hpp"
#include "core/Window.hpp"
#include "core/SceneManager.hpp"
#include "lighting/LightManager.hpp"
#include "lighting/LightingController.hpp"
#include "audio/SoundEvent.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace ui {

MainMenuUI::MainMenuUI() = default;
MainMenuUI::~MainMenuUI() = default;

bool MainMenuUI::init(UIRenderer* renderer, core::GameContext* ctx) {
    m_renderer = renderer;
    m_ctx = ctx;

    if (!m_renderer) {
        std::cerr << "[ERROR] [MainMenuUI] Puntero UIRenderer nulo recibido en init()." << std::endl;
        return false;
    }

    auto loadTexture = [](renderer::Texture& tex, const std::string& subpath) -> bool {
        std::string p2x = "assets/kenney_ui-pack/PNG/" + subpath;
        size_t idx = p2x.find("/Default/");
        if (idx != std::string::npos) {
            p2x.replace(idx, 9, "/Double/");
        }
        if (tex.loadFromFile(resolveAssetPath(p2x), false)) {
            return true;
        }
        return tex.loadFromFile(resolveAssetPath("assets/kenney_ui-pack/PNG/" + subpath), false);
    };

    bool ok = true;
    ok &= loadTexture(m_btnIdle, "Blue/Default/button_rectangle_depth_flat.png");
    ok &= loadTexture(m_btnHover, "Yellow/Default/button_rectangle_depth_gradient.png");
    ok &= loadTexture(m_btnActive, "Green/Default/button_rectangle_depth_border.png");
    ok &= loadTexture(m_arrowPointer, "Yellow/Default/arrow_decorative_e.png");
    ok &= loadTexture(m_panelFrame, "Blue/Default/button_square_border.png");
    ok &= loadTexture(m_checkOn, "Yellow/Default/check_square_color_checkmark.png");
    ok &= loadTexture(m_checkOff, "Grey/Default/check_square_grey.png");

    if (!ok) {
        std::cerr << "[WARN] [MainMenuUI] Alguna textura del Kenney UI Pack no se pudo cargar desde assets/kenney_ui-pack/PNG/." << std::endl;
    }

    m_selectedOption = MainMenuOption::PLAY;
    m_selectedSettingsOption = SettingsOption::SHADING_MODEL;
    m_inSettings = false;
    m_animTimer = 0.0f;
    m_initialized = true;

    std::cout << "[OK] [MainMenuUI] Interfaz de Menu Principal y Configuracion inicializada con exito." << std::endl;
    return true;
}

void MainMenuUI::update(float deltaTime) {
    m_animTimer += deltaTime;
}

void MainMenuUI::handleInput(float deltaTime) {
    (void)deltaTime;
    if (!m_initialized) return;

    float sw = (m_ctx && m_ctx->window) ? static_cast<float>(m_ctx->window->getWidth()) : 1280.0f;
    float sh = (m_ctx && m_ctx->window) ? static_cast<float>(m_ctx->window->getHeight()) : 720.0f;
    float mx = core::Input::getMouseX();
    float my = core::Input::getMouseY();
    bool mouseClicked = core::Input::isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT);

    auto closeAppWindow = [this]() {
        if (m_ctx && m_ctx->window) {
            glfwSetWindowShouldClose(m_ctx->window->getNativeWindow(), GLFW_TRUE);
        } else if (GLFWwindow* win = glfwGetCurrentContext()) {
            glfwSetWindowShouldClose(win, GLFW_TRUE);
        }
    };

    if (!m_inSettings) {
        const MainMenuOption prevOption = m_selectedOption;

        if (core::Input::isKeyJustPressed(GLFW_KEY_UP) || core::Input::isKeyJustPressed(GLFW_KEY_W)) {
            m_selectedOption = static_cast<MainMenuOption>((static_cast<int>(m_selectedOption) + 2) % 3);
        }
        if (core::Input::isKeyJustPressed(GLFW_KEY_DOWN) || core::Input::isKeyJustPressed(GLFW_KEY_S)) {
            m_selectedOption = static_cast<MainMenuOption>((static_cast<int>(m_selectedOption) + 1) % 3);
        }

        if (core::Input::isKeyJustPressed(GLFW_KEY_1) || core::Input::isKeyJustPressed(GLFW_KEY_KP_1)) {
            m_selectedOption = MainMenuOption::PLAY;
        } else if (core::Input::isKeyJustPressed(GLFW_KEY_2) || core::Input::isKeyJustPressed(GLFW_KEY_KP_2)) {
            m_selectedOption = MainMenuOption::SETTINGS;
        } else if (core::Input::isKeyJustPressed(GLFW_KEY_3) || core::Input::isKeyJustPressed(GLFW_KEY_KP_3)) {
            m_selectedOption = MainMenuOption::QUIT;
        }

        bool activate = core::Input::isKeyJustPressed(GLFW_KEY_ENTER) ||
                        core::Input::isKeyJustPressed(GLFW_KEY_KP_ENTER) ||
                        core::Input::isKeyJustPressed(GLFW_KEY_SPACE);

        const float btnW = 300.0f, btnH = 56.0f, spacing = 72.0f;
        const float btnX = (sw - btnW) * 0.5f;
        const float startY = sh * 0.44f;

        for (int i = 0; i < 3; ++i) {
            UIRect btnRect{btnX, startY + static_cast<float>(i) * spacing, btnW, btnH};
            if (btnRect.contains(mx, my)) {
                m_selectedOption = static_cast<MainMenuOption>(i);
                if (mouseClicked) activate = true;
            }
        }

        if (m_selectedOption != prevOption && m_ctx) {
            m_ctx->getAudio().playSound(audio::SoundEvent::UI_NAVIGATE);
        }

        if (activate) {
            if (m_selectedOption == MainMenuOption::PLAY) {
                if (m_ctx) m_ctx->getAudio().playSound(audio::SoundEvent::UI_CONFIRM);
                std::cout << "[MENU] Opcion JUGAR activada -> Cambiando a VEHICLE_SELECT..." << std::endl;
                if (m_ctx && m_ctx->sceneManager) {
                    m_ctx->selectionCategory = core::SelectionCategory::DRIVER;
                    m_ctx->sceneManager->changeScene(core::GameState::VEHICLE_SELECT);
                }
            } else if (m_selectedOption == MainMenuOption::SETTINGS) {
                if (m_ctx) m_ctx->getAudio().playSound(audio::SoundEvent::UI_CONFIRM);
                std::cout << "[MENU] Opcion CONFIGURACION activada -> Abriendo panel modal..." << std::endl;
                openSettings();
            } else if (m_selectedOption == MainMenuOption::QUIT) {
                std::cout << "[MENU] Opcion SALIR activada -> Cerrando juego..." << std::endl;
                closeAppWindow();
            }
        }

        if (core::Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            std::cout << "[MENU] ESC presionado en Menu -> Saliendo del juego..." << std::endl;
            closeAppWindow();
        }
    } else {
        if (core::Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            if (m_ctx) m_ctx->getAudio().playSound(audio::SoundEvent::UI_BACK);
            closeSettings();
            return;
        }

        const SettingsOption prevSettingsOption = m_selectedSettingsOption;

        if (core::Input::isKeyJustPressed(GLFW_KEY_UP) || core::Input::isKeyJustPressed(GLFW_KEY_W)) {
            m_selectedSettingsOption = static_cast<SettingsOption>((static_cast<int>(m_selectedSettingsOption) + 5) % 6);
        }
        if (core::Input::isKeyJustPressed(GLFW_KEY_DOWN) || core::Input::isKeyJustPressed(GLFW_KEY_S)) {
            m_selectedSettingsOption = static_cast<SettingsOption>((static_cast<int>(m_selectedSettingsOption) + 1) % 6);
        }

        bool goLeft = core::Input::isKeyJustPressed(GLFW_KEY_LEFT) || core::Input::isKeyJustPressed(GLFW_KEY_A);
        bool goRight = core::Input::isKeyJustPressed(GLFW_KEY_RIGHT) || core::Input::isKeyJustPressed(GLFW_KEY_D);
        bool activate = core::Input::isKeyJustPressed(GLFW_KEY_ENTER) ||
                        core::Input::isKeyJustPressed(GLFW_KEY_KP_ENTER) ||
                        core::Input::isKeyJustPressed(GLFW_KEY_SPACE);

        const float modalW = 660.0f, modalH = 460.0f;
        const float modalX = (sw - modalW) * 0.5f;
        const float modalY = (sh - modalH) * 0.5f;
        const float optStartY = modalY + 68.0f;
        const float rowHeight = 52.0f;

        for (int i = 0; i < 6; ++i) {
            UIRect rowRect{modalX + 20.0f, optStartY + static_cast<float>(i) * rowHeight, modalW - 40.0f, rowHeight - 6.0f};
            if (rowRect.contains(mx, my)) {
                m_selectedSettingsOption = static_cast<SettingsOption>(i);
                if (mouseClicked) activate = true;
            }
        }

        if (m_selectedSettingsOption != prevSettingsOption && m_ctx) {
            m_ctx->getAudio().playSound(audio::SoundEvent::UI_NAVIGATE);
        }

        if (goLeft || goRight || activate) {
            switch (m_selectedSettingsOption) {
                case SettingsOption::SHADING_MODEL:
                    if (m_ctx) m_ctx->getAudio().playSound(audio::SoundEvent::UI_SWITCH);
                    if (m_ctx && m_ctx->lightManager) {
                        if (goLeft) {
                            int prev = (static_cast<int>(m_ctx->lightManager->getLightingModel()) + 2) % 3;
                            m_ctx->lightManager->setLightingModel(static_cast<lighting::LightingModel>(prev));
                        } else {
                            m_ctx->lightManager->cycleNextModel();
                        }
                    }
                    break;
                case SettingsOption::DAY_NIGHT:
                    if (m_ctx) m_ctx->getAudio().playSound(audio::SoundEvent::UI_SWITCH);
                    if (m_ctx && m_ctx->lightingController) m_ctx->lightingController->toggleDayNight();
                    break;
                case SettingsOption::HEADLIGHTS:
                    if (m_ctx) m_ctx->getAudio().playSound(audio::SoundEvent::UI_SWITCH);
                    if (m_ctx && m_ctx->lightingController) m_ctx->lightingController->toggleHeadlights();
                    break;
                case SettingsOption::TRACK_LIGHTS:
                    if (m_ctx) m_ctx->getAudio().playSound(audio::SoundEvent::UI_SWITCH);
                    if (m_ctx && m_ctx->lightManager) m_ctx->lightManager->togglePointLights();
                    break;
                case SettingsOption::INITIAL_CAMERA:
                    if (m_ctx) {
                        m_ctx->getAudio().playSound(audio::SoundEvent::UI_SWITCH);
                        m_ctx->cameraMode = (m_ctx->cameraMode == core::CameraMode::CHASE)
                                            ? core::CameraMode::FPS : core::CameraMode::CHASE;
                    }
                    break;
                case SettingsOption::BACK:
                    if (m_ctx) m_ctx->getAudio().playSound(audio::SoundEvent::UI_BACK);
                    closeSettings();
                    break;
            }
        }
    }
}

void MainMenuUI::render(float screenWidth, float screenHeight) {
    if (!m_initialized || !m_renderer) return;

    m_renderer->begin(screenWidth, screenHeight);
    renderMainMenu(screenWidth, screenHeight);
    if (m_inSettings) {
        renderSettingsModal(screenWidth, screenHeight);
    }
    m_renderer->end();
}

void MainMenuUI::renderMainMenu(float screenWidth, float screenHeight) {
    float titleY = screenHeight * 0.16f;
    m_renderer->drawTextCenteredWithShadow(0.0f, titleY, screenWidth, 40.0f,
                                           "MARIO KART - CGI (UCV)", 4.0f,
                                           glm::vec4(1.0f, 0.86f, 0.15f, 1.0f),
                                           glm::vec4(0.0f, 0.0f, 0.0f, 0.85f), 2.0f);

    m_renderer->drawTextCenteredWithShadow(0.0f, titleY + 46.0f, screenWidth, 20.0f,
                                           "PROTOTIPO DE CARRERAS - OPENGL 3.3 CORE", 2.0f,
                                           glm::vec4(0.88f, 0.92f, 1.0f, 0.95f),
                                           glm::vec4(0.0f, 0.0f, 0.0f, 0.80f), 1.5f);

    const float btnW = 300.0f, btnH = 56.0f, spacing = 72.0f;
    const float btnX = (screenWidth - btnW) * 0.5f;
    const float startY = screenHeight * 0.44f;
    const char* labels[3] = { "JUGAR", "CONFIGURACION", "SALIR" };

    for (int i = 0; i < 3; ++i) {
        UIRect btnRect{btnX, startY + static_cast<float>(i) * spacing, btnW, btnH};
        bool isSelected = (!m_inSettings && static_cast<int>(m_selectedOption) == i);
        glm::vec4 txtColor = isSelected ? glm::vec4(0.12f, 0.12f, 0.14f, 1.0f) : glm::vec4(1.0f);

        UIButton::draw(*m_renderer, btnRect, m_btnIdle, m_btnHover, labels[i], 2.2f, isSelected, txtColor, 20.0f);
        if (isSelected) {
            UIButton::drawArrowIndicator(*m_renderer, m_arrowPointer,
                                         btnRect.x - 46.0f, btnRect.y + (btnRect.h - 32.0f) * 0.5f,
                                         32.0f, m_animTimer, 6.0f);
        }
    }

    m_renderer->drawTextCenteredWithShadow(0.0f, screenHeight - 44.0f, screenWidth, 20.0f,
                                           "[W/S/FLECHAS] Navegar | [ENTER/ESPACIO] Seleccionar | [ESC] Salir",
                                           1.8f, glm::vec4(0.90f, 0.92f, 0.98f, 0.85f),
                                           glm::vec4(0.0f, 0.0f, 0.0f, 0.85f), 1.5f);
}

void MainMenuUI::renderSettingsModal(float screenWidth, float screenHeight) {
    m_renderer->drawRect(0.0f, 0.0f, screenWidth, screenHeight, glm::vec4(0.0f, 0.0f, 0.0f, 0.75f));

    glm::vec2 modalPos = computeAnchoredPosition(UIAnchor::CENTER, 680.0f, 470.0f, screenWidth, screenHeight);
    UIRect modalRect{modalPos.x, modalPos.y, 680.0f, 470.0f};
    UIPanel::drawFramedPanel(*m_renderer, modalRect, m_panelFrame, 24.0f,
                             glm::vec4(0.04f, 0.06f, 0.10f, 0.98f),
                             glm::vec4(0.35f, 0.65f, 0.95f, 0.85f));

    UIRect innerRect{modalRect.x + 12.0f, modalRect.y + 12.0f, modalRect.w - 24.0f, modalRect.h - 24.0f};
    m_renderer->drawRect(innerRect.x, innerRect.y, innerRect.w, innerRect.h, glm::vec4(0.05f, 0.07f, 0.12f, 0.96f));
    m_renderer->drawBorder(innerRect.x, innerRect.y, innerRect.w, innerRect.h, 1.5f, glm::vec4(0.18f, 0.28f, 0.45f, 0.70f));

    m_renderer->drawTextCenteredWithShadow(modalRect.x, modalRect.y + 18.0f, modalRect.w, 30.0f,
                                           "CONFIGURACION DEL SISTEMA", 2.6f,
                                           glm::vec4(1.0f, 0.88f, 0.20f, 1.0f),
                                           glm::vec4(0.0f, 0.0f, 0.0f, 0.90f), 1.5f);
    m_renderer->drawRect(modalRect.x + 30.0f, modalRect.y + 54.0f, modalRect.w - 60.0f, 2.0f,
                         glm::vec4(0.20f, 0.35f, 0.60f, 0.75f));

    const float optStartY = modalRect.y + 68.0f;
    const float rowHeight = 52.0f;

    auto drawCheckboxRow = [this, &modalRect, rowHeight](float rowY, const char* label, const glm::vec4& labelColor,
                                                         bool checked, const char* onText, const char* offText, bool invertHighlight = false) {
        m_renderer->drawTextWithShadow(modalRect.x + 54.0f, rowY + 12.0f, label, 1.9f, labelColor);
        const renderer::Texture& chkTex = checked ? m_checkOn : m_checkOff;
        float chkSize = 28.0f;
        float chkX = modalRect.x + modalRect.w - 210.0f;
        float chkY = rowY + (rowHeight - 6.0f - chkSize) * 0.5f;
        m_renderer->drawSprite(chkTex, chkX, chkY, chkSize, chkSize);
        bool positive = invertHighlight ? !checked : checked;
        glm::vec4 stateColor = positive ? glm::vec4(0.20f, 0.96f, 0.64f, 1.0f) : glm::vec4(1.0f, 0.45f, 0.25f, 1.0f);
        m_renderer->drawTextWithShadow(chkX + chkSize + 12.0f, rowY + 12.0f, checked ? onText : offText, 1.8f, stateColor);
    };

    for (int i = 0; i < 6; ++i) {
        float rowY = optStartY + static_cast<float>(i) * rowHeight;
        bool isSelected = (m_selectedSettingsOption == static_cast<SettingsOption>(i));

        if (isSelected) {
            m_renderer->drawRect(modalRect.x + 18.0f, rowY - 2.0f, modalRect.w - 36.0f, rowHeight - 6.0f,
                                 glm::vec4(0.10f, 0.20f, 0.42f, 0.75f));
            m_renderer->drawBorder(modalRect.x + 18.0f, rowY - 2.0f, modalRect.w - 36.0f, rowHeight - 6.0f,
                                   1.5f, glm::vec4(1.0f, 0.843f, 0.0f, 1.0f));
            UIButton::drawArrowIndicator(*m_renderer, m_arrowPointer, modalRect.x + 24.0f, rowY + 10.0f, 22.0f, m_animTimer, 4.0f);
        }

        glm::vec4 labelColor = isSelected ? glm::vec4(1.0f, 0.95f, 0.40f, 1.0f) : glm::vec4(1.0f);
        glm::vec4 valColor = isSelected ? glm::vec4(0.0f, 0.90f, 1.0f, 1.0f) : glm::vec4(0.65f, 0.90f, 1.0f, 0.95f);

        switch (static_cast<SettingsOption>(i)) {
            case SettingsOption::SHADING_MODEL: {
                m_renderer->drawTextWithShadow(modalRect.x + 54.0f, rowY + 12.0f, "MODELO DE ILUMINACION", 1.9f, labelColor);
                const char* modelName = (m_ctx && m_ctx->lightManager) ? m_ctx->lightManager->getCurrentModelName() : "BLINN-PHONG";
                std::string valStr = "< " + sanitizeText(modelName) + " >";
                m_renderer->drawTextWithShadow(modalRect.x + modalRect.w - 270.0f, rowY + 12.0f, valStr, 1.8f, valColor);
                break;
            }
            case SettingsOption::DAY_NIGHT: {
                bool isNight = (m_ctx && m_ctx->lightingController) ? m_ctx->lightingController->isNight() : false;
                drawCheckboxRow(rowY, "CICLO DIA / NOCHE", labelColor, isNight, "NOCHE", "DIA", true);
                break;
            }
            case SettingsOption::HEADLIGHTS: {
                bool headlights = (m_ctx && m_ctx->lightingController) ? m_ctx->lightingController->areHeadlightsOn() : true;
                drawCheckboxRow(rowY, "FAROS DEL KART (SPOT)", labelColor, headlights, "ON", "OFF");
                break;
            }
            case SettingsOption::TRACK_LIGHTS: {
                bool pointsOn = (m_ctx && m_ctx->lightManager && !m_ctx->lightManager->getPointLights().empty())
                                ? m_ctx->lightManager->getPointLights()[0].enabled : true;
                drawCheckboxRow(rowY, "FAROLAS DE PISTA (POINTS)", labelColor, pointsOn, "ON", "OFF");
                break;
            }
            case SettingsOption::INITIAL_CAMERA: {
                m_renderer->drawTextWithShadow(modalRect.x + 54.0f, rowY + 12.0f, "MODO DE CAMARA", 1.9f, labelColor);
                bool isChase = (m_ctx && m_ctx->cameraMode == core::CameraMode::CHASE);
                m_renderer->drawTextWithShadow(modalRect.x + modalRect.w - 270.0f, rowY + 12.0f,
                                               isChase ? "< CHASE (3RA) >" : "< FPS LIBRE >", 1.8f, valColor);
                break;
            }
            case SettingsOption::BACK: {
                UIRect backRect{modalRect.x + (modalRect.w - 280.0f) * 0.5f, rowY + 3.0f, 280.0f, 38.0f};
                glm::vec4 bTextColor = isSelected ? glm::vec4(0.12f, 0.12f, 0.14f, 1.0f) : glm::vec4(1.0f);
                UIButton::draw(*m_renderer, backRect, m_btnIdle, m_btnHover, "VOLVER AL MENU", 1.8f, isSelected, bTextColor, 0.0f);
                break;
            }
        }
    }

    m_renderer->drawTextCenteredWithShadow(modalRect.x, modalRect.y + modalRect.h - 34.0f, modalRect.w, 20.0f,
                                           "[W/S/FLECHAS] Navegar | [ENTER/A/D] Modificar | [ESC] Volver",
                                           1.6f, glm::vec4(0.85f, 0.90f, 1.0f, 0.90f));
}

} // namespace ui
