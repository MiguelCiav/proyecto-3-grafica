#include "ui/ShowroomUI.hpp"
#include "ui/UIUtils.hpp"
#include "ui/UIWidgets.hpp"
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
#include <utility>

namespace ui {

ShowroomUI::ShowroomUI() = default;

ShowroomUI::~ShowroomUI() {
    destroy();
}

ShowroomUI::ShowroomUI(ShowroomUI&& other) noexcept = default;
ShowroomUI& ShowroomUI::operator=(ShowroomUI&& other) noexcept = default;

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

    const std::pair<renderer::Texture*, const char*> texList[] = {
        {&m_tabIdle,        "Blue/Default/button_rectangle_depth_flat.png"},
        {&m_tabActive,      "Yellow/Default/button_rectangle_depth_gradient.png"},
        {&m_tabHover,       "Yellow/Default/button_rectangle_depth_flat.png"},
        {&m_panelFrame,     "Blue/Default/button_square_border.png"},
        {&m_panelInner,     "Grey/Default/button_square_depth_flat.png"},
        {&m_arrowLeft,      "Yellow/Default/arrow_decorative_w.png"},
        {&m_arrowRight,     "Yellow/Default/arrow_decorative_e.png"},
        {&m_arrowIndicator, "Yellow/Default/arrow_decorative_s.png"},
        {&m_star,           "Yellow/Default/star.png"},
        {&m_starEmpty,      "Grey/Default/star.png"},
        {&m_btnStart,       "Green/Default/button_rectangle_depth_border.png"},
        {&m_btnStartHover,  "Green/Default/button_rectangle_depth_gradient.png"},
        {&m_btnBack,        "Grey/Default/button_rectangle_depth_flat.png"},
        {&m_btnBackHover,   "Red/Default/button_rectangle_depth_flat.png"},
        {&m_barBg,          "Grey/Default/slide_horizontal_grey_section_wide.png"},
        {&m_barFill,        "Yellow/Default/slide_horizontal_color_section_wide.png"},
        {&m_barFillGreen,   "Green/Default/slide_horizontal_color_section_wide.png"}
    };

    bool ok = true;
    for (const auto& [texPtr, path] : texList) {
        ok &= loadTexture(*texPtr, path);
    }

    if (!ok) {
        std::cerr << "[WARN] [ShowroomUI] Algunas texturas no pudieron cargarse del Kenney UI Pack." << std::endl;
    }

    m_animTimer = 0.0f;
    m_initialized = true;
    std::cout << "[OK] [ShowroomUI] Interfaz grafica del Showroom inicializada con exito." << std::endl;
    return true;
}

