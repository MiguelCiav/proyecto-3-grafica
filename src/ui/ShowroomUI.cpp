#include "ui/ShowroomUI.hpp"
#include "core/GameContext.hpp"
#include "core/Input.hpp"
#include "core/Window.hpp"
#include "core/SceneManager.hpp"
#include "world/TrackEnvironmentRegistry.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iomanip>

namespace ui {

ShowroomUI::ShowroomUI() = default;

ShowroomUI::~ShowroomUI() {
    destroy();
}

ShowroomUI::ShowroomUI(ShowroomUI&& other) noexcept
    : m_ctx(other.m_ctx)
    , m_renderer(other.m_renderer)
    , m_ownedRenderer(std::move(other.m_ownedRenderer))
    , m_tabIdle(std::move(other.m_tabIdle))
    , m_tabActive(std::move(other.m_tabActive))
    , m_tabHover(std::move(other.m_tabHover))
    , m_panelFrame(std::move(other.m_panelFrame))
    , m_panelInner(std::move(other.m_panelInner))
    , m_arrowLeft(std::move(other.m_arrowLeft))
    , m_arrowRight(std::move(other.m_arrowRight))
    , m_arrowIndicator(std::move(other.m_arrowIndicator))
    , m_star(std::move(other.m_star))
    , m_starEmpty(std::move(other.m_starEmpty))
    , m_btnStart(std::move(other.m_btnStart))
    , m_btnStartHover(std::move(other.m_btnStartHover))
    , m_btnBack(std::move(other.m_btnBack))
    , m_btnBackHover(std::move(other.m_btnBackHover))
    , m_barBg(std::move(other.m_barBg))
    , m_barFill(std::move(other.m_barFill))
    , m_barFillGreen(std::move(other.m_barFillGreen))
    , m_animTimer(other.m_animTimer)
    , m_initialized(other.m_initialized)
{
    other.m_ctx = nullptr;
    other.m_renderer = nullptr;
    other.m_initialized = false;
}

ShowroomUI& ShowroomUI::operator=(ShowroomUI&& other) noexcept {
    if (this != &other) {
        destroy();

        m_ctx = other.m_ctx;
        m_renderer = other.m_renderer;
        m_ownedRenderer = std::move(other.m_ownedRenderer);
        m_tabIdle = std::move(other.m_tabIdle);
        m_tabActive = std::move(other.m_tabActive);
        m_tabHover = std::move(other.m_tabHover);
        m_panelFrame = std::move(other.m_panelFrame);
        m_panelInner = std::move(other.m_panelInner);
        m_arrowLeft = std::move(other.m_arrowLeft);
        m_arrowRight = std::move(other.m_arrowRight);
        m_arrowIndicator = std::move(other.m_arrowIndicator);
        m_star = std::move(other.m_star);
        m_starEmpty = std::move(other.m_starEmpty);
        m_btnStart = std::move(other.m_btnStart);
        m_btnStartHover = std::move(other.m_btnStartHover);
        m_btnBack = std::move(other.m_btnBack);
        m_btnBackHover = std::move(other.m_btnBackHover);
        m_barBg = std::move(other.m_barBg);
        m_barFill = std::move(other.m_barFill);
        m_barFillGreen = std::move(other.m_barFillGreen);
        m_animTimer = other.m_animTimer;
        m_initialized = other.m_initialized;

        other.m_ctx = nullptr;
        other.m_renderer = nullptr;
        other.m_initialized = false;
    }
    return *this;
}

bool ShowroomUI::init(core::GameContext* ctx, UIRenderer* externalRenderer) {
    m_ctx = ctx;

    if (externalRenderer) {
        m_renderer = externalRenderer;
    } else {
        m_ownedRenderer = std::make_unique<UIRenderer>();
        if (!m_ownedRenderer->init("shaders/ui.vert", "shaders/ui.frag")) {
            std::cerr << "[ERROR] [ShowroomUI] Fallo al inicializar renderizador UIRenderer interno." << std::endl;
            return false;
        }
        m_renderer = m_ownedRenderer.get();
    }

    if (!m_renderer) {
        std::cerr << "[ERROR] [ShowroomUI] Puntero UIRenderer nulo en init()." << std::endl;
        return false;
    }

    // Helper para cargar texturas de alta resolución (Double) con fallback a (Default)
    auto loadTexture = [](renderer::Texture& tex, const std::string& subpath) -> bool {
        std::string doublePath = "assets/kenney_ui-pack/PNG/" + subpath;
        std::string p2x = doublePath;
        size_t idx = p2x.find("/Default/");
        if (idx != std::string::npos) {
            p2x.replace(idx, 9, "/Double/");
        }
        if (tex.loadFromFile(p2x, false)) {
            return true;
        }
        return tex.loadFromFile("assets/kenney_ui-pack/PNG/" + subpath, false);
    };

    bool ok = true;
    ok &= loadTexture(m_tabIdle, "Blue/Default/button_rectangle_depth_flat.png");
    ok &= loadTexture(m_tabActive, "Yellow/Default/button_rectangle_depth_gradient.png");
    ok &= loadTexture(m_tabHover, "Yellow/Default/button_rectangle_depth_flat.png");
    ok &= loadTexture(m_panelFrame, "Blue/Default/button_square_border.png");
    ok &= loadTexture(m_panelInner, "Grey/Default/button_square_depth_flat.png");
    ok &= loadTexture(m_arrowLeft, "Yellow/Default/arrow_decorative_w.png");
    ok &= loadTexture(m_arrowRight, "Yellow/Default/arrow_decorative_e.png");
    ok &= loadTexture(m_arrowIndicator, "Yellow/Default/arrow_decorative_s.png");
    ok &= loadTexture(m_star, "Yellow/Default/star.png");
    ok &= loadTexture(m_starEmpty, "Grey/Default/star.png");
    ok &= loadTexture(m_btnStart, "Green/Default/button_rectangle_depth_border.png");
    ok &= loadTexture(m_btnStartHover, "Green/Default/button_rectangle_depth_gradient.png");
    ok &= loadTexture(m_btnBack, "Grey/Default/button_rectangle_depth_flat.png");
    ok &= loadTexture(m_btnBackHover, "Red/Default/button_rectangle_depth_flat.png");
    ok &= loadTexture(m_barBg, "Grey/Default/slide_horizontal_grey_section_wide.png");
    ok &= loadTexture(m_barFill, "Yellow/Default/slide_horizontal_color_section_wide.png");
    ok &= loadTexture(m_barFillGreen, "Green/Default/slide_horizontal_color_section_wide.png");

    if (!ok) {
        std::cerr << "[WARN] [ShowroomUI] Algunas texturas no pudieron cargarse del Kenney UI Pack." << std::endl;
    }

    m_animTimer = 0.0f;
    m_initialized = true;

    std::cout << "[OK] [ShowroomUI] Interfaz grafica del Showroom inicializada con exito." << std::endl;
    return true;
}

void ShowroomUI::destroy() {
    m_tabIdle = renderer::Texture();
    m_tabActive = renderer::Texture();
    m_tabHover = renderer::Texture();
    m_panelFrame = renderer::Texture();
    m_panelInner = renderer::Texture();
    m_arrowLeft = renderer::Texture();
    m_arrowRight = renderer::Texture();
    m_arrowIndicator = renderer::Texture();
    m_star = renderer::Texture();
    m_starEmpty = renderer::Texture();
    m_btnStart = renderer::Texture();
    m_btnStartHover = renderer::Texture();
    m_btnBack = renderer::Texture();
    m_btnBackHover = renderer::Texture();
    m_barBg = renderer::Texture();
    m_barFill = renderer::Texture();
    m_barFillGreen = renderer::Texture();

    if (m_ownedRenderer) {
        m_ownedRenderer->destroy();
        m_ownedRenderer.reset();
    }
    m_renderer = nullptr;
    m_ctx = nullptr;
    m_initialized = false;
}

bool ShowroomUI::areTexturesLoaded() const {
    return m_tabIdle.isLoaded() && m_tabActive.isLoaded() && m_panelFrame.isLoaded() &&
           m_arrowLeft.isLoaded() && m_arrowRight.isLoaded() && m_star.isLoaded() &&
           m_btnStart.isLoaded();
}

void ShowroomUI::update(float deltaTime) {
    m_animTimer += deltaTime;
}

void ShowroomUI::handleInput(float deltaTime) {
    (void)deltaTime;
    if (!m_initialized) return;

    float sw = (m_ctx && m_ctx->window) ? static_cast<float>(m_ctx->window->getWidth()) : 1280.0f;
    float sh = (m_ctx && m_ctx->window) ? static_cast<float>(m_ctx->window->getHeight()) : 720.0f;
    float mx = core::Input::getMouseX();
    float my = core::Input::getMouseY();
    bool mouseClicked = core::Input::isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT);

