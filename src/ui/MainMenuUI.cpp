#include "ui/MainMenuUI.hpp"
#include "core/GameContext.hpp"
#include "core/Input.hpp"
#include "core/Window.hpp"
#include "core/SceneManager.hpp"
#include "lighting/LightManager.hpp"
#include "lighting/LightingController.hpp"

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

    // Carga de texturas de alta resolución (Double 2x) del Kenney UI Pack estándar con fallback a Default
    auto loadTexture = [](renderer::Texture& tex, const std::string& subpath) -> bool {
        std::string doublePath = "assets/kenney_ui-pack/PNG/" + subpath;
        // Intento 1: Carga 2x Double
        std::string p2x = doublePath;
        size_t idx = p2x.find("/Default/");
        if (idx != std::string::npos) {
            p2x.replace(idx, 9, "/Double/");
        }
        if (tex.loadFromFile(p2x, false)) {
            return true;
        }
        // Intento 2: Fallback 1x Default
        return tex.loadFromFile("assets/kenney_ui-pack/PNG/" + subpath, false);
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

    if (!m_inSettings) {
        // -------------------------------------------------------------
        // Navegacion Teclado en Menu Principal
        // -------------------------------------------------------------
        if (core::Input::isKeyJustPressed(GLFW_KEY_UP) || core::Input::isKeyJustPressed(GLFW_KEY_W)) {
            int opt = static_cast<int>(m_selectedOption);
            opt = (opt - 1 + 3) % 3;
            m_selectedOption = static_cast<MainMenuOption>(opt);
        }
        if (core::Input::isKeyJustPressed(GLFW_KEY_DOWN) || core::Input::isKeyJustPressed(GLFW_KEY_S)) {
            int opt = static_cast<int>(m_selectedOption);
            opt = (opt + 1) % 3;
            m_selectedOption = static_cast<MainMenuOption>(opt);
        }

        // Atajos directos por teclado numérico o fila superior
        if (core::Input::isKeyJustPressed(GLFW_KEY_1) || core::Input::isKeyJustPressed(GLFW_KEY_KP_1)) {
            m_selectedOption = MainMenuOption::PLAY;
        } else if (core::Input::isKeyJustPressed(GLFW_KEY_2) || core::Input::isKeyJustPressed(GLFW_KEY_KP_2)) {
            m_selectedOption = MainMenuOption::SETTINGS;
        } else if (core::Input::isKeyJustPressed(GLFW_KEY_3) || core::Input::isKeyJustPressed(GLFW_KEY_KP_3)) {
            m_selectedOption = MainMenuOption::QUIT;
        }

        // Activacion de opcion por ENTER o ESPACIO
        bool activate = core::Input::isKeyJustPressed(GLFW_KEY_ENTER) ||
                        core::Input::isKeyJustPressed(GLFW_KEY_KP_ENTER) ||
                        core::Input::isKeyJustPressed(GLFW_KEY_SPACE);

        // -------------------------------------------------------------
        // Interaccion con Mouse en Botones Principales
        // -------------------------------------------------------------
        float btnW = 300.0f;
        float btnH = 56.0f;
        float btnX = (sw - btnW) * 0.5f;
        float startY = sh * 0.44f;
        float spacing = 72.0f;

        for (int i = 0; i < 3; ++i) {
            float y = startY + static_cast<float>(i) * spacing;
            if (mx >= btnX && mx <= btnX + btnW && my >= y && my <= y + btnH) {
                m_selectedOption = static_cast<MainMenuOption>(i);
                if (mouseClicked) {
                    activate = true;
                }
            }
        }

        if (activate) {
            if (m_selectedOption == MainMenuOption::PLAY) {
                std::cout << "[MENU] Opcion JUGAR activada -> Cambiando a VEHICLE_SELECT..." << std::endl;
                if (m_ctx && m_ctx->sceneManager) {
                    m_ctx->selectionCategory = core::SelectionCategory::DRIVER;
                    m_ctx->sceneManager->changeScene(core::GameState::VEHICLE_SELECT);
                }
            } else if (m_selectedOption == MainMenuOption::SETTINGS) {
                std::cout << "[MENU] Opcion CONFIGURACION activada -> Abriendo panel modal..." << std::endl;
                openSettings();
            } else if (m_selectedOption == MainMenuOption::QUIT) {
                std::cout << "[MENU] Opcion SALIR activada -> Cerrando juego..." << std::endl;
                if (m_ctx && m_ctx->window) {
                    glfwSetWindowShouldClose(m_ctx->window->getNativeWindow(), GLFW_TRUE);
                } else {
                    GLFWwindow* win = glfwGetCurrentContext();
                    if (win) glfwSetWindowShouldClose(win, GLFW_TRUE);
                }
            }
        }

        if (core::Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            std::cout << "[MENU] ESC presionado en Menu -> Saliendo del juego..." << std::endl;
            if (m_ctx && m_ctx->window) {
                glfwSetWindowShouldClose(m_ctx->window->getNativeWindow(), GLFW_TRUE);
            } else {
                GLFWwindow* win = glfwGetCurrentContext();
                if (win) glfwSetWindowShouldClose(win, GLFW_TRUE);
            }
        }
    } else {
        // -------------------------------------------------------------
        // Navegacion dentro del Modal de Configuracion
        // -------------------------------------------------------------
        if (core::Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            closeSettings();
            return;
        }

        // Navegacion vertical entre 6 opciones (0 a 5)
        if (core::Input::isKeyJustPressed(GLFW_KEY_UP) || core::Input::isKeyJustPressed(GLFW_KEY_W)) {
            int opt = static_cast<int>(m_selectedSettingsOption);
            opt = (opt - 1 + 6) % 6;
            m_selectedSettingsOption = static_cast<SettingsOption>(opt);
        }
        if (core::Input::isKeyJustPressed(GLFW_KEY_DOWN) || core::Input::isKeyJustPressed(GLFW_KEY_S)) {
            int opt = static_cast<int>(m_selectedSettingsOption);
            opt = (opt + 1) % 6;
            m_selectedSettingsOption = static_cast<SettingsOption>(opt);
        }

        bool goLeft = core::Input::isKeyJustPressed(GLFW_KEY_LEFT) || core::Input::isKeyJustPressed(GLFW_KEY_A);
        bool goRight = core::Input::isKeyJustPressed(GLFW_KEY_RIGHT) || core::Input::isKeyJustPressed(GLFW_KEY_D);
        bool activate = core::Input::isKeyJustPressed(GLFW_KEY_ENTER) ||
                        core::Input::isKeyJustPressed(GLFW_KEY_KP_ENTER) ||
                        core::Input::isKeyJustPressed(GLFW_KEY_SPACE);

        // Interaccion con Mouse en filas de configuracion
        float modalW = 660.0f;
        float modalH = 460.0f;
        float modalX = (sw - modalW) * 0.5f;
        float modalY = (sh - modalH) * 0.5f;
        float optStartY = modalY + 68.0f;
        float rowHeight = 52.0f;

        for (int i = 0; i < 6; ++i) {
            float rowY = optStartY + static_cast<float>(i) * rowHeight;
            if (mx >= modalX + 20.0f && mx <= modalX + modalW - 20.0f && my >= rowY && my <= rowY + rowHeight - 6.0f) {
                m_selectedSettingsOption = static_cast<SettingsOption>(i);
                if (mouseClicked) {
                    activate = true;
                }
            }
        }

        // Aplicacion de modificaciones
        if (goLeft || goRight || activate) {
            switch (m_selectedSettingsOption) {
                case SettingsOption::SHADING_MODEL: {
                    if (m_ctx && m_ctx->lightManager) {
                        if (goLeft) {
                            int prev = (static_cast<int>(m_ctx->lightManager->getLightingModel()) + 2) % 3;
                            m_ctx->lightManager->setLightingModel(static_cast<lighting::LightingModel>(prev));
                        } else {
                            m_ctx->lightManager->cycleNextModel();
                        }
                    }
                    break;
                }
                case SettingsOption::DAY_NIGHT: {
                    if (m_ctx && m_ctx->lightingController) {
                        m_ctx->lightingController->toggleDayNight();
                    }
                    break;
                }
                case SettingsOption::HEADLIGHTS: {
                    if (m_ctx && m_ctx->lightingController) {
                        m_ctx->lightingController->toggleHeadlights();
                    }
                    break;
                }
                case SettingsOption::TRACK_LIGHTS: {
                    if (m_ctx && m_ctx->lightManager) {
                        m_ctx->lightManager->togglePointLights();
                    }
                    break;
                }
                case SettingsOption::INITIAL_CAMERA: {
                    if (m_ctx) {
                        m_ctx->cameraMode = (m_ctx->cameraMode == core::CameraMode::CHASE)
                                            ? core::CameraMode::FPS
                                            : core::CameraMode::CHASE;
                    }
                    break;
                }
                case SettingsOption::BACK: {
                    if (activate || goLeft || goRight) {
                        closeSettings();
                    }
                    break;
                }
            }
        }
    }
}