void ShowroomUI::destroy() {
    for (auto* tex : {&m_tabIdle, &m_tabActive, &m_tabHover, &m_panelFrame, &m_panelInner,
                      &m_arrowLeft, &m_arrowRight, &m_arrowIndicator, &m_star, &m_starEmpty,
                      &m_btnStart, &m_btnStartHover, &m_btnBack, &m_btnBackHover,
                      &m_barBg, &m_barFill, &m_barFillGreen}) {
        *tex = renderer::Texture();
    }

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

    if (core::Input::isKeyJustPressed(GLFW_KEY_ESCAPE) || core::Input::isKeyJustPressed(GLFW_KEY_BACKSPACE)) {
        returnToMenu();
        return;
    }

    if (core::Input::isKeyJustPressed(GLFW_KEY_ENTER) || core::Input::isKeyJustPressed(GLFW_KEY_KP_ENTER) ||
        core::Input::isKeyJustPressed(GLFW_KEY_SPACE)) {
        confirmSelection();
        return;
    }

    if (core::Input::isKeyJustPressed(GLFW_KEY_1) || core::Input::isKeyJustPressed(GLFW_KEY_KP_1)) {
        setActiveCategory(core::SelectionCategory::DRIVER);
    } else if (core::Input::isKeyJustPressed(GLFW_KEY_2) || core::Input::isKeyJustPressed(GLFW_KEY_KP_2)) {
        setActiveCategory(core::SelectionCategory::VEHICLE);
    } else if (core::Input::isKeyJustPressed(GLFW_KEY_3) || core::Input::isKeyJustPressed(GLFW_KEY_KP_3)) {
        setActiveCategory(core::SelectionCategory::TRACK);
    }

    if (core::Input::isKeyJustPressed(GLFW_KEY_TAB) || core::Input::isKeyJustPressed(GLFW_KEY_S) || core::Input::isKeyJustPressed(GLFW_KEY_DOWN)) {
        int nextCat = (static_cast<int>(getActiveCategory()) + 1) % 3;
        setActiveCategory(static_cast<core::SelectionCategory>(nextCat));
    } else if (core::Input::isKeyJustPressed(GLFW_KEY_W) || core::Input::isKeyJustPressed(GLFW_KEY_UP)) {
        int prevCat = (static_cast<int>(getActiveCategory()) + 2) % 3;
        setActiveCategory(static_cast<core::SelectionCategory>(prevCat));
    }

    if (core::Input::isKeyJustPressed(GLFW_KEY_A) || core::Input::isKeyJustPressed(GLFW_KEY_LEFT)) {
        selectPrevious();
    }
    if (core::Input::isKeyJustPressed(GLFW_KEY_D) || core::Input::isKeyJustPressed(GLFW_KEY_RIGHT)) {
        selectNext();
    }

    if (!mouseClicked) return;

    const float tabW = 210.0f, tabH = 46.0f, tabSpacing = 16.0f;
    const float startTabsX = (sw - (3.0f * tabW + 2.0f * tabSpacing)) * 0.5f;
    int clickedTab = UITabBar::hitTest(startTabsX, 20.0f, tabW, tabH, tabSpacing, 3, mx, my);
    if (clickedTab >= 0) {
        setActiveCategory(static_cast<core::SelectionCategory>(clickedTab));
    }

    const float panelX = sw - 425.0f;
    const float arrowSize = 54.0f;
    const float arrowY = sh * 0.44f;
    if (UIRect{35.0f, arrowY, arrowSize, arrowSize}.contains(mx, my)) {
        selectPrevious();
    } else if (UIRect{panelX - arrowSize - 20.0f, arrowY, arrowSize, arrowSize}.contains(mx, my)) {
        selectNext();
    }

    const float btnY = sh - 54.0f;
    if (UIRect{25.0f, btnY, 230.0f, 42.0f}.contains(mx, my)) {
        returnToMenu();
    } else if (UIRect{sw - 280.0f, btnY, 255.0f, 42.0f}.contains(mx, my)) {
        confirmSelection();
    }
}

void ShowroomUI::render(float screenWidth, float screenHeight) {
    if (!m_initialized || !m_renderer) return;

    m_renderer->begin(screenWidth, screenHeight);
    renderTopTabs(screenWidth, screenHeight);
    renderCenterCarousel(screenWidth, screenHeight);
    renderSidePanel(screenWidth, screenHeight);
    renderBottomBar(screenWidth, screenHeight);
    m_renderer->end();
}

void ShowroomUI::renderTopTabs(float screenWidth, float screenHeight) {
    (void)screenHeight;
    const float tabW = 210.0f, tabH = 46.0f, tabSpacing = 16.0f;
    const float startTabsX = (screenWidth - (3.0f * tabW + 2.0f * tabSpacing)) * 0.5f;

    UITabBar::draw(*m_renderer, startTabsX, 20.0f, tabW, tabH, tabSpacing,
                   {"1. CORREDOR", "2. VEHICULO", "3. CIRCUITO"},
                   static_cast<int>(getActiveCategory()),
                   m_tabIdle, m_tabActive, m_tabHover, &m_arrowIndicator,
                   core::Input::getMouseX(), core::Input::getMouseY(), false);
}

