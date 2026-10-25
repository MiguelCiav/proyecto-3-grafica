#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "renderer/Shader.hpp"
#include "renderer/Texture.hpp"
#include "ui/UIRenderer.hpp"

namespace ui {

struct HudVertex {
    glm::vec2 pos;
    glm::vec2 uv;
    glm::vec4 color;
};

struct LapTimingData {
    int currentLap{1};
    int totalLaps{3};
    float currentLapTime{0.0f};
    float bestLapTime{0.0f};
    float totalRaceTime{0.0f};
    float lapNoticeTimer{0.0f};
    float lastLapTime{0.0f};
    bool isNewRecord{false};
};

struct TelemetryData {
    float speedKmH{0.0f};
    float maxSpeedKmH{80.0f};
    bool isBoosting{false};
    bool isBraking{false};
    bool isReversing{false};
    bool isSkidding{false};
    bool isOffRoad{false};
};

/**
 * @brief Sistema de interfaz de usuario HUD para carreras (Zero Blender).
 * Proporciona:
 * 1. Velocimetro digital / Tacometro sci-fi con Kenney Space Expansion Pack:
 *    - Marco de fijacion con tornillos blade (button_square_header_blade_rectangle_screws).
 *    - Contenedor de barra de velocidad con relleno progresivo dinamico (bar_round_gloss_large).
 *    - Alerta pulsante de TURBO BOOST (bar_square_gloss_large).
 *    - Indicadores digitales LED de velocidad KM/H, marchas y tacometro.
 * 2. Cronometro de precision de vuelta (MM:SS.cc) sci-fi:
 *    - Marco blade sci-fi (button_square_header_blade_rectangle).
 *    - Insignia de vuelta (button_square_header_small_square).
 *    - Tiempos de vuelta actual, mejor vuelta y total en formato digital 7-segmentos.
 * 3. Notificaciones flotantes de vuelta completada y nuevo record.
 */
class RacingHud {
public:
    RacingHud();
    ~RacingHud();

    RacingHud(const RacingHud&) = delete;
    RacingHud& operator=(const RacingHud&) = delete;

    RacingHud(RacingHud&& other) noexcept;
    RacingHud& operator=(RacingHud&& other) noexcept;

    bool init(const std::string& vertPath = "shaders/ui.vert",
              const std::string& fragPath = "shaders/ui.frag");
    void render(float screenWidth, float screenHeight,
                const TelemetryData& telemetry,
                const LapTimingData& timing);

    bool isInitialized() const { return m_initialized; }

    // Consultas de estado de texturas cargadas
    bool areTexturesLoaded() const;

private:
    void flush();
    void addQuad(float x, float y, float w, float h, const glm::vec4& color);
    void addBorder(float x, float y, float w, float h, float thickness, const glm::vec4& color);

    // Primitivas de texto y digitos digitales
    void drawText(float x, float y, const std::string& text, float scale, const glm::vec4& color);
    void draw7SegmentDigit(float x, float y, int digit, float w, float h, float thickness,
                           const glm::vec4& activeColor, const glm::vec4& inactiveColor);
    void draw7SegmentTime(float x, float y, float timeSeconds, float scale, const glm::vec4& color);
    void drawColon(float x, float y, float scale, const glm::vec4& color);
    void drawDot(float x, float y, float scale, const glm::vec4& color);

    // Paneles HUD individuales
    void renderSpeedometer(float screenWidth, float screenHeight, const TelemetryData& t, const LapTimingData& timing);
    void renderLapTimer(float screenWidth, float screenHeight, const LapTimingData& timing);
    void renderLapNotice(float screenWidth, float screenHeight, const LapTimingData& timing);

    // Renderizador 2D por lotes
    UIRenderer m_uiRenderer;

    // Texturas de Kenney Space Expansion Pack
    renderer::Texture m_texSpeedGaugeFrame;      // button_square_header_blade_rectangle_screws.png
    renderer::Texture m_texSpeedBarContainer;    // Fallback single
    renderer::Texture m_texSpeedBarFillYellow;   // Fallback single
    renderer::Texture m_texSpeedBarFillRed;      // Fallback single

    // Piezas modulares 3-Slice para la barra de velocidad
    renderer::Texture m_texSpeedBarContainerL;   // bar_round_gloss_large_l.png (Blue)
    renderer::Texture m_texSpeedBarContainerM;   // bar_round_gloss_large_m.png (Blue)
    renderer::Texture m_texSpeedBarContainerR;   // bar_round_gloss_large_r.png (Blue)

    renderer::Texture m_texSpeedBarFillYellowL;  // bar_round_gloss_large_l.png (Yellow)
    renderer::Texture m_texSpeedBarFillYellowM;  // bar_round_gloss_large_m.png (Yellow)
    renderer::Texture m_texSpeedBarFillYellowR;  // bar_round_gloss_large_r.png (Yellow)

    renderer::Texture m_texSpeedBarFillRedL;     // bar_round_gloss_large_l.png (Red)
    renderer::Texture m_texSpeedBarFillRedM;     // bar_round_gloss_large_m.png (Red)
    renderer::Texture m_texSpeedBarFillRedR;     // bar_round_gloss_large_r.png (Red)

    renderer::Texture m_texLapTimerFrame;        // Blue button_square_header_blade_rectangle.png
    renderer::Texture m_texLapBadge;             // Yellow button_square_header_small_square.png
    renderer::Texture m_texTurboBoostBanner;     // Red bar_square_gloss_large.png

    bool m_initialized{false};
    float m_pulseTimer{0.0f};
};

} // namespace ui