void MainMenuUI::render(float screenWidth, float screenHeight) {
    if (!m_initialized || !m_renderer) return;

    m_renderer->begin(screenWidth, screenHeight);

    // Dibuja el menu principal
    renderMainMenu(screenWidth, screenHeight);

    // Si el modal de configuracion esta activo, dibuja el panel flotante encima
    if (m_inSettings) {
        renderSettingsModal(screenWidth, screenHeight);
    }

    m_renderer->end();
}

void MainMenuUI::renderMainMenu(float screenWidth, float screenHeight) {
    // -------------------------------------------------------------
    // 1. Banner de Titulo Principal
    // -------------------------------------------------------------
    float titleY = screenHeight * 0.16f;
    // Titulo principal con sombra proyectada para maxima legibilidad sobre la escena 3D
    m_renderer->drawTextCenteredWithShadow(0.0f, titleY, screenWidth, 40.0f,
                                           "MARIO KART - CGI (UCV)", 4.0f,
                                           glm::vec4(1.0f, 0.86f, 0.15f, 1.0f),
                                           glm::vec4(0.0f, 0.0f, 0.0f, 0.85f), 2.0f);

    // Subtitulo institucional
    m_renderer->drawTextCenteredWithShadow(0.0f, titleY + 46.0f, screenWidth, 20.0f,
                                           "PROTOTIPO DE CARRERAS - OPENGL 3.3 CORE", 2.0f,
                                           glm::vec4(0.88f, 0.92f, 1.0f, 0.95f),
                                           glm::vec4(0.0f, 0.0f, 0.0f, 0.80f), 1.5f);

    // -------------------------------------------------------------
    // 2. Botones Principales (Jugar, Configuracion, Salir)
    // -------------------------------------------------------------
    float btnW = 300.0f;
    float btnH = 56.0f;
    float btnX = (screenWidth - btnW) * 0.5f;
    float startY = screenHeight * 0.44f;
    float spacing = 72.0f;

    const char* labels[3] = { "JUGAR", "CONFIGURACION", "SALIR" };

    for (int i = 0; i < 3; ++i) {
        float y = startY + static_cast<float>(i) * spacing;
        bool isSelected = (!m_inSettings && static_cast<int>(m_selectedOption) == i);

        // Seleccion de textura: Hover (amarillo con relieve) si esta seleccionado, Idle (azul plano) en reposo
        const renderer::Texture& tex = isSelected ? m_btnHover : m_btnIdle;
        // 9-Slice para preservar esquinas redondeadas y biseles nativos sin deformacion
        m_renderer->drawNineSlice(tex, btnX, y, btnW, btnH, 20.0f, 20.0f);

        // El texto sobre boton hover amarillo se dibuja en gris oscuro; sobre boton azul en blanco con sombra
        if (isSelected) {
            m_renderer->drawTextCentered(btnX, y, btnW, btnH, labels[i], 2.2f, glm::vec4(0.12f, 0.12f, 0.14f, 1.0f));
        } else {
            m_renderer->drawTextCenteredWithShadow(btnX, y, btnW, btnH, labels[i], 2.2f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }

        // Puntero de flecha decorativa animado a la izquierda del boton seleccionado
        if (isSelected) {
            float arrowW = 32.0f;
            float arrowH = 32.0f;
            float arrowBounce = std::sin(m_animTimer * 6.0f) * 6.0f;
            float arrowX = btnX - arrowW - 14.0f + arrowBounce;
            float arrowY = y + (btnH - arrowH) * 0.5f;
            m_renderer->drawSprite(m_arrowPointer, arrowX, arrowY, arrowW, arrowH);
        }
    }

    // -------------------------------------------------------------
    // 3. Guia de Controles Inferior
    // -------------------------------------------------------------
    m_renderer->drawTextCenteredWithShadow(0.0f, screenHeight - 44.0f, screenWidth, 20.0f,
                                           "[W/S/FLECHAS] Navegar | [ENTER/ESPACIO] Seleccionar | [ESC] Salir",
                                           1.8f, glm::vec4(0.90f, 0.92f, 0.98f, 0.85f),
                                           glm::vec4(0.0f, 0.0f, 0.0f, 0.85f), 1.5f);
}

void MainMenuUI::renderSettingsModal(float screenWidth, float screenHeight) {
    // 1. Telon de fondo translucido oscuro para oscurecer la escena 3D y centrar atencion
    m_renderer->drawRect(0.0f, 0.0f, screenWidth, screenHeight, glm::vec4(0.0f, 0.0f, 0.0f, 0.75f));

    // 2. Cuadro de dialogo modal centrado
    float modalW = 680.0f;
    float modalH = 470.0f;
    float modalX = (screenWidth - modalW) * 0.5f;
    float modalY = (screenHeight - modalH) * 0.5f;

    // Fondo solido base oscuro
    m_renderer->drawRect(modalX + 4.0f, modalY + 4.0f, modalW - 8.0f, modalH - 8.0f, glm::vec4(0.04f, 0.06f, 0.10f, 0.98f));
    // Marco exterior con textura Kenney mediante 9-Slice para mantener las esquinas y remaches nitidos
    m_renderer->drawNineSlice(m_panelFrame, modalX, modalY, modalW, modalH, 24.0f, 24.0f);
    // Borde brillante de acento exterior
    m_renderer->drawBorder(modalX, modalY, modalW, modalH, 2.0f, glm::vec4(0.35f, 0.65f, 0.95f, 0.85f));

    // Placa interior oscura profunda tipo pizarra/carbon para maximo contraste contra el texto
    float innerMargin = 12.0f;
    float innerX = modalX + innerMargin;
    float innerY = modalY + innerMargin;
    float innerW = modalW - innerMargin * 2.0f;
    float innerH = modalH - innerMargin * 2.0f;
    m_renderer->drawRect(innerX, innerY, innerW, innerH, glm::vec4(0.05f, 0.07f, 0.12f, 0.96f));
    m_renderer->drawBorder(innerX, innerY, innerW, innerH, 1.5f, glm::vec4(0.18f, 0.28f, 0.45f, 0.70f));

    // 3. Titulo de la ventana modal: Amarillo oro #FFDF32 con sombra proyectada
    m_renderer->drawTextCenteredWithShadow(modalX, modalY + 18.0f, modalW, 30.0f,
                                           "CONFIGURACION DEL SISTEMA", 2.6f,
                                           glm::vec4(1.0f, 0.88f, 0.20f, 1.0f),
                                           glm::vec4(0.0f, 0.0f, 0.0f, 0.90f), 1.5f);

    // Linea divisoria estetica
    m_renderer->drawRect(modalX + 30.0f, modalY + 54.0f, modalW - 60.0f, 2.0f, glm::vec4(0.20f, 0.35f, 0.60f, 0.75f));

    // 4. Filas de Opciones de Configuracion
    float optStartY = modalY + 68.0f;
    float rowHeight = 52.0f;

    for (int i = 0; i < 6; ++i) {
        float rowY = optStartY + static_cast<float>(i) * rowHeight;
        bool isSelected = (m_selectedSettingsOption == static_cast<SettingsOption>(i));

        // Resaltado de fondo de fila seleccionada: Azul medianoche translucido con borde dorado #FFD700
        if (isSelected) {
            m_renderer->drawRect(modalX + 18.0f, rowY - 2.0f, modalW - 36.0f, rowHeight - 6.0f,
                                 glm::vec4(0.10f, 0.20f, 0.42f, 0.75f));
            m_renderer->drawBorder(modalX + 18.0f, rowY - 2.0f, modalW - 36.0f, rowHeight - 6.0f,
                                   1.5f, glm::vec4(1.0f, 0.843f, 0.0f, 1.0f));

            // Flecha indicadora animada
            float arrowBounce = std::sin(m_animTimer * 6.0f) * 4.0f;
            m_renderer->drawSprite(m_arrowPointer, modalX + 24.0f + arrowBounce, rowY + 10.0f, 22.0f, 22.0f);
        }

        // Etiquetas: Blanco brillante puro #FFFFFF con sombra si es normal; Amarillo brillante #FFF266 si es seleccionada
        glm::vec4 labelColor = isSelected ? glm::vec4(1.0f, 0.95f, 0.40f, 1.0f) : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

        switch (static_cast<SettingsOption>(i)) {
            case SettingsOption::SHADING_MODEL: {
                m_renderer->drawTextWithShadow(modalX + 54.0f, rowY + 12.0f, "MODELO DE ILUMINACION", 1.9f, labelColor);

                const char* modelName = (m_ctx && m_ctx->lightManager) ? m_ctx->lightManager->getCurrentModelName() : "BLINN-PHONG";
                std::string valStr = std::string("< ") + modelName + " >";
                for (auto& c : valStr) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

                // Valores de modelo: Cian brillante #00E5FF con sombra
                glm::vec4 valColor = isSelected ? glm::vec4(0.0f, 0.90f, 1.0f, 1.0f) : glm::vec4(0.65f, 0.90f, 1.0f, 0.95f);
                m_renderer->drawTextWithShadow(modalX + modalW - 270.0f, rowY + 12.0f, valStr, 1.8f, valColor);
                break;
            }
            case SettingsOption::DAY_NIGHT: {
                m_renderer->drawTextWithShadow(modalX + 54.0f, rowY + 12.0f, "CICLO DIA / NOCHE", 1.9f, labelColor);

                bool isNight = (m_ctx && m_ctx->lightingController) ? m_ctx->lightingController->isNight() : false;
                const renderer::Texture& chkTex = isNight ? m_checkOn : m_checkOff;
                float chkSize = 28.0f;
                float chkX = modalX + modalW - 210.0f;
                float chkY = rowY + (rowHeight - 6.0f - chkSize) * 0.5f;

                m_renderer->drawSprite(chkTex, chkX, chkY, chkSize, chkSize);
                // Estados de casillas: (DIA) Verde esmeralda brillante #32F5A3, (NOCHE) Naranja/Ambar contrastado
                glm::vec4 stateColor = (!isNight) ? glm::vec4(0.20f, 0.96f, 0.64f, 1.0f) : glm::vec4(1.0f, 0.45f, 0.25f, 1.0f);
                m_renderer->drawTextWithShadow(chkX + chkSize + 12.0f, rowY + 12.0f, isNight ? "NOCHE" : "DIA", 1.8f, stateColor);
                break;
            }
            case SettingsOption::HEADLIGHTS: {
                m_renderer->drawTextWithShadow(modalX + 54.0f, rowY + 12.0f, "FAROS DEL KART (SPOT)", 1.9f, labelColor);

                bool headlights = (m_ctx && m_ctx->lightingController) ? m_ctx->lightingController->areHeadlightsOn() : true;
                const renderer::Texture& chkTex = headlights ? m_checkOn : m_checkOff;
                float chkSize = 28.0f;
                float chkX = modalX + modalW - 210.0f;
                float chkY = rowY + (rowHeight - 6.0f - chkSize) * 0.5f;

                m_renderer->drawSprite(chkTex, chkX, chkY, chkSize, chkSize);
                // Estados de casillas: (ON) Verde esmeralda brillante #32F5A3, (OFF) Naranja/Ambar contrastado
                glm::vec4 stateColor = headlights ? glm::vec4(0.20f, 0.96f, 0.64f, 1.0f) : glm::vec4(1.0f, 0.45f, 0.25f, 1.0f);
                m_renderer->drawTextWithShadow(chkX + chkSize + 12.0f, rowY + 12.0f, headlights ? "ON" : "OFF", 1.8f, stateColor);
                break;
            }
            case SettingsOption::TRACK_LIGHTS: {
                m_renderer->drawTextWithShadow(modalX + 54.0f, rowY + 12.0f, "FAROLAS DE PISTA (POINTS)", 1.9f, labelColor);

                bool pointsOn = (m_ctx && m_ctx->lightManager && !m_ctx->lightManager->getPointLights().empty())
                                ? m_ctx->lightManager->getPointLights()[0].enabled : true;
                const renderer::Texture& chkTex = pointsOn ? m_checkOn : m_checkOff;
                float chkSize = 28.0f;
                float chkX = modalX + modalW - 210.0f;
                float chkY = rowY + (rowHeight - 6.0f - chkSize) * 0.5f;

                m_renderer->drawSprite(chkTex, chkX, chkY, chkSize, chkSize);
                // Estados de casillas: (ON) Verde esmeralda brillante #32F5A3, (OFF) Naranja/Ambar contrastado
                glm::vec4 stateColor = pointsOn ? glm::vec4(0.20f, 0.96f, 0.64f, 1.0f) : glm::vec4(1.0f, 0.45f, 0.25f, 1.0f);
                m_renderer->drawTextWithShadow(chkX + chkSize + 12.0f, rowY + 12.0f, pointsOn ? "ON" : "OFF", 1.8f, stateColor);
                break;
            }
            case SettingsOption::INITIAL_CAMERA: {
                m_renderer->drawTextWithShadow(modalX + 54.0f, rowY + 12.0f, "MODO DE CAMARA", 1.9f, labelColor);

                bool isChase = (m_ctx && m_ctx->cameraMode == core::CameraMode::CHASE);
                std::string camStr = isChase ? "< CHASE (3RA) >" : "< FPS LIBRE >";

                // Valores de camara: Cian brillante #00E5FF con sombra
                glm::vec4 camColor = isSelected ? glm::vec4(0.0f, 0.90f, 1.0f, 1.0f) : glm::vec4(0.65f, 0.90f, 1.0f, 0.95f);
                m_renderer->drawTextWithShadow(modalX + modalW - 270.0f, rowY + 12.0f, camStr, 1.8f, camColor);
                break;
            }
            case SettingsOption::BACK: {
                float bW = 280.0f;
                float bH = 38.0f;
                float bX = modalX + (modalW - bW) * 0.5f;
                float bY = rowY + 3.0f;

                const renderer::Texture& bTex = isSelected ? m_btnHover : m_btnIdle;
                m_renderer->drawSprite(bTex, bX, bY, bW, bH);

                glm::vec4 bTextColor = isSelected ? glm::vec4(0.12f, 0.12f, 0.14f, 1.0f) : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                if (isSelected) {
                    m_renderer->drawTextCentered(bX, bY, bW, bH, "VOLVER AL MENU", 1.8f, bTextColor);
                } else {
                    m_renderer->drawTextCenteredWithShadow(bX, bY, bW, bH, "VOLVER AL MENU", 1.8f, bTextColor);
                }
                break;
            }
        }
    }

    // 5. Pie de pagina con atajos de navegacion dentro del modal
    m_renderer->drawTextCenteredWithShadow(modalX, modalY + modalH - 34.0f, modalW, 20.0f,
                                           "[W/S/FLECHAS] Navegar | [ENTER/A/D] Modificar | [ESC] Volver",
                                           1.6f, glm::vec4(0.85f, 0.90f, 1.0f, 0.90f));
}

} // namespace ui