void ShowroomUI::renderCenterCarousel(float screenWidth, float screenHeight) {
    const float availableW = screenWidth - 425.0f;
    const float mx = core::Input::getMouseX();
    const float my = core::Input::getMouseY();

    const float arrowSize = 54.0f;
    const float arrowY = screenHeight * 0.44f;
    UIRect leftRect{35.0f, arrowY, arrowSize, arrowSize};
    UIRect rightRect{availableW - arrowSize - 20.0f, arrowY, arrowSize, arrowSize};

    UIButton::drawIconButton(*m_renderer, leftRect, m_arrowLeft, leftRect.contains(mx, my));
    UIButton::drawIconButton(*m_renderer, rightRect, m_arrowRight, rightRect.contains(mx, my));

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

    const float bannerW = std::min(460.0f, availableW - 120.0f);
    const float bannerH = 46.0f;
    UIRect bannerRect{(availableW - bannerW) * 0.5f, screenHeight - 146.0f, bannerW, bannerH};

    m_renderer->drawRect(bannerRect.x, bannerRect.y, bannerRect.w, bannerRect.h, glm::vec4(0.06f, 0.08f, 0.14f, 0.88f));
    UIPanel::drawBannerBox(*m_renderer, bannerRect, m_panelInner,
                           "<  " + sanitizeText(currentName) + "  >", 2.0f,
                           glm::vec4(1.0f, 0.95f, 0.4f, 1.0f),
                           glm::vec4(1.0f, 0.85f, 0.2f, 0.8f),
                           glm::vec4(0.2f, 0.35f, 0.55f, 0.95f), 10.0f);

    m_renderer->drawTextCentered(bannerRect.x, bannerRect.y + bannerRect.h + 6.0f, bannerRect.w, 20.0f,
                                 "[A / D] O FLECHAS PARA ROTAR SELECCION", 1.4f,
                                 glm::vec4(0.7f, 0.85f, 1.0f, 0.9f));
}

void ShowroomUI::renderSidePanel(float screenWidth, float screenHeight) {
    UIRect panelRect{screenWidth - 425.0f, 82.0f, 400.0f, screenHeight - 165.0f};
    UIPanel::drawFramedPanel(*m_renderer, panelRect, m_panelFrame, 16.0f,
                             glm::vec4(0.06f, 0.08f, 0.14f, 0.90f),
                             glm::vec4(0.40f, 0.65f, 1.0f, 0.95f));

    core::SelectionCategory cat = getActiveCategory();
    std::string headerTitle = (cat == core::SelectionCategory::DRIVER) ? "DATOS DEL PILOTO" :
                              (cat == core::SelectionCategory::VEHICLE) ? "ESPECIFICACIONES DEL AUTO" :
                                                                          "INFORMACION DEL CIRCUITO";

    UIPanel::drawBannerBox(*m_renderer,
                           UIRect{panelRect.x + 12.0f, panelRect.y + 12.0f, panelRect.w - 24.0f, 42.0f},
                           m_tabIdle, headerTitle, 1.8f,
                           glm::vec4(1.0f, 0.95f, 0.3f, 1.0f), glm::vec4(0.0f),
                           glm::vec4(0.20f, 0.35f, 0.60f, 1.0f), 10.0f);

    float cx = panelRect.x + 20.0f, cy = panelRect.y + 66.0f;
    float cw = panelRect.w - 40.0f, ch = panelRect.h - 80.0f;
    if (cat == core::SelectionCategory::DRIVER) renderDriverInfo(cx, cy, cw, ch);
    else if (cat == core::SelectionCategory::VEHICLE) renderVehicleInfo(cx, cy, cw, ch);
    else renderTrackInfo(cx, cy, cw, ch);
}