    // -----------------------------------------------------------------
    // 1. Teclado: Navegación de Flujo (Escape / Enter)
    // -----------------------------------------------------------------
    if (core::Input::isKeyJustPressed(GLFW_KEY_ESCAPE) || core::Input::isKeyJustPressed(GLFW_KEY_BACKSPACE)) {
        returnToMenu();
        return;
    }

    if (core::Input::isKeyJustPressed(GLFW_KEY_ENTER) || core::Input::isKeyJustPressed(GLFW_KEY_KP_ENTER) ||
        core::Input::isKeyJustPressed(GLFW_KEY_SPACE)) {
        confirmSelection();
        return;
    }

    // -----------------------------------------------------------------
    // 2. Teclado: Acceso directo numérico a Categorías (1, 2, 3)
    // -----------------------------------------------------------------
    if (core::Input::isKeyJustPressed(GLFW_KEY_1) || core::Input::isKeyJustPressed(GLFW_KEY_KP_1)) {
        setActiveCategory(core::SelectionCategory::DRIVER);
    } else if (core::Input::isKeyJustPressed(GLFW_KEY_2) || core::Input::isKeyJustPressed(GLFW_KEY_KP_2)) {
        setActiveCategory(core::SelectionCategory::VEHICLE);
    } else if (core::Input::isKeyJustPressed(GLFW_KEY_3) || core::Input::isKeyJustPressed(GLFW_KEY_KP_3)) {
        setActiveCategory(core::SelectionCategory::TRACK);
    }

    // -----------------------------------------------------------------
    // 3. Teclado: Alternar Pestañas / Categorías (TAB, W, S, Up, Down)
    // -----------------------------------------------------------------
    if (core::Input::isKeyJustPressed(GLFW_KEY_TAB) || core::Input::isKeyJustPressed(GLFW_KEY_S) || core::Input::isKeyJustPressed(GLFW_KEY_DOWN)) {
        core::SelectionCategory cat = getActiveCategory();
        if (cat == core::SelectionCategory::DRIVER) setActiveCategory(core::SelectionCategory::VEHICLE);
        else if (cat == core::SelectionCategory::VEHICLE) setActiveCategory(core::SelectionCategory::TRACK);
        else setActiveCategory(core::SelectionCategory::DRIVER);
    } else if (core::Input::isKeyJustPressed(GLFW_KEY_W) || core::Input::isKeyJustPressed(GLFW_KEY_UP)) {
        core::SelectionCategory cat = getActiveCategory();
        if (cat == core::SelectionCategory::DRIVER) setActiveCategory(core::SelectionCategory::TRACK);
        else if (cat == core::SelectionCategory::TRACK) setActiveCategory(core::SelectionCategory::VEHICLE);
        else setActiveCategory(core::SelectionCategory::DRIVER);
    }

    // -----------------------------------------------------------------
    // 4. Teclado: Navegación del Carrusel (A/D, Flechas Izq/Der)
    // -----------------------------------------------------------------
    if (core::Input::isKeyJustPressed(GLFW_KEY_A) || core::Input::isKeyJustPressed(GLFW_KEY_LEFT)) {
        selectPrevious();
    }
    if (core::Input::isKeyJustPressed(GLFW_KEY_D) || core::Input::isKeyJustPressed(GLFW_KEY_RIGHT)) {
        selectNext();
    }

    // -----------------------------------------------------------------
    // 5. Ratón: Clics en la Barra de Pestañas Superior
    // -----------------------------------------------------------------
    const float tabW = 210.0f;
    const float tabH = 46.0f;
    const float tabSpacing = 16.0f;
    const float totalTabsW = 3.0f * tabW + 2.0f * tabSpacing;
    const float startTabsX = (sw - totalTabsW) * 0.5f;
    const float tabsY = 20.0f;

    if (mouseClicked) {
        for (int i = 0; i < 3; ++i) {
            float tx = startTabsX + static_cast<float>(i) * (tabW + tabSpacing);
            if (mx >= tx && mx <= tx + tabW && my >= tabsY && my <= tabsY + tabH) {
                setActiveCategory(static_cast<core::SelectionCategory>(i));
                break;
            }
        }
    }

    // -----------------------------------------------------------------
    // 6. Ratón: Clics en Flechas del Carrusel Central
    // -----------------------------------------------------------------
    const float panelW = 400.0f;
    const float panelX = sw - panelW - 25.0f;
    const float arrowSize = 54.0f;
    const float leftArrowX = 35.0f;
    const float leftArrowY = sh * 0.44f;
    const float rightArrowX = panelX - arrowSize - 20.0f;
    const float rightArrowY = sh * 0.44f;

