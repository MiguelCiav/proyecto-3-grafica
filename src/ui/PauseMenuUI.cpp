#include "ui/PauseMenuUI.hpp"
#include "ui/UIRenderer.hpp"
#include "ui/UIUtils.hpp"
#include "core/Input.hpp"
#include "lighting/LightManager.hpp"
#include "lighting/LightingController.hpp"
#include "audio/IAudioService.hpp"
#include "audio/SoundEvent.hpp"
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <algorithm>

namespace ui {

PauseMenuUI::PauseMenuUI() = default;

bool PauseMenuUI::init(UIRenderer* renderer) {
    if (!renderer) {
        std::cerr << "[ERROR] [PauseMenuUI] UIRenderer provisto es nulo." << std::endl;
        return false;
    }

    m_renderer = renderer;

    const std::string spaceExpansionPath = "assets/kenney_ui-pack-space-expansion/PNG/";
    const std::string uiPackBasePath = "assets/kenney_ui-pack/PNG/";

    auto loadUiTexture = [](renderer::Texture& tex, const std::string& basePath, const std::string& relPath) -> bool {
        std::string doubleRel = relPath;
        size_t idx = doubleRel.find("/Default/");
        if (idx != std::string::npos) {
            doubleRel.replace(idx, 9, "/Double/");
        }
        std::string resolvedDouble = resolveAssetPath(basePath + doubleRel);
        if (tex.loadFromFile(resolvedDouble, false)) {
            return true;
        }
        std::string resolvedDefault = resolveAssetPath(basePath + relPath);
        return tex.loadFromFile(resolvedDefault, false);
    };

    bool ok = true;
    ok &= loadUiTexture(m_texPauseModalFrame, spaceExpansionPath, "Blue/Default/button_square_header_large_rectangle_screws.png");
    ok &= loadUiTexture(m_texPauseBtnIdle, uiPackBasePath, "Blue/Default/button_rectangle_depth_flat.png");
    ok &= loadUiTexture(m_texPauseBtnHover, uiPackBasePath, "Yellow/Default/button_rectangle_depth_gradient.png");
    ok &= loadUiTexture(m_texArrowPointer, uiPackBasePath, "Yellow/Default/arrow_decorative_e.png");
    ok &= loadUiTexture(m_texCheckOn, uiPackBasePath, "Yellow/Default/check_square_color_checkmark.png");
    ok &= loadUiTexture(m_texCheckOff, uiPackBasePath, "Grey/Default/check_square_grey.png");

    if (!ok) {
        std::cerr << "[WARN] [PauseMenuUI] Algunas texturas de UI Pack no se pudieron cargar; se usaran fallbacks visuales." << std::endl;
    }

    m_selectedOption = 0;
    m_isOpen = false;
    m_pendingAction = PauseAction::NONE;
    m_animTimer = 0.0f;
    m_initialized = true;

    std::cout << "[OK] [PauseMenuUI] Componente modular de Pausa inicializado exitosamente." << std::endl;
    return true;
}

void PauseMenuUI::open() {
    m_isOpen = true;
    m_pendingAction = PauseAction::NONE;
    core::Input::setCursorVisible(true);
    if (m_audio) {
        m_audio->playSound(audio::SoundEvent::UI_PAUSE);
    }
}

void PauseMenuUI::close() {
    m_isOpen = false;
}

void PauseMenuUI::update(float deltaTime) {
    if (m_isOpen) {
        m_animTimer += deltaTime;
    }
}

void PauseMenuUI::setSelectedOption(int option) {
    m_selectedOption = std::clamp(option, 0, 6);
}

bool PauseMenuUI::areTexturesLoaded() const {
    return m_texPauseModalFrame.isLoaded() &&
           m_texPauseBtnIdle.isLoaded() &&
           m_texPauseBtnHover.isLoaded() &&
           m_texArrowPointer.isLoaded() &&
           m_texCheckOn.isLoaded() &&
           m_texCheckOff.isLoaded();
}

PauseAction PauseMenuUI::consumeAction() {
    PauseAction act = m_pendingAction;
    m_pendingAction = PauseAction::NONE;
    return act;
}

void PauseMenuUI::handleInput(float deltaTime,
                              lighting::LightingController* lightingCtrl,
                              lighting::LightManager* lightMgr) {
    handleInput(deltaTime, lightingCtrl, lightMgr, 0.0f, 0.0f);
}

void PauseMenuUI::handleInput(float deltaTime,
                              lighting::LightingController* lightingCtrl,
                              lighting::LightManager* lightMgr,
                              float screenWidth,
                              float screenHeight) {
    if (!m_isOpen || !m_initialized) {
        return;
    }

    m_animTimer += deltaTime;
    core::Input::setCursorVisible(true);

    // Tecla ESC para cerrar pausa y continuar la carrera
    if (core::Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
        if (m_audio) {
            m_audio->playSound(audio::SoundEvent::UI_BACK);
        }
        m_pendingAction = PauseAction::RESUME;
        close();
        return;
    }

    const int prevOption = m_selectedOption;

    // 1. Navegación por teclado vertical (W / S / Flechas Arriba/Abajo) con wrap-around cíclico
    if (core::Input::isKeyJustPressed(GLFW_KEY_UP) || core::Input::isKeyJustPressed(GLFW_KEY_W)) {
        m_selectedOption = (m_selectedOption - 1 + 7) % 7;
    }
    if (core::Input::isKeyJustPressed(GLFW_KEY_DOWN) || core::Input::isKeyJustPressed(GLFW_KEY_S)) {
        m_selectedOption = (m_selectedOption + 1) % 7;
    }

    // Atajos numéricos directos (1 a 7)
    if (core::Input::isKeyJustPressed(GLFW_KEY_1) || core::Input::isKeyJustPressed(GLFW_KEY_KP_1)) m_selectedOption = 0;
    else if (core::Input::isKeyJustPressed(GLFW_KEY_2) || core::Input::isKeyJustPressed(GLFW_KEY_KP_2)) m_selectedOption = 1;
    else if (core::Input::isKeyJustPressed(GLFW_KEY_3) || core::Input::isKeyJustPressed(GLFW_KEY_KP_3)) m_selectedOption = 2;
    else if (core::Input::isKeyJustPressed(GLFW_KEY_4) || core::Input::isKeyJustPressed(GLFW_KEY_KP_4)) m_selectedOption = 3;
    else if (core::Input::isKeyJustPressed(GLFW_KEY_5) || core::Input::isKeyJustPressed(GLFW_KEY_KP_5)) m_selectedOption = 4;
    else if (core::Input::isKeyJustPressed(GLFW_KEY_6) || core::Input::isKeyJustPressed(GLFW_KEY_KP_6)) m_selectedOption = 5;
    else if (core::Input::isKeyJustPressed(GLFW_KEY_7) || core::Input::isKeyJustPressed(GLFW_KEY_KP_7)) m_selectedOption = 6;

    bool goLeft = core::Input::isKeyJustPressed(GLFW_KEY_LEFT) || core::Input::isKeyJustPressed(GLFW_KEY_A);
    bool goRight = core::Input::isKeyJustPressed(GLFW_KEY_RIGHT) || core::Input::isKeyJustPressed(GLFW_KEY_D);
    bool activate = core::Input::isKeyJustPressed(GLFW_KEY_ENTER) ||
                    core::Input::isKeyJustPressed(GLFW_KEY_KP_ENTER) ||
                    core::Input::isKeyJustPressed(GLFW_KEY_SPACE);

    // 2. Hit-testing e interacción con ratón
    float sw = (screenWidth > 0.0f) ? screenWidth : m_screenWidth;
    float sh = (screenHeight > 0.0f) ? screenHeight : m_screenHeight;

    if (screenWidth <= 0.0f || screenHeight <= 0.0f) {
        GLFWwindow* win = glfwGetCurrentContext();
        if (win) {
            int w = 0, h = 0;
            glfwGetWindowSize(win, &w, &h);
            if (w > 0 && h > 0) {
                sw = static_cast<float>(w);
                sh = static_cast<float>(h);
            }
        }
    }

    float modalW = 680.0f;
    float modalH = 500.0f;
    float modalX = (sw - modalW) * 0.5f;
    float modalY = (sh - modalH) * 0.5f;
    float optStartY = modalY + 66.0f;
    float rowHeight = 48.0f;
    float rowW = modalW - 48.0f;
    float rowX = modalX + 24.0f;

    float mx = core::Input::getMouseX();
    float my = core::Input::getMouseY();
    bool mouseClicked = core::Input::isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT);