void ShowroomUI::renderDriverInfo(float x, float y, float w, float h) {
    (void)h;
    if (!m_ctx || m_ctx->drivers.empty()) return;

    int idx = std::clamp(m_ctx->selectedDriverIndex, 0, static_cast<int>(m_ctx->drivers.size()) - 1);
    const auto& driver = m_ctx->drivers[idx];
    float curY = y;

    m_renderer->drawText(x, curY, "PILOTO " + std::to_string(idx + 1) + " / " + std::to_string(m_ctx->drivers.size()),
                         1.6f, glm::vec4(0.70f, 0.85f, 1.0f, 1.0f));
    curY += 28.0f;

    UIPanel::drawBannerBox(*m_renderer, UIRect{x, curY, w, 44.0f}, m_panelInner,
                           driver.name, 2.2f, driver.accentColor, driver.accentColor);
    curY += 62.0f;

    m_renderer->drawText(x, curY, "PERFIL DEL CORREDOR:", 1.6f, glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));
    curY += 24.0f;

    for (const auto& line : wrapText(driver.description, w - 10.0f, 1.5f)) {
        m_renderer->drawText(x + 4.0f, curY, line, 1.5f, glm::vec4(0.92f, 0.94f, 0.98f, 1.0f));
        curY += 22.0f;
    }
    curY += 14.0f;

    m_renderer->drawRect(x, curY, w, 150.0f, glm::vec4(0.08f, 0.11f, 0.18f, 0.85f));
    m_renderer->drawBorder(x, curY, w, 150.0f, 1.5f, glm::vec4(0.3f, 0.45f, 0.7f, 0.7f));

    float specY = curY + 14.0f;
    m_renderer->drawText(x + 14.0f, specY, "CODIGO ID:", 1.4f, glm::vec4(0.7f, 0.8f, 0.95f, 1.0f));
    m_renderer->drawText(x + 130.0f, specY, sanitizeText(driver.id), 1.4f, glm::vec4(0.3f, 0.9f, 1.0f, 1.0f));
    specY += 28.0f;

    m_renderer->drawText(x + 14.0f, specY, "COLOR EQUIPO:", 1.4f, glm::vec4(0.7f, 0.8f, 0.95f, 1.0f));
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

    m_renderer->drawText(x, curY, "VEHICULO " + std::to_string(idx + 1) + " / " + std::to_string(m_ctx->vehicles.size()),
                         1.6f, glm::vec4(0.70f, 0.85f, 1.0f, 1.0f));
    curY += 28.0f;

    UIPanel::drawBannerBox(*m_renderer, UIRect{x, curY, w, 44.0f}, m_panelInner,
                           vehicle.name, 2.2f, glm::vec4(1.0f, 0.95f, 0.35f, 1.0f),
                           glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));
    curY += 56.0f;

    m_renderer->drawText(x, curY, "CATEGORIA: [ " + sanitizeText(vehicle.category) + " ]",
                         1.5f, glm::vec4(0.35f, 0.85f, 1.0f, 1.0f));
    curY += 24.0f;

    for (const auto& line : wrapText(vehicle.description, w - 10.0f, 1.4f)) {
        m_renderer->drawText(x + 4.0f, curY, line, 1.4f, glm::vec4(0.90f, 0.92f, 0.96f, 1.0f));
        curY += 20.0f;
    }
    curY += 12.0f;

    auto formatOneDec = [](float val, const char* unit) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << val << " " << unit;
        return ss.str();
    };

    float speedKmH = vehicle.maxSpeed * 3.6f;
    UIProgressBar::drawStatBar(*m_renderer, UIRect{x, curY, w, 38.0f}, "VELOCIDAD MAXIMA",
                               formatOneDec(speedKmH, "KM/H"), std::clamp(speedKmH / 65.0f, 0.1f, 1.0f),
                               glm::vec4(1.0f, 0.40f, 0.20f, 1.0f), m_barBg, m_barFill);
    curY += 46.0f;

    UIProgressBar::drawStatBar(*m_renderer, UIRect{x, curY, w, 38.0f}, "ACELERACION",
                               formatOneDec(vehicle.acceleration, "M/S2"), std::clamp(vehicle.acceleration / 15.0f, 0.1f, 1.0f),
                               glm::vec4(0.20f, 0.80f, 1.0f, 1.0f), m_barBg, m_barFill);
    curY += 46.0f;

    UIProgressBar::drawStatBar(*m_renderer, UIRect{x, curY, w, 38.0f}, "MANEJO / GIRO",
                               formatOneDec(vehicle.turnSpeed, "DEG/S"), std::clamp(vehicle.turnSpeed / 185.0f, 0.1f, 1.0f),
                               glm::vec4(0.25f, 0.95f, 0.45f, 1.0f), m_barBg, m_barFill);
    curY += 46.0f;

    int offRoadPct = static_cast<int>(vehicle.offRoadFriction * 100.0f + 0.5f);
    UIProgressBar::drawStatBar(*m_renderer, UIRect{x, curY, w, 38.0f}, "TRACCION EN CESPED",
                               std::to_string(offRoadPct) + " %", std::clamp(vehicle.offRoadFriction / 0.40f, 0.1f, 1.0f),
                               glm::vec4(1.0f, 0.85f, 0.20f, 1.0f), m_barBg, m_barFill);
}

