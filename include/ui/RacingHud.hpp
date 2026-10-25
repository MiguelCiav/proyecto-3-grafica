#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "renderer/Shader.hpp"

namespace ui {

struct HudVertex {
    glm::vec2 pos;
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
 * 1. Velocimetro digital / Tacometro con barra de RPM LED dinamica y alertas.
 * 2. Cronometro de precision de vuelta (MM:SS.cc), record de vuelta y contador de vueltas.
 * 3. Renderizado procedural en lote mediante OpenGL Core Profile (una sola pasada 2D).
 */
class RacingHud {
public:
    RacingHud();
    ~RacingHud();

    RacingHud(const RacingHud&) = delete;
    RacingHud& operator=(const RacingHud&) = delete;

    bool init(const std::string& vertPath = "shaders/hud.vert",
              const std::string& fragPath = "shaders/hud.frag");
    void render(float screenWidth, float screenHeight,
                const TelemetryData& telemetry,
                const LapTimingData& timing);

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
    void renderSpeedometer(float screenWidth, float screenHeight, const TelemetryData& t);
    void renderLapTimer(float screenWidth, float screenHeight, const LapTimingData& timing);
    void renderLapNotice(float screenWidth, float screenHeight, const LapTimingData& timing);

    unsigned int m_vao{0};
    unsigned int m_vbo{0};
    renderer::Shader m_shader;
    bool m_initialized{false};

    std::vector<HudVertex> m_vertices;
    glm::mat4 m_orthoProj{1.0f};
};

} // namespace ui