    if (mouseClicked) {
        // Flecha izquierda [ < ]
        if (mx >= leftArrowX && mx <= leftArrowX + arrowSize &&
            my >= leftArrowY && my <= leftArrowY + arrowSize) {
            selectPrevious();
        }
        // Flecha derecha [ > ]
        if (mx >= rightArrowX && mx <= rightArrowX + arrowSize &&
            my >= rightArrowY && my <= rightArrowY + arrowSize) {
            selectNext();
        }
    }

    // -----------------------------------------------------------------
    // 7. Ratón: Clics en Barra Inferior (Volver / Iniciar)
    // -----------------------------------------------------------------
    const float barH = 65.0f;
    const float barY = sh - barH;
    const float btnBackX = 25.0f;
    const float btnBackY = barY + 11.0f;
    const float btnBackW = 230.0f;
    const float btnBackH = 42.0f;

    const float btnStartX = sw - 280.0f;
    const float btnStartY = barY + 11.0f;
    const float btnStartW = 255.0f;
    const float btnStartH = 42.0f;

    if (mouseClicked) {
        if (mx >= btnBackX && mx <= btnBackX + btnBackW &&
            my >= btnBackY && my <= btnBackY + btnBackH) {
            returnToMenu();
        } else if (mx >= btnStartX && mx <= btnStartX + btnStartW &&
                   my >= btnStartY && my <= btnStartY + btnStartH) {
            confirmSelection();
        }
    }
}

void ShowroomUI::render(float screenWidth, float screenHeight) {
    if (!m_initialized || !m_renderer) return;

    m_renderer->begin(screenWidth, screenHeight);

    // 1. Barra superior con 3 pestañas 9-slice y realce dorado
    renderTopTabs(screenWidth, screenHeight);

    // 2. Selector de carrusel central con flechas interactivas y banner
    renderCenterCarousel(screenWidth, screenHeight);

    // 3. Panel lateral / Tarjeta de estadísticas e información técnica
    renderSidePanel(screenWidth, screenHeight);

    // 4. Barra inferior de navegación con atajos y botones de acción
    renderBottomBar(screenWidth, screenHeight);

    m_renderer->end();
}

void ShowroomUI::renderTopTabs(float screenWidth, float screenHeight) {
    (void)screenHeight;
    const float tabW = 210.0f;
    const float tabH = 46.0f;
    const float tabSpacing = 16.0f;
    const float totalTabsW = 3.0f * tabW + 2.0f * tabSpacing;
    const float startTabsX = (screenWidth - totalTabsW) * 0.5f;
    const float tabsY = 20.0f;

    const float mx = core::Input::getMouseX();
    const float my = core::Input::getMouseY();

    const char* tabNames[3] = {
        "1. CORREDOR",
        "2. VEHICULO",
        "3. CIRCUITO"
    };

    int activeIdx = static_cast<int>(getActiveCategory());

    for (int i = 0; i < 3; ++i) {
        float tx = startTabsX + static_cast<float>(i) * (tabW + tabSpacing);
        bool isActive = (i == activeIdx);
        bool isHover = (mx >= tx && mx <= tx + tabW && my >= tabsY && my <= tabsY + tabH);

        if (isActive) {
            // Fondo activo amarillo 9-slice
            m_renderer->drawNineSlice(m_tabActive.isLoaded() ? m_tabActive : m_tabIdle,
                                      tx, tabsY, tabW, tabH, 12.0f, 12.0f);

            // Borde dorado destacado
            m_renderer->drawBorder(tx - 2.0f, tabsY - 2.0f, tabW + 4.0f, tabH + 4.0f, 3.0f,
                                   glm::vec4(1.0f, 0.85f, 0.15f, 1.0f));

            // Flecha indicadora apuntando hacia abajo
            if (m_arrowIndicator.isLoaded()) {
                m_renderer->drawSprite(m_arrowIndicator,
                                       tx + (tabW - 24.0f) * 0.5f, tabsY + tabH + 3.0f,
                                       24.0f, 20.0f,
                                       glm::vec4(1.0f, 0.85f, 0.15f, 1.0f));
            }

            // Texto contrastado
            m_renderer->drawTextCentered(tx, tabsY, tabW, tabH, tabNames[i], 1.9f,
                                         glm::vec4(0.10f, 0.12f, 0.18f, 1.0f));
        } else if (isHover) {
            // Fondo hover
            m_renderer->drawNineSlice(m_tabHover.isLoaded() ? m_tabHover : m_tabIdle,
                                      tx, tabsY, tabW, tabH, 12.0f, 12.0f,
                                      glm::vec4(1.0f, 1.0f, 0.8f, 1.0f));
            m_renderer->drawBorder(tx, tabsY, tabW, tabH, 1.5f,
                                   glm::vec4(0.9f, 0.9f, 0.5f, 0.8f));
            m_renderer->drawTextCentered(tx, tabsY, tabW, tabH, tabNames[i], 1.8f,
                                         glm::vec4(1.0f, 0.95f, 0.7f, 1.0f));
        } else {
            // Fondo inactivo azul estándar 9-slice
            m_renderer->drawNineSlice(m_tabIdle, tx, tabsY, tabW, tabH, 12.0f, 12.0f);
            m_renderer->drawTextCentered(tx, tabsY, tabW, tabH, tabNames[i], 1.8f,
                                         glm::vec4(0.85f, 0.92f, 1.0f, 0.9f));
        }
    }
}

