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

    // Barra de estadística 3-slice progresiva con ticks estilo Mario Kart
    void renderStatBar(float x, float y, float w, float h,
                       const std::string& label, const std::string& valueStr,
                       float ratio, const glm::vec4& fillColor);

    [[nodiscard]] std::vector<std::string> wrapText(const std::string& text, float maxWidth, float scale) const;
    [[nodiscard]] std::string sanitizeText(const std::string& text) const;

    core::GameContext* m_ctx{nullptr};
    UIRenderer* m_renderer{nullptr};
    std::unique_ptr<UIRenderer> m_ownedRenderer;

    // Texturas de Kenney UI Pack
    renderer::Texture m_tabIdle;          // Blue/Double/button_rectangle_depth_flat.png
    renderer::Texture m_tabActive;        // Yellow/Double/button_rectangle_depth_gradient.png
    renderer::Texture m_tabHover;         // Yellow/Double/button_rectangle_depth_flat.png
    renderer::Texture m_panelFrame;       // Blue/Double/button_square_border.png
    renderer::Texture m_panelInner;       // Grey/Double/button_square_depth_flat.png
    renderer::Texture m_arrowLeft;        // Yellow/Double/arrow_decorative_w.png
    renderer::Texture m_arrowRight;       // Yellow/Double/arrow_decorative_e.png
    renderer::Texture m_arrowIndicator;   // Yellow/Double/arrow_decorative_s.png
    renderer::Texture m_star;             // Yellow/Double/star.png
    renderer::Texture m_starEmpty;        // Grey/Double/star.png
    renderer::Texture m_btnStart;         // Green/Double/button_rectangle_depth_border.png
    renderer::Texture m_btnStartHover;    // Green/Double/button_rectangle_depth_gradient.png
    renderer::Texture m_btnBack;          // Grey/Double/button_rectangle_depth_flat.png
    renderer::Texture m_btnBackHover;     // Red/Double/button_rectangle_depth_flat.png
    renderer::Texture m_barBg;            // Grey/Double/slide_horizontal_grey_section_wide.png
    renderer::Texture m_barFill;          // Yellow/Double/slide_horizontal_color_section_wide.png
    renderer::Texture m_barFillGreen;     // Green/Double/slide_horizontal_color_section_wide.png

    float m_animTimer{0.0f};
    bool m_initialized{false};
};

} // namespace ui