void ShowroomUI::renderTrackInfo(float x, float y, float w, float h) {
    (void)h;
    if (!m_ctx || m_ctx->tracks.empty()) return;

    int idx = std::clamp(m_ctx->selectedTrackIndex, 0, static_cast<int>(m_ctx->tracks.size()) - 1);
    const auto& track = m_ctx->tracks[idx];
    float curY = y;

    m_renderer->drawText(x, curY, "CIRCUITO " + std::to_string(idx + 1) + " / " + std::to_string(m_ctx->tracks.size()),
                         1.6f, glm::vec4(0.70f, 0.85f, 1.0f, 1.0f));
    curY += 28.0f;

    UIPanel::drawBannerBox(*m_renderer, UIRect{x, curY, w, 44.0f}, m_panelInner,
                           track.name, 1.7f, glm::vec4(1.0f, 0.95f, 0.35f, 1.0f),
                           glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));
    curY += 60.0f;

    m_renderer->drawText(x, curY, "DIFICULTAD:", 1.5f, glm::vec4(0.85f, 0.90f, 0.95f, 1.0f));
    int numStars = 3;
    if (track.difficulty.find("Extremo") != std::string::npos || track.difficulty.find("Infernal") != std::string::npos) {
        numStars = 5;
    } else if (track.difficulty.find("Dakar") != std::string::npos || track.difficulty.find("Nocturno") != std::string::npos) {
        numStars = 4;
    }

    for (int s = 0; s < 5; ++s) {
        float sx = x + 120.0f + static_cast<float>(s) * 28.0f;
        const renderer::Texture& starTex = (s < numStars) ? m_star : m_starEmpty;
        glm::vec4 starTint = (s < numStars) ? glm::vec4(1.0f, 0.85f, 0.15f, 1.0f) : glm::vec4(0.40f, 0.45f, 0.55f, 0.8f);
        if (starTex.isLoaded()) {
            m_renderer->drawSprite(starTex, sx, curY - 3.0f, 24.0f, 24.0f, starTint);
        }
    }
    curY += 28.0f;

    std::string biomeName = "DIA (CESPED CLASICO)";
    glm::vec4 biomeColor(0.35f, 0.90f, 0.35f, 1.0f);
    switch (track.theme) {
        case world::TrackTheme::DAY_GRASSLAND:   break;
        case world::TrackTheme::NIGHT_NEON:      biomeName = "NOCHE (NEON GP - FAROLAS)"; biomeColor = glm::vec4(0.40f, 0.85f, 1.0f, 1.0f); break;
        case world::TrackTheme::DESERT_DUNES:    biomeName = "DESIERTO (DUNAS DAKAR)";    biomeColor = glm::vec4(1.0f, 0.75f, 0.30f, 1.0f); break;
        case world::TrackTheme::COSMIC_SPACE:    biomeName = "ESPACIO (ORBITAL ESTELAR)"; biomeColor = glm::vec4(0.85f, 0.50f, 1.0f, 1.0f); break;
        case world::TrackTheme::AQUATIC_REEF:    biomeName = "ACUATICO (ARRECIFE ABISAL)"; biomeColor = glm::vec4(0.20f, 0.95f, 0.85f, 1.0f); break;
        case world::TrackTheme::VOLCANO_CALDERA: biomeName = "VOLCAN (CRATER DE MAGMA)";  biomeColor = glm::vec4(1.0f, 0.35f, 0.20f, 1.0f); break;
    }

    m_renderer->drawText(x, curY, "BIOMA:", 1.5f, glm::vec4(0.85f, 0.90f, 0.95f, 1.0f));
    m_renderer->drawText(x + 75.0f, curY, biomeName, 1.5f, biomeColor);
    curY += 26.0f;

    m_renderer->drawText(x, curY, "DIMENSIONES: " + std::to_string(track.cols) + " x " + std::to_string(track.rows) + " CELDAS",
                         1.5f, glm::vec4(0.70f, 0.85f, 1.0f, 1.0f));
    curY += 28.0f;

    m_renderer->drawText(x, curY, "DESCRIPCION DEL TRAZADO:", 1.5f, glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));
    curY += 22.0f;

    for (const auto& line : wrapText(track.description, w - 10.0f, 1.4f)) {
        m_renderer->drawText(x + 4.0f, curY, line, 1.4f, glm::vec4(0.90f, 0.94f, 0.98f, 1.0f));
        curY += 20.0f;
    }
}