void ShowroomUI::renderCenterCarousel(float screenWidth, float screenHeight) {
    const float panelW = 400.0f;
    const float panelX = screenWidth - panelW - 25.0f;
    const float availableW = panelX;

    const float mx = core::Input::getMouseX();
    const float my = core::Input::getMouseY();

    const float arrowSize = 54.0f;
    const float leftArrowX = 35.0f;
    const float leftArrowY = screenHeight * 0.44f;
    const float rightArrowX = panelX - arrowSize - 20.0f;
    const float rightArrowY = screenHeight * 0.44f;

    bool hoverLeft = (mx >= leftArrowX && mx <= leftArrowX + arrowSize &&
                      my >= leftArrowY && my <= leftArrowY + arrowSize);
    bool hoverRight = (mx >= rightArrowX && mx <= rightArrowX + arrowSize &&
                       my >= rightArrowY && my <= rightArrowY + arrowSize);

    // 1. Flecha izquierda [ < ]
    if (m_arrowLeft.isLoaded()) {
        float scale = hoverLeft ? 1.15f : 1.0f;
        float drawW = arrowSize * scale;
        float drawH = arrowSize * scale;
        float drawX = leftArrowX - (drawW - arrowSize) * 0.5f;
        float drawY = leftArrowY - (drawH - arrowSize) * 0.5f;
        glm::vec4 tint = hoverLeft ? glm::vec4(1.0f, 1.0f, 0.4f, 1.0f) : glm::vec4(1.0f, 0.88f, 0.25f, 0.9f);

        m_renderer->drawSprite(m_arrowLeft, drawX, drawY, drawW, drawH, tint);
    }

    // 2. Flecha derecha [ > ]
    if (m_arrowRight.isLoaded()) {
        float scale = hoverRight ? 1.15f : 1.0f;
        float drawW = arrowSize * scale;
        float drawH = arrowSize * scale;
        float drawX = rightArrowX - (drawW - arrowSize) * 0.5f;
        float drawY = rightArrowY - (drawH - arrowSize) * 0.5f;
        glm::vec4 tint = hoverRight ? glm::vec4(1.0f, 1.0f, 0.4f, 1.0f) : glm::vec4(1.0f, 0.88f, 0.25f, 0.9f);

        m_renderer->drawSprite(m_arrowRight, drawX, drawY, drawW, drawH, tint);
    }

    // 3. Banner del nombre del elemento seleccionado bajo el pedestal 3D
    std::string currentName;
    core::SelectionCategory cat = getActiveCategory();
    if (m_ctx) {
        if (cat == core::SelectionCategory::DRIVER && !m_ctx->drivers.empty()) {
            int idx = std::clamp(m_ctx->selectedDriverIndex, 0, static_cast<int>(m_ctx->drivers.size()) - 1);
            currentName = m_ctx->drivers[idx].name;
        } else if (cat == core::SelectionCategory::VEHICLE && !m_ctx->vehicles.empty()) {
            int idx = std::clamp(m_ctx->selectedVehicleIndex, 0, static_cast<int>(m_ctx->vehicles.size()) - 1);
            currentName = m_ctx->vehicles[idx].name;
        } else if (cat == core::SelectionCategory::TRACK && !m_ctx->tracks.empty()) {
            int idx = std::clamp(m_ctx->selectedTrackIndex, 0, static_cast<int>(m_ctx->tracks.size()) - 1);
            currentName = m_ctx->tracks[idx].name;
        }
    }
    if (currentName.empty()) currentName = "SELECCIONANDO";

    currentName = sanitizeText(currentName);

    const float bannerW = std::min(460.0f, availableW - 120.0f);
    const float bannerH = 46.0f;
    const float bannerX = (availableW - bannerW) * 0.5f;
    const float bannerY = screenHeight - 146.0f;

    // Placa de contraste y marco 9-slice
    m_renderer->drawRect(bannerX, bannerY, bannerW, bannerH, glm::vec4(0.06f, 0.08f, 0.14f, 0.88f));
    if (m_panelInner.isLoaded()) {
        m_renderer->drawNineSlice(m_panelInner, bannerX, bannerY, bannerW, bannerH, 10.0f, 10.0f,
                                  glm::vec4(0.2f, 0.35f, 0.55f, 0.95f));
    }
    m_renderer->drawBorder(bannerX, bannerY, bannerW, bannerH, 2.0f, glm::vec4(1.0f, 0.85f, 0.2f, 0.8f));

    std::string bannerTitle = "<  " + currentName + "  >";
    m_renderer->drawTextCentered(bannerX, bannerY, bannerW, bannerH, bannerTitle, 2.0f,
                                 glm::vec4(1.0f, 0.95f, 0.4f, 1.0f));

    // Indicador secundario de teclado
    m_renderer->drawTextCentered(bannerX, bannerY + bannerH + 6.0f, bannerW, 20.0f,
                                 "[A / D] O FLECHAS PARA ROTAR SELECCION", 1.4f,
                                 glm::vec4(0.7f, 0.85f, 1.0f, 0.9f));
}

void ShowroomUI::renderSidePanel(float screenWidth, float screenHeight) {
    const float panelW = 400.0f;
    const float panelH = screenHeight - 165.0f;
    const float panelX = screenWidth - panelW - 25.0f;
    const float panelY = 82.0f;

    // Fondo semi-transparente oscuro
    m_renderer->drawRect(panelX, panelY, panelW, panelH, glm::vec4(0.06f, 0.08f, 0.14f, 0.90f));

    // Marco exterior con técnica 9-Slice de Kenney UI Pack
    if (m_panelFrame.isLoaded()) {
        m_renderer->drawNineSlice(m_panelFrame, panelX, panelY, panelW, panelH, 16.0f, 16.0f,
                                  glm::vec4(0.40f, 0.65f, 1.0f, 0.95f));
    } else {
        m_renderer->drawBorder(panelX, panelY, panelW, panelH, 3.0f, glm::vec4(0.4f, 0.65f, 1.0f, 1.0f));
    }

    // Cabecera superior del panel (9-slice)
    const float headerH = 42.0f;
    if (m_tabIdle.isLoaded()) {
        m_renderer->drawNineSlice(m_tabIdle, panelX + 12.0f, panelY + 12.0f, panelW - 24.0f, headerH, 10.0f, 10.0f,
                                  glm::vec4(0.20f, 0.35f, 0.60f, 1.0f));
    }

    core::SelectionCategory cat = getActiveCategory();
    std::string headerTitle = (cat == core::SelectionCategory::DRIVER) ? "DATOS DEL PILOTO" :
                              (cat == core::SelectionCategory::VEHICLE) ? "ESPECIFICACIONES DEL AUTO" :
                                                                          "INFORMACION DEL CIRCUITO";
    m_renderer->drawTextCentered(panelX + 12.0f, panelY + 12.0f, panelW - 24.0f, headerH, headerTitle, 1.8f,
                                 glm::vec4(1.0f, 0.95f, 0.3f, 1.0f));

    // Renderizar vista correspondiente
    float contentX = panelX + 20.0f;
    float contentY = panelY + 66.0f;
    float contentW = panelW - 40.0f;
    float contentH = panelH - 80.0f;

    if (cat == core::SelectionCategory::DRIVER) {
        renderDriverInfo(contentX, contentY, contentW, contentH);
    } else if (cat == core::SelectionCategory::VEHICLE) {
        renderVehicleInfo(contentX, contentY, contentW, contentH);
    } else {
        renderTrackInfo(contentX, contentY, contentW, contentH);
    }
}