    for (int i = 0; i < 7; ++i) {
        float rowY = optStartY + static_cast<float>(i) * rowHeight;
        if (mx >= rowX && mx <= rowX + rowW && my >= rowY && my <= rowY + rowHeight - 6.0f) {
            m_selectedOption = i;
            if (mouseClicked) {
                activate = true;
            }
        }
    }

    if (m_selectedOption != prevOption && m_audio) {
        m_audio->playSound(audio::SoundEvent::UI_NAVIGATE);
    }

    // 3. Ejecución y despacho de acciones según la opción seleccionada
    if (activate || goLeft || goRight) {
        switch (m_selectedOption) {
            case 0: { // 1) CONTINUAR CARRERA (Resume)
                if (activate) {
                    if (m_audio) m_audio->playSound(audio::SoundEvent::UI_CONFIRM);
                    m_pendingAction = PauseAction::RESUME;
                    close();
                }
                break;
            }
            case 1: { // 2) MODELO DE ILUMINACIÓN (Flat / Lambert / Blinn-Phong)
                if (m_audio) m_audio->playSound(audio::SoundEvent::UI_SWITCH);
                if (lightMgr) {
                    if (goLeft) {
                        int prev = (static_cast<int>(lightMgr->getLightingModel()) + 2) % 3;
                        lightMgr->setLightingModel(static_cast<lighting::LightingModel>(prev));
                    } else {
                        lightMgr->cycleNextModel();
                    }
                } else if (lightingCtrl) {
                    if (goLeft) {
                        int prev = (static_cast<int>(lightingCtrl->getLightingModel()) + 2) % 3;
                        lightingCtrl->setLightingModel(static_cast<lighting::LightingModel>(prev));
                    } else {
                        int next = (static_cast<int>(lightingCtrl->getLightingModel()) + 1) % 3;
                        lightingCtrl->setLightingModel(static_cast<lighting::LightingModel>(next));
                    }
                }
                break;
            }
            case 2: { // 3) CICLO DÍA / NOCHE (Toggle)
                if (m_audio) m_audio->playSound(audio::SoundEvent::UI_SWITCH);
                if (lightingCtrl) {
                    lightingCtrl->toggleDayNight();
                }
                break;
            }
            case 3: { // 4) FAROS DEL KART (ON / OFF)
                if (m_audio) m_audio->playSound(audio::SoundEvent::UI_SWITCH);
                if (lightingCtrl) {
                    lightingCtrl->toggleHeadlights();
                } else if (lightMgr) {
                    lightMgr->toggleSpotLight();
                }
                break;
            }
            case 4: { // 5) REINICIAR CARRERA (Restart)
                if (activate) {
                    if (m_audio) m_audio->playSound(audio::SoundEvent::UI_CONFIRM);
                    m_pendingAction = PauseAction::RESTART;
                    close();
                }
                break;
            }
            case 5: { // 6) VOLVER AL MENÚ PRINCIPAL (Return to Main Menu)
                if (activate) {
                    if (m_audio) m_audio->playSound(audio::SoundEvent::UI_CONFIRM);
                    m_pendingAction = PauseAction::GO_TO_MAIN_MENU;
                    close();
                }
                break;
            }
            case 6: { // 7) SALIR AL ESCRITORIO (Quit)
                if (activate) {
                    if (m_audio) m_audio->playSound(audio::SoundEvent::UI_BACK);
                    m_pendingAction = PauseAction::QUIT_GAME;
                    close();
                }
                break;
            }
            default:
                break;
        }
    }
}