void ShowroomUI::renderBottomBar(float screenWidth, float screenHeight) {
    const float barH = 65.0f;
    const float barY = screenHeight - barH;
    const float mx = core::Input::getMouseX();
    const float my = core::Input::getMouseY();

    m_renderer->drawRect(0.0f, barY, screenWidth, barH, glm::vec4(0.04f, 0.06f, 0.10f, 0.96f));
    m_renderer->drawBorder(0.0f, barY, screenWidth, 2.0f, 2.0f, glm::vec4(0.25f, 0.35f, 0.55f, 1.0f));

    UIRect backRect{25.0f, barY + 11.0f, 230.0f, 42.0f};
    bool hoverBack = backRect.contains(mx, my);
    UIButton::draw(*m_renderer, backRect, m_btnBack, m_btnBackHover, "[ESC] VOLVER AL MENU", 1.6f, hoverBack,
                   hoverBack ? glm::vec4(1.0f, 0.9f, 0.9f, 1.0f) : glm::vec4(0.9f, 0.95f, 1.0f, 1.0f), 12.0f);

    UIRect startRect{screenWidth - 280.0f, barY + 11.0f, 255.0f, 42.0f};
    bool hoverStart = startRect.contains(mx, my);
    float pulse = (std::sin(m_animTimer * 5.0f) + 1.0f) * 0.5f;
    glm::vec4 startTint = hoverStart ? glm::vec4(1.0f, 1.0f, 0.85f, 1.0f)
                                     : glm::vec4(0.90f + 0.10f * pulse, 1.0f, 0.90f + 0.10f * pulse, 1.0f);
    UIButton::drawTinted(*m_renderer, startRect, m_btnStart, m_btnStartHover, "[ENTER] INICIAR CARRERA!", 1.6f, hoverStart,
                         glm::vec4(1.0f, 1.0f, 0.95f, 1.0f), startTint, 12.0f);

    float centerBoxX = backRect.x + backRect.w + 10.0f;
    float centerBoxW = startRect.x - centerBoxX - 10.0f;
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
        case core::SelectionCategory::DRIVER:  setDriverIndex(m_ctx->selectedDriverIndex - 1); break;
        case core::SelectionCategory::VEHICLE: setVehicleIndex(m_ctx->selectedVehicleIndex - 1); break;
        case core::SelectionCategory::TRACK:   setTrackIndex(m_ctx->selectedTrackIndex - 1); break;
    }
}

void ShowroomUI::selectNext() {
    if (!m_ctx) return;
    switch (getActiveCategory()) {
        case core::SelectionCategory::DRIVER:  setDriverIndex(m_ctx->selectedDriverIndex + 1); break;
        case core::SelectionCategory::VEHICLE: setVehicleIndex(m_ctx->selectedVehicleIndex + 1); break;
        case core::SelectionCategory::TRACK:   setTrackIndex(m_ctx->selectedTrackIndex + 1); break;
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
    m_ctx->selectDriver(((index % count) + count) % count);
}

void ShowroomUI::setVehicleIndex(int index) {
    if (!m_ctx || m_ctx->vehicles.empty()) return;
    int count = static_cast<int>(m_ctx->vehicles.size());
    m_ctx->selectVehicle(((index % count) + count) % count);
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

} // namespace ui