void ShowroomUI::renderDriverInfo(float x, float y, float w, float h) {
    (void)h;
    if (!m_ctx || m_ctx->drivers.empty()) return;

    int idx = std::clamp(m_ctx->selectedDriverIndex, 0, static_cast<int>(m_ctx->drivers.size()) - 1);
    const auto& driver = m_ctx->drivers[idx];

    float curY = y;

    // 1. Contador de elementos: (1 / 5)
    std::string counterStr = "PILOTO " + std::to_string(idx + 1) + " / " + std::to_string(m_ctx->drivers.size());
    m_renderer->drawText(x, curY, counterStr, 1.6f, glm::vec4(0.70f, 0.85f, 1.0f, 1.0f));
    curY += 28.0f;

    // 2. Nombre destacado con caja 9-slice y color de acento
    const float nameBoxH = 44.0f;
    if (m_panelInner.isLoaded()) {
        m_renderer->drawNineSlice(m_panelInner, x, curY, w, nameBoxH, 8.0f, 8.0f,
                                  glm::vec4(0.12f, 0.18f, 0.28f, 0.95f));
    }
    m_renderer->drawBorder(x, curY, w, nameBoxH, 1.5f, driver.accentColor);
    m_renderer->drawTextCentered(x, curY, w, nameBoxH, sanitizeText(driver.name), 2.2f, driver.accentColor);
    curY += nameBoxH + 18.0f;

    // 3. Tarjeta de características y biografía
    m_renderer->drawText(x, curY, "PERFIL DEL CORREDOR:", 1.6f, glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));
    curY += 24.0f;

    std::vector<std::string> descLines = wrapText(driver.description, w - 10.0f, 1.5f);
    for (const auto& line : descLines) {
        m_renderer->drawText(x + 4.0f, curY, line, 1.5f, glm::vec4(0.92f, 0.94f, 0.98f, 1.0f));
        curY += 22.0f;
    }
    curY += 14.0f;

    // 4. Rasgos técnicos del personaje
    const float specsBoxH = 150.0f;
    m_renderer->drawRect(x, curY, w, specsBoxH, glm::vec4(0.08f, 0.11f, 0.18f, 0.85f));
    m_renderer->drawBorder(x, curY, w, specsBoxH, 1.5f, glm::vec4(0.3f, 0.45f, 0.7f, 0.7f));

    float specY = curY + 14.0f;
    m_renderer->drawText(x + 14.0f, specY, "CODIGO ID:", 1.4f, glm::vec4(0.7f, 0.8f, 0.95f, 1.0f));
    m_renderer->drawText(x + 130.0f, specY, sanitizeText(driver.id), 1.4f, glm::vec4(0.3f, 0.9f, 1.0f, 1.0f));
    specY += 28.0f;

    m_renderer->drawText(x + 14.0f, specY, "COLOR EQUIPO:", 1.4f, glm::vec4(0.7f, 0.8f, 0.95f, 1.0f));
    // Muestra de color rectangular
    m_renderer->drawRect(x + 130.0f, specY + 2.0f, 32.0f, 14.0f, driver.accentColor);
    m_renderer->drawBorder(x + 130.0f, specY + 2.0f, 32.0f, 14.0f, 1.0f, glm::vec4(1.0f));
    specY += 28.0f;

    m_renderer->drawText(x + 14.0f, specY, "COMPATIBILIDAD:", 1.4f, glm::vec4(0.7f, 0.8f, 0.95f, 1.0f));
    m_renderer->drawText(x + 130.0f, specY, "UNIVERSAL (KART/AUTO)", 1.4f, glm::vec4(0.3f, 1.0f, 0.5f, 1.0f));
    specY += 28.0f;

    m_renderer->drawText(x + 14.0f, specY, "ESTADO:", 1.4f, glm::vec4(0.7f, 0.8f, 0.95f, 1.0f));
    m_renderer->drawText(x + 130.0f, specY, "LISTO EN GRILLA", 1.4f, glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));
}

void ShowroomUI::renderVehicleInfo(float x, float y, float w, float h) {
    (void)h;
    if (!m_ctx || m_ctx->vehicles.empty()) return;

    int idx = std::clamp(m_ctx->selectedVehicleIndex, 0, static_cast<int>(m_ctx->vehicles.size()) - 1);
    const auto& vehicle = m_ctx->vehicles[idx];

    float curY = y;

    // 1. Contador: VEHICULO (1 / 11)
    std::string counterStr = "VEHICULO " + std::to_string(idx + 1) + " / " + std::to_string(m_ctx->vehicles.size());
    m_renderer->drawText(x, curY, counterStr, 1.6f, glm::vec4(0.70f, 0.85f, 1.0f, 1.0f));
    curY += 28.0f;

    // 2. Nombre del vehículo en caja 9-slice
    const float nameBoxH = 44.0f;
    if (m_panelInner.isLoaded()) {
        m_renderer->drawNineSlice(m_panelInner, x, curY, w, nameBoxH, 8.0f, 8.0f,
                                  glm::vec4(0.12f, 0.18f, 0.28f, 0.95f));
    }
    m_renderer->drawBorder(x, curY, w, nameBoxH, 1.5f, glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));
    m_renderer->drawTextCentered(x, curY, w, nameBoxH, sanitizeText(vehicle.name), 2.2f,
                                 glm::vec4(1.0f, 0.95f, 0.35f, 1.0f));
    curY += nameBoxH + 12.0f;

    // 3. Tipo / Categoría
    std::string catBadge = "CATEGORIA: [ " + sanitizeText(vehicle.category) + " ]";
    m_renderer->drawText(x, curY, catBadge, 1.5f, glm::vec4(0.35f, 0.85f, 1.0f, 1.0f));
    curY += 24.0f;

    // 4. Descripción breve
    std::vector<std::string> descLines = wrapText(vehicle.description, w - 10.0f, 1.4f);
    for (const auto& line : descLines) {
        m_renderer->drawText(x + 4.0f, curY, line, 1.4f, glm::vec4(0.90f, 0.92f, 0.96f, 1.0f));
        curY += 20.0f;
    }
    curY += 12.0f;

    // 5. 4 Barras de Estadísticas Progresivas Segmentadas tipo Mario Kart
    // -------------------------------------------------------------
    // Bar 1: VELOCIDAD MAXIMA (km/h)
    float speedKmH = vehicle.maxSpeed * 3.6f;
    std::stringstream ssSpeed;
    ssSpeed << std::fixed << std::setprecision(1) << speedKmH << " KM/H";
    float speedRatio = std::clamp(speedKmH / 65.0f, 0.1f, 1.0f);
    renderStatBar(x, curY, w, 38.0f, "VELOCIDAD MAXIMA", ssSpeed.str(), speedRatio,
                  glm::vec4(1.0f, 0.40f, 0.20f, 1.0f));
    curY += 46.0f;

    // Bar 2: ACELERACIÓN (m/s²)
    std::stringstream ssAcc;
    ssAcc << std::fixed << std::setprecision(1) << vehicle.acceleration << " M/S2";
    float accRatio = std::clamp(vehicle.acceleration / 15.0f, 0.1f, 1.0f);
    renderStatBar(x, curY, w, 38.0f, "ACELERACION", ssAcc.str(), accRatio,
                  glm::vec4(0.20f, 0.80f, 1.0f, 1.0f));
    curY += 46.0f;

    // Bar 3: MANEJO / GIRO (deg/s)
    std::stringstream ssTurn;
    ssTurn << std::fixed << std::setprecision(1) << vehicle.turnSpeed << " DEG/S";
    float turnRatio = std::clamp(vehicle.turnSpeed / 185.0f, 0.1f, 1.0f);
    renderStatBar(x, curY, w, 38.0f, "MANEJO / GIRO", ssTurn.str(), turnRatio,
                  glm::vec4(0.25f, 0.95f, 0.45f, 1.0f));
    curY += 46.0f;

    // Bar 4: TRACCIÓN EN CÉSPED (%)
    int offRoadPct = static_cast<int>(vehicle.offRoadFriction * 100.0f + 0.5f);
    std::string offRoadStr = std::to_string(offRoadPct) + " %";
    float frictionRatio = std::clamp(vehicle.offRoadFriction / 0.40f, 0.1f, 1.0f);
    renderStatBar(x, curY, w, 38.0f, "TRACCION EN CESPED", offRoadStr, frictionRatio,
                  glm::vec4(1.0f, 0.85f, 0.20f, 1.0f));
}

