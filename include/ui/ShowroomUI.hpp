#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "renderer/Texture.hpp"
#include "ui/UIRenderer.hpp"
#include "core/GameContext.hpp"

namespace ui {

/**
 * @brief Interfaz gráfica interactiva y profesional 2D/3D para el Showroom de Mario Kart CGI.
 * Proporciona:
 * 1. Barra superior de pestañas 9-slice: [1. CORREDOR], [2. VEHÍCULO], [3. CIRCUITO] con realce dorado.
 * 2. Panel lateral 9-slice con estadísticas tipo Mario Kart (barras progresivas segmentadas),
 *    perfil de personaje, rasgos y ficha técnica de pista con estrellas de dificultad.
 * 3. Selector de carrusel central con flechas interactivas (< y >) y banner de nombre.
 * 4. Barra inferior de navegación con accesos directos y botones de acción.
 */
class ShowroomUI {
public:
    ShowroomUI();
    ~ShowroomUI();

    ShowroomUI(const ShowroomUI&) = delete;
    ShowroomUI& operator=(const ShowroomUI&) = delete;

    ShowroomUI(ShowroomUI&& other) noexcept;
    ShowroomUI& operator=(ShowroomUI&& other) noexcept;

    // Inicializa texturas del Kenney UI Pack y el renderizador 2D
    bool init(core::GameContext* ctx, UIRenderer* externalRenderer = nullptr);
    void destroy();

    void update(float deltaTime);
    void handleInput(float deltaTime);
    void render(float screenWidth, float screenHeight);

    // Navegación de categorías
    void setActiveCategory(core::SelectionCategory category);
    [[nodiscard]] core::SelectionCategory getActiveCategory() const;

    // Navegación del carrusel de elementos
    void selectPrevious();
    void selectNext();

    // Acciones de navegación de juego
    void confirmSelection();
    void returnToMenu();

    [[nodiscard]] bool isInitialized() const { return m_initialized; }
    [[nodiscard]] bool areTexturesLoaded() const;

    // Índices de selección actuales
    [[nodiscard]] int getSelectedDriverIndex() const;
    [[nodiscard]] int getSelectedVehicleIndex() const;
    [[nodiscard]] int getSelectedTrackIndex() const;

    // Modificadores directos de selección
    void setDriverIndex(int index);
    void setVehicleIndex(int index);
    void setTrackIndex(int index);

private:
    void renderTopTabs(float screenWidth, float screenHeight);
    void renderSidePanel(float screenWidth, float screenHeight);
    void renderCenterCarousel(float screenWidth, float screenHeight);
    void renderBottomBar(float screenWidth, float screenHeight);

    // Vistas especializadas dentro de la tarjeta lateral
    void renderDriverInfo(float x, float y, float w, float h);
    void renderVehicleInfo(float x, float y, float w, float h);
    void renderTrackInfo(float x, float y, float w, float h);

    core::GameContext* m_ctx{nullptr};
    UIRenderer* m_renderer{nullptr};
    std::unique_ptr<UIRenderer> m_ownedRenderer;

    // Texturas de Kenney UI Pack
    renderer::Texture m_tabIdle;
    renderer::Texture m_tabActive;
    renderer::Texture m_tabHover;
    renderer::Texture m_panelFrame;
    renderer::Texture m_panelInner;
    renderer::Texture m_arrowLeft;
    renderer::Texture m_arrowRight;
    renderer::Texture m_arrowIndicator;
    renderer::Texture m_star;
    renderer::Texture m_starEmpty;
    renderer::Texture m_btnStart;
    renderer::Texture m_btnStartHover;
    renderer::Texture m_btnBack;
    renderer::Texture m_btnBackHover;
    renderer::Texture m_barBg;
    renderer::Texture m_barFill;
    renderer::Texture m_barFillGreen;

    float m_animTimer{0.0f};
    bool m_initialized{false};
};

} // namespace ui