void PauseMenuUI::render(float screenWidth,
                         float screenHeight,
                         const lighting::LightingController* lightingCtrl,
                         const lighting::LightManager* lightMgr) {
    if (!m_isOpen || !m_initialized || !m_renderer) {
        return;
    }

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    m_renderer->begin(screenWidth, screenHeight);

    // 1. Oscurecimiento de fondo (alpha = 0.70f)
    m_renderer->drawRect(0.0f, 0.0f, screenWidth, screenHeight, glm::vec4(0.0f, 0.0f, 0.0f, 0.70f));

    // 2. Cuadro de diálogo modal centrado
    float modalW = 680.0f;
    float modalH = 500.0f;
    glm::vec2 modalPos = computeAnchoredPosition(UIAnchor::CENTER, modalW, modalH, screenWidth, screenHeight);
    float modalX = modalPos.x;
    float modalY = modalPos.y;

    // Fondo solido azul profundo
    m_renderer->drawRect(modalX + 6.0f, modalY + 6.0f, modalW - 12.0f, modalH - 12.0f,
                         glm::vec4(0.05f, 0.08f, 0.15f, 0.96f));

    // Marco exterior con textura Kenney mediante 9-Slice para remaches y esquinas nitidas
    if (m_texPauseModalFrame.isLoaded()) {
        m_renderer->drawNineSlice(m_texPauseModalFrame, modalX, modalY, modalW, modalH, 28.0f, 28.0f);
    }

    // Borde brillante de acento
    m_renderer->drawBorder(modalX, modalY, modalW, modalH, 2.0f, glm::vec4(0.35f, 0.65f, 0.95f, 0.85f));

    // 3. Titulo de la ventana modal
    m_renderer->drawTextCentered(modalX, modalY + 18.0f, modalW, 30.0f,
                                 "PAUSA", 2.8f, glm::vec4(1.0f, 0.88f, 0.20f, 1.0f));

    // Linea divisoria estetica
    m_renderer->drawRect(modalX + 30.0f, modalY + 54.0f, modalW - 60.0f, 2.0f, glm::vec4(0.3f, 0.5f, 0.8f, 0.6f));

    // 4. Filas de Opciones de Pausa / Configuracion (7 opciones)
    float optStartY = modalY + 66.0f;
    float rowHeight = 48.0f;
    float rowW = modalW - 48.0f;
    float rowX = modalX + 24.0f;

    for (int i = 0; i < 7; ++i) {
        float rowY = optStartY + static_cast<float>(i) * rowHeight;
        bool isSelected = (m_selectedOption == i);

        // Boton 9-slice con Kenney UI Pack
        const auto& btnTex = isSelected ? m_texPauseBtnHover : m_texPauseBtnIdle;
        if (btnTex.isLoaded()) {
            m_renderer->drawNineSlice(btnTex, rowX, rowY, rowW, rowHeight - 6.0f, 16.0f, 16.0f);
        } else {
            m_renderer->drawRect(rowX, rowY, rowW, rowHeight - 6.0f,
                                 isSelected ? glm::vec4(0.2f, 0.4f, 0.8f, 0.9f)
                                            : glm::vec4(0.1f, 0.15f, 0.25f, 0.9f));
        }

        // Flecha animada pulsante al seleccionar
        if (isSelected && m_texArrowPointer.isLoaded()) {
            float arrowBounce = std::sin(m_animTimer * 6.0f) * 4.0f;
            m_renderer->drawSprite(m_texArrowPointer, rowX - 30.0f + arrowBounce,
                                   rowY + (rowHeight - 6.0f - 24.0f) * 0.5f, 24.0f, 24.0f);
        }

        glm::vec4 textColor = isSelected ? glm::vec4(0.12f, 0.12f, 0.14f, 1.0f)
                                         : glm::vec4(0.92f, 0.95f, 1.0f, 1.0f);

        switch (i) {
            case 0: { // 1) CONTINUAR CARRERA (Resume)
                m_renderer->drawTextCentered(rowX, rowY, rowW, rowHeight - 6.0f,
                                             "CONTINUAR CARRERA", 2.0f, textColor);
                break;
            }
            case 1: { // 2) MODELO DE ILUMINACION (Flat / Lambert / Blinn-Phong)
                m_renderer->drawText(rowX + 20.0f, rowY + 11.0f, "MODELO DE ILUMINACION", 1.8f, textColor);

                std::string modelStrShort = "BLINN-PHONG";
                if (lightMgr) {
                    switch (lightMgr->getLightingModel()) {
                        case lighting::LightingModel::FLAT: modelStrShort = "FLAT"; break;
                        case lighting::LightingModel::LAMBERT: modelStrShort = "LAMBERT"; break;
                        case lighting::LightingModel::BLINN_PHONG: modelStrShort = "BLINN-PHONG"; break;
                    }
                } else if (lightingCtrl) {
                    switch (lightingCtrl->getLightingModel()) {
                        case lighting::LightingModel::FLAT: modelStrShort = "FLAT"; break;
                        case lighting::LightingModel::LAMBERT: modelStrShort = "LAMBERT"; break;
                        case lighting::LightingModel::BLINN_PHONG: modelStrShort = "BLINN-PHONG"; break;
                    }
                }
                std::string modelStr = "< " + modelStrShort + " >";
                m_renderer->drawText(rowX + rowW - 250.0f, rowY + 11.0f, modelStr, 1.8f,
                                     isSelected ? glm::vec4(0.10f, 0.35f, 0.15f, 1.0f)
                                                : glm::vec4(0.35f, 1.0f, 0.65f, 1.0f));
                break;
            }
            case 2: { // 3) CICLO DIA / NOCHE (Toggle)
                m_renderer->drawText(rowX + 20.0f, rowY + 11.0f, "CICLO DIA / NOCHE", 1.8f, textColor);

                bool isNight = lightingCtrl ? lightingCtrl->isNight() : false;
                const auto& chkTex = isNight ? m_texCheckOn : m_texCheckOff;
                float chkSize = 24.0f;
                float chkX = rowX + rowW - 170.0f;
                float chkY = rowY + (rowHeight - 6.0f - chkSize) * 0.5f;

                if (chkTex.isLoaded()) {
                    m_renderer->drawSprite(chkTex, chkX, chkY, chkSize, chkSize);
                }
                m_renderer->drawText(chkX + chkSize + 10.0f, rowY + 11.0f, isNight ? "NOCHE" : "DIA", 1.8f,
                                     isSelected ? glm::vec4(0.12f, 0.12f, 0.14f, 1.0f)
                                                : (isNight ? glm::vec4(0.4f, 0.8f, 1.0f, 1.0f)
                                                           : glm::vec4(1.0f, 0.9f, 0.4f, 1.0f)));
                break;
            }
            case 3: { // 4) FAROS DEL KART (ON / OFF)
                m_renderer->drawText(rowX + 20.0f, rowY + 11.0f, "FAROS DEL KART (SPOT)", 1.8f, textColor);

                bool areHeadlightsOn = true;
                if (lightingCtrl) {
                    areHeadlightsOn = lightingCtrl->areHeadlightsOn();
                } else if (lightMgr) {
                    areHeadlightsOn = lightMgr->getSpotLight().enabled;
                }
                const auto& chkTex = areHeadlightsOn ? m_texCheckOn : m_texCheckOff;
                float chkSize = 24.0f;
                float chkX = rowX + rowW - 170.0f;
                float chkY = rowY + (rowHeight - 6.0f - chkSize) * 0.5f;

                if (chkTex.isLoaded()) {
                    m_renderer->drawSprite(chkTex, chkX, chkY, chkSize, chkSize);
                }
                m_renderer->drawText(chkX + chkSize + 10.0f, rowY + 11.0f, areHeadlightsOn ? "ON" : "OFF", 1.8f,
                                     isSelected ? glm::vec4(0.12f, 0.12f, 0.14f, 1.0f)
                                                : (areHeadlightsOn ? glm::vec4(0.35f, 1.0f, 0.65f, 1.0f)
                                                                   : glm::vec4(0.7f, 0.7f, 0.7f, 0.8f)));
                break;
            }
            case 4: { // 5) REINICIAR CARRERA (Restart)
                m_renderer->drawTextCentered(rowX, rowY, rowW, rowHeight - 6.0f,
                                             "REINICIAR CARRERA", 2.0f, textColor);
                break;
            }
            case 5: { // 6) VOLVER AL MENU PRINCIPAL (Return to Main Menu)
                m_renderer->drawTextCentered(rowX, rowY, rowW, rowHeight - 6.0f,
                                             "VOLVER AL MENU PRINCIPAL", 2.0f, textColor);
                break;
            }
            case 6: { // 7) SALIR AL ESCRITORIO (Quit)
                m_renderer->drawTextCentered(rowX, rowY, rowW, rowHeight - 6.0f,
                                             "SALIR AL ESCRITORIO", 2.0f, textColor);
                break;
            }
            default:
                break;
        }
    }

    // 5. Pie de pagina con atajos de navegacion dentro del modal
    m_renderer->drawTextCentered(modalX, modalY + modalH - 32.0f, modalW, 20.0f,
                                 "[W/S/FLECHAS] Navegar | [ENTER/A/D] Modificar | [ESC] Continuar",
                                 1.6f, glm::vec4(0.80f, 0.88f, 0.98f, 0.85f));

    m_renderer->end();
}

} // namespace ui