void ShowroomUI::renderTrackInfo(float x, float y, float w, float h) {
    (void)h;
    if (!m_ctx || m_ctx->tracks.empty()) return;

    int idx = std::clamp(m_ctx->selectedTrackIndex, 0, static_cast<int>(m_ctx->tracks.size()) - 1);
    const auto& track = m_ctx->tracks[idx];

    float curY = y;

    // 1. Contador: CIRCUITO (1 / 6)
    std::string counterStr = "CIRCUITO " + std::to_string(idx + 1) + " / " + std::to_string(m_ctx->tracks.size());
    m_renderer->drawText(x, curY, counterStr, 1.6f, glm::vec4(0.70f, 0.85f, 1.0f, 1.0f));
    curY += 28.0f;

    // 2. Nombre del circuito en caja 9-slice
    const float nameBoxH = 44.0f;
    if (m_panelInner.isLoaded()) {
        m_renderer->drawNineSlice(m_panelInner, x, curY, w, nameBoxH, 8.0f, 8.0f,
                                  glm::vec4(0.12f, 0.18f, 0.28f, 0.95f));
    }
    m_renderer->drawBorder(x, curY, w, nameBoxH, 1.5f, glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));
    m_renderer->drawTextCentered(x, curY, w, nameBoxH, sanitizeText(track.name), 1.7f,
                                 glm::vec4(1.0f, 0.95f, 0.35f, 1.0f));
    curY += nameBoxH + 16.0f;

    // 3. Estrellas de dificultad
    m_renderer->drawText(x, curY, "DIFICULTAD:", 1.5f, glm::vec4(0.85f, 0.90f, 0.95f, 1.0f));
    float starX = x + 120.0f;
    const float starSize = 24.0f;

    int numStars = 3;
    if (track.difficulty.find("Extremo") != std::string::npos ||
        track.difficulty.find("Infernal") != std::string::npos) {
        numStars = 5;
    } else if (track.difficulty.find("Dakar") != std::string::npos ||
               track.difficulty.find("Nocturno") != std::string::npos) {
        numStars = 4;
    }

    for (int s = 0; s < 5; ++s) {
        float sx = starX + static_cast<float>(s) * (starSize + 4.0f);
        if (s < numStars) {
            if (m_star.isLoaded()) {
                m_renderer->drawSprite(m_star, sx, curY - 3.0f, starSize, starSize,
                                       glm::vec4(1.0f, 0.85f, 0.15f, 1.0f));
            }
        } else {
            if (m_starEmpty.isLoaded()) {
                m_renderer->drawSprite(m_starEmpty, sx, curY - 3.0f, starSize, starSize,
                                       glm::vec4(0.40f, 0.45f, 0.55f, 0.8f));
            }
        }
    }
    curY += 28.0f;

    // 4. Bioma de la pista
    std::string biomeName;
    glm::vec4 biomeColor(1.0f);
    switch (track.theme) {
        case world::TrackTheme::DAY_GRASSLAND:
            biomeName = "DIA (CESPED CLASICO)";
            biomeColor = glm::vec4(0.35f, 0.90f, 0.35f, 1.0f);
            break;
        case world::TrackTheme::NIGHT_NEON:
            biomeName = "NOCHE (NEON GP - FAROLAS)";
            biomeColor = glm::vec4(0.40f, 0.85f, 1.0f, 1.0f);
            break;
        case world::TrackTheme::DESERT_DUNES:
            biomeName = "DESIERTO (DUNAS DAKAR)";
            biomeColor = glm::vec4(1.0f, 0.75f, 0.30f, 1.0f);
            break;
        case world::TrackTheme::COSMIC_SPACE:
            biomeName = "ESPACIO (ORBITAL ESTELAR)";
            biomeColor = glm::vec4(0.85f, 0.50f, 1.0f, 1.0f);
            break;
        case world::TrackTheme::AQUATIC_REEF:
            biomeName = "ACUATICO (ARRECIFE ABISAL)";
            biomeColor = glm::vec4(0.20f, 0.95f, 0.85f, 1.0f);
            break;
        case world::TrackTheme::VOLCANO_CALDERA:
            biomeName = "VOLCAN (CRATER DE MAGMA)";
            biomeColor = glm::vec4(1.0f, 0.35f, 0.20f, 1.0f);
            break;
    }

    m_renderer->drawText(x, curY, "BIOMA:", 1.5f, glm::vec4(0.85f, 0.90f, 0.95f, 1.0f));
    m_renderer->drawText(x + 75.0f, curY, biomeName, 1.5f, biomeColor);
    curY += 26.0f;

    // 5. Dimensiones de la pista
    std::string dimStr = "DIMENSIONES: " + std::to_string(track.cols) + " x " + std::to_string(track.rows) + " CELDAS";
    m_renderer->drawText(x, curY, dimStr, 1.5f, glm::vec4(0.70f, 0.85f, 1.0f, 1.0f));
    curY += 28.0f;

    // 6. Descripción del trazado
    m_renderer->drawText(x, curY, "DESCRIPCION DEL TRAZADO:", 1.5f, glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));
    curY += 22.0f;

    std::vector<std::string> descLines = wrapText(track.description, w - 10.0f, 1.4f);
    for (const auto& line : descLines) {
        m_renderer->drawText(x + 4.0f, curY, line, 1.4f, glm::vec4(0.90f, 0.94f, 0.98f, 1.0f));
        curY += 20.0f;
    }
}

