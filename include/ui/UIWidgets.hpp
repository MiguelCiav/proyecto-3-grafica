#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "renderer/Texture.hpp"
#include "ui/UIRenderer.hpp"

namespace ui {

/**
 * @brief Rectangulo 2D en espacio de pantalla con soporte de hit-testing puntual.
 */
struct UIRect {
    float x{0.0f}, y{0.0f}, w{0.0f}, h{0.0f};

    [[nodiscard]] bool contains(float px, float py) const;
};

/**
 * @brief Widget estatico para renderizar botones 9-slice, iconos interactivos e indicadores animados.
 */
struct UIButton {
    static void draw(UIRenderer& renderer,
                     const UIRect& rect,
                     const renderer::Texture& texIdle,
                     const renderer::Texture& texHover,
                     const std::string& label,
                     float textScale,
                     bool isHovered,
                     const glm::vec4& textColor = glm::vec4(1.0f),
                     float cornerSize = 16.0f);

    static void drawTinted(UIRenderer& renderer,
                           const UIRect& rect,
                           const renderer::Texture& texIdle,
                           const renderer::Texture& texHover,
                           const std::string& label,
                           float textScale,
                           bool isHovered,
                           const glm::vec4& textColor,
                           const glm::vec4& bgTint,
                           float cornerSize = 16.0f);

    static void drawIconButton(UIRenderer& renderer,
                               const UIRect& rect,
                               const renderer::Texture& iconTex,
                               bool isHovered,
                               const glm::vec4& idleTint = glm::vec4(1.0f, 0.88f, 0.25f, 0.9f),
                               const glm::vec4& hoverTint = glm::vec4(1.0f, 1.0f, 0.4f, 1.0f),
                               float hoverScale = 1.15f);

    static void drawArrowIndicator(UIRenderer& renderer,
                                   const renderer::Texture& arrowTex,
                                   float x,
                                   float y,
                                   float size,
                                   float animTimer,
                                   float bounceAmp = 6.0f);
};

/**
 * @brief Widget estatico para dibujar paneles enmarcados 9-slice, cajas de cabecera y tarjetas.
 */
struct UIPanel {
    static void drawFramedPanel(UIRenderer& renderer,
                                const UIRect& rect,
                                const renderer::Texture& frameTex,
                                float cornerSize = 24.0f,
                                const glm::vec4& bgTint = glm::vec4(0.05f, 0.08f, 0.14f, 0.94f),
                                const glm::vec4& borderTint = glm::vec4(0.30f, 0.60f, 0.95f, 0.80f));

    static void drawBannerBox(UIRenderer& renderer,
                              const UIRect& rect,
                              const renderer::Texture& innerTex,
                              const std::string& text,
                              float textScale,
                              const glm::vec4& textColor,
                              const glm::vec4& borderColor,
                              const glm::vec4& innerTint = glm::vec4(0.12f, 0.18f, 0.28f, 0.95f),
                              float cornerSize = 8.0f);
};

/**
 * @brief Widget estatico para barras de estadisticas segmentadas estilo Mario Kart.
 */
struct UIProgressBar {
    static void drawStatBar(UIRenderer& renderer,
                            const UIRect& rect,
                            const std::string& label,
                            const std::string& valueStr,
                            float ratio,
                            const glm::vec4& fillColor,
                            const renderer::Texture& barBgL,
                            const renderer::Texture& barBgM,
                            const renderer::Texture& barBgR,
                            const renderer::Texture& barFillL,
                            const renderer::Texture& barFillM,
                            const renderer::Texture& barFillR);

    static void drawStatBar(UIRenderer& renderer,
                            const UIRect& rect,
                            const std::string& label,
                            const std::string& valueStr,
                            float ratio,
                            const glm::vec4& fillColor,
                            const renderer::Texture& barBg,
                            const renderer::Texture& barFill);
};

/**
 * @brief Widget estatico para barra horizontal de pestanas 9-slice con hit-testing.
 */
struct UITabBar {
    [[nodiscard]] static int hitTest(float startX,
                                     float y,
                                     float tabW,
                                     float tabH,
                                     float spacing,
                                     int tabCount,
                                     float mouseX,
                                     float mouseY);

    static int draw(UIRenderer& renderer,
                    float startX,
                    float y,
                    float tabW,
                    float tabH,
                    float spacing,
                    const std::vector<std::string>& labels,
                    int activeIndex,
                    const renderer::Texture& texIdle,
                    const renderer::Texture& texActive,
                    const renderer::Texture& texHover,
                    const renderer::Texture* arrowDownTex = nullptr,
                    float mouseX = -1.0f,
                    float mouseY = -1.0f,
                    bool mouseClicked = false);
};

} // namespace ui