void ShowroomUI::renderStatBar(float x, float y, float w, float h,
                               const std::string& label, const std::string& valueStr,
                               float ratio, const glm::vec4& fillColor) {
    if (!m_renderer) return;

    // Etiqueta y valor numérico
    m_renderer->drawText(x, y, label, 1.4f, glm::vec4(0.85f, 0.90f, 0.96f, 1.0f));
    float valW = m_renderer->getTextWidth(valueStr, 1.4f);
    m_renderer->drawText(x + w - valW, y, valueStr, 1.4f, fillColor);

    // Contenedor de la barra
    float barY = y + 16.0f;
    float barH = h - 16.0f;
    if (barH < 12.0f) barH = 12.0f;

    // Fondo gris 3-slice
    if (m_barBg.isLoaded()) {
        m_renderer->drawThreeSliceHorizontal(m_barBg, x, barY, w, barH, 8.0f,
                                             glm::vec4(0.30f, 0.35f, 0.45f, 0.95f));
    } else {
        m_renderer->drawRect(x, barY, w, barH, glm::vec4(0.18f, 0.20f, 0.25f, 0.95f));
    }

    // Relleno progresivo con 3-slice / recorte
    float clampedRatio = std::clamp(ratio, 0.05f, 1.0f);
    if (m_barFill.isLoaded()) {
        m_renderer->drawSpriteProgress(m_barFill, x + 2.0f, barY + 2.0f, w - 4.0f, barH - 4.0f,
                                       clampedRatio, true, fillColor);
    } else {
        float fillW = clampedRatio * (w - 4.0f);
        m_renderer->drawRect(x + 2.0f, barY + 2.0f, fillW, barH - 4.0f, fillColor);
    }

    // Ticks verticales tipo Mario Kart (4 segmentos / 5 divisiones)
    for (int t = 1; t <= 4; ++t) {
        float tickX = x + (w * 0.20f * static_cast<float>(t));
        m_renderer->drawRect(tickX - 1.0f, barY + 1.0f, 2.0f, barH - 2.0f,
                             glm::vec4(0.08f, 0.10f, 0.15f, 0.65f));
    }

    // Borde exterior sutil
    m_renderer->drawBorder(x, barY, w, barH, 1.0f, glm::vec4(0.4f, 0.5f, 0.65f, 0.5f));
}

void ShowroomUI::renderBottomBar(float screenWidth, float screenHeight) {
    const float barH = 65.0f;
    const float barY = screenHeight - barH;

    const float mx = core::Input::getMouseX();
    const float my = core::Input::getMouseY();

    // 1. Zócalo inferior oscuro con división sutil
    m_renderer->drawRect(0.0f, barY, screenWidth, barH, glm::vec4(0.04f, 0.06f, 0.10f, 0.96f));
    m_renderer->drawBorder(0.0f, barY, screenWidth, 2.0f, 2.0f, glm::vec4(0.25f, 0.35f, 0.55f, 1.0f));

    // 2. Botón Izquierdo: [ ESC: VOLVER AL MENÚ ]
    const float btnBackX = 25.0f;
    const float btnBackY = barY + 11.0f;
    const float btnBackW = 230.0f;
    const float btnBackH = 42.0f;
    bool hoverBack = (mx >= btnBackX && mx <= btnBackX + btnBackW &&
                      my >= btnBackY && my <= btnBackY + btnBackH);

    renderer::Texture& backTex = hoverBack ? (m_btnBackHover.isLoaded() ? m_btnBackHover : m_btnBack)
                                           : m_btnBack;
    if (backTex.isLoaded()) {
        m_renderer->drawNineSlice(backTex, btnBackX, btnBackY, btnBackW, btnBackH, 12.0f, 12.0f);
    } else {
        m_renderer->drawRect(btnBackX, btnBackY, btnBackW, btnBackH,
                             hoverBack ? glm::vec4(0.7f, 0.2f, 0.2f, 1.0f) : glm::vec4(0.3f, 0.35f, 0.45f, 1.0f));
    }
    m_renderer->drawTextCentered(btnBackX, btnBackY, btnBackW, btnBackH, "[ESC] VOLVER AL MENU", 1.6f,
                                 hoverBack ? glm::vec4(1.0f, 0.9f, 0.9f, 1.0f) : glm::vec4(0.9f, 0.95f, 1.0f, 1.0f));

    // 3. Botón Derecho Destacado: [ ENTER / ESPACIO: ¡INICIAR CARRERA! ] (9-slice verde con hover y pulso)
    const float btnStartX = screenWidth - 280.0f;
    const float btnStartY = barY + 11.0f;
    const float btnStartW = 255.0f;
    const float btnStartH = 42.0f;
    bool hoverStart = (mx >= btnStartX && mx <= btnStartX + btnStartW &&
                       my >= btnStartY && my <= btnStartY + btnStartH);

    float pulse = (std::sin(m_animTimer * 5.0f) + 1.0f) * 0.5f;
    glm::vec4 startTint = hoverStart ? glm::vec4(1.0f, 1.0f, 0.85f, 1.0f)
                                     : glm::vec4(0.90f + 0.10f * pulse, 1.0f, 0.90f + 0.10f * pulse, 1.0f);

    renderer::Texture& startTex = hoverStart ? (m_btnStartHover.isLoaded() ? m_btnStartHover : m_btnStart)
                                             : m_btnStart;
    if (startTex.isLoaded()) {
        m_renderer->drawNineSlice(startTex, btnStartX, btnStartY, btnStartW, btnStartH, 12.0f, 12.0f, startTint);
    } else {
        m_renderer->drawRect(btnStartX, btnStartY, btnStartW, btnStartH, glm::vec4(0.2f, 0.7f, 0.3f, 1.0f));
    }
    m_renderer->drawTextCentered(btnStartX, btnStartY, btnStartW, btnStartH, "[ENTER] INICIAR CARRERA!", 1.6f,
                                 glm::vec4(1.0f, 1.0f, 0.95f, 1.0f));

    // 4. Centro: Resumen de atajos de teclado
    float centerBoxX = btnBackX + btnBackW + 10.0f;
    float centerBoxW = btnStartX - centerBoxX - 10.0f;
    m_renderer->drawTextCentered(centerBoxX, barY, centerBoxW, barH,
                                 "[TAB / 1-2-3] PESTANA  |  [A / D] ELEGIR  |  [ENTER] CORRER", 1.4f,
                                 glm::vec4(0.70f, 0.85f, 1.0f, 0.9f));
}

void ShowroomUI::setActiveCategory(core::SelectionCategory category) {
    if (m_ctx) {
        m_ctx->selectionCategory = category;
        m_ctx->printSelectionHUD();
    }
}

core::SelectionCategory ShowroomUI::getActiveCategory() const {
    return m_ctx ? m_ctx->selectionCategory : core::SelectionCategory::DRIVER;
}

void ShowroomUI::selectPrevious() {
    if (!m_ctx) return;
    switch (getActiveCategory()) {
        case core::SelectionCategory::DRIVER:
            if (!m_ctx->drivers.empty()) {
                int count = static_cast<int>(m_ctx->drivers.size());
                int nextIdx = ((m_ctx->selectedDriverIndex - 1) % count + count) % count;
                setDriverIndex(nextIdx);
            }
            break;
        case core::SelectionCategory::VEHICLE:
            if (!m_ctx->vehicles.empty()) {
                int count = static_cast<int>(m_ctx->vehicles.size());
                int nextIdx = ((m_ctx->selectedVehicleIndex - 1) % count + count) % count;
                setVehicleIndex(nextIdx);
            }
            break;
        case core::SelectionCategory::TRACK:
            if (!m_ctx->tracks.empty()) {
                int count = static_cast<int>(m_ctx->tracks.size());
                int nextIdx = ((m_ctx->selectedTrackIndex - 1) % count + count) % count;
                setTrackIndex(nextIdx);
            }
            break;
    }
}

void ShowroomUI::selectNext() {
    if (!m_ctx) return;
    switch (getActiveCategory()) {
        case core::SelectionCategory::DRIVER:
            if (!m_ctx->drivers.empty()) {
                int count = static_cast<int>(m_ctx->drivers.size());
                int nextIdx = ((m_ctx->selectedDriverIndex + 1) % count + count) % count;
                setDriverIndex(nextIdx);
            }
            break;
        case core::SelectionCategory::VEHICLE:
            if (!m_ctx->vehicles.empty()) {
                int count = static_cast<int>(m_ctx->vehicles.size());
                int nextIdx = ((m_ctx->selectedVehicleIndex + 1) % count + count) % count;
                setVehicleIndex(nextIdx);
            }
            break;
        case core::SelectionCategory::TRACK:
            if (!m_ctx->tracks.empty()) {
                int count = static_cast<int>(m_ctx->tracks.size());
                int nextIdx = ((m_ctx->selectedTrackIndex + 1) % count + count) % count;
                setTrackIndex(nextIdx);
            }
            break;
    }
}

void ShowroomUI::confirmSelection() {
    if (!m_ctx) return;
    if (!m_ctx->tracks.empty() && m_ctx->selectedTrackIndex < static_cast<int>(m_ctx->tracks.size())) {
        std::cout << "[SHOWROOM] Seleccion confirmada! Iniciando carrera en "
                  << m_ctx->tracks[m_ctx->selectedTrackIndex].name << "..." << std::endl;
    }
    if (m_ctx->sceneManager) {
        m_ctx->sceneManager->changeScene(core::GameState::RACING);
    }
}

void ShowroomUI::returnToMenu() {
    if (!m_ctx) return;
    std::cout << "[SHOWROOM] Regresando al Menu Principal..." << std::endl;
    if (m_ctx->sceneManager) {
        m_ctx->sceneManager->changeScene(core::GameState::MAIN_MENU);
    }
}

int ShowroomUI::getSelectedDriverIndex() const {
    return m_ctx ? m_ctx->selectedDriverIndex : 0;
}

int ShowroomUI::getSelectedVehicleIndex() const {
    return m_ctx ? m_ctx->selectedVehicleIndex : 0;
}

int ShowroomUI::getSelectedTrackIndex() const {
    return m_ctx ? m_ctx->selectedTrackIndex : 0;
}

void ShowroomUI::setDriverIndex(int index) {
    if (!m_ctx || m_ctx->drivers.empty()) return;
    int count = static_cast<int>(m_ctx->drivers.size());
    int validIdx = ((index % count) + count) % count;
    m_ctx->selectDriver(validIdx);
}

void ShowroomUI::setVehicleIndex(int index) {
    if (!m_ctx || m_ctx->vehicles.empty()) return;
    int count = static_cast<int>(m_ctx->vehicles.size());
    int validIdx = ((index % count) + count) % count;
    m_ctx->selectVehicle(validIdx);
}

void ShowroomUI::setTrackIndex(int index) {
    if (!m_ctx || m_ctx->tracks.empty()) return;
    int count = static_cast<int>(m_ctx->tracks.size());
    int validIdx = ((index % count) + count) % count;

    if (m_ctx->trackGrid && m_ctx->propSpawner && m_ctx->zoneManager && m_ctx->trackRenderer) {
        m_ctx->selectTrack(validIdx);
    } else {
        m_ctx->selectedTrackIndex = validIdx;
        m_ctx->printSelectionHUD();
    }
}

std::vector<std::string> ShowroomUI::wrapText(const std::string& text, float maxWidth, float scale) const {
    std::vector<std::string> lines;
    if (text.empty() || maxWidth <= 0.0f) return lines;

    std::string clean = sanitizeText(text);
    std::istringstream words(clean);
    std::string word;
    std::string currentLine;

    while (words >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        float testW = m_renderer ? m_renderer->getTextWidth(testLine, scale)
                                 : static_cast<float>(testLine.size()) * 6.0f * scale;
        if (testW <= maxWidth) {
            currentLine = testLine;
        } else {
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
            }
            currentLine = word;
        }
    }
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    return lines;
}

std::string ShowroomUI::sanitizeText(const std::string& text) const {
    std::string result;
    result.reserve(text.size());

    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);

        // Manejo básico de caracteres acentuados en UTF-8
        if (c == 0xC3 && i + 1 < text.size()) {
            unsigned char c2 = static_cast<unsigned char>(text[++i]);
            switch (c2) {
                case 0xA1: case 0x81: result += 'A'; break; // á, Á
                case 0xA9: case 0x89: result += 'E'; break; // é, É
                case 0xAD: case 0x8D: result += 'I'; break; // í, Í
                case 0xB3: case 0x93: result += 'O'; break; // ó, Ó
                case 0xBA: case 0x9A: result += 'U'; break; // ú, Ú
                case 0xB1: case 0x91: result += 'N'; break; // ñ, Ñ
                default: result += '?'; break;
            }
            continue;
        }

        if (c >= 32 && c <= 126) {
            result += static_cast<char>(std::toupper(c));
        } else {
            result += ' ';
        }
    }

    return result;
}

} // namespace ui
