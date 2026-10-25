#include "ui/RacingHud.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>

namespace ui {

namespace {

// Helper para localizar rutas de assets independientemente de si el ejecutable
// corre desde la raiz del repositorio o desde la carpeta de construccion build/
std::string resolveAssetPath(const std::string& relativePath) {
    std::ifstream f1(relativePath);
    if (f1.good()) return relativePath;

    std::string parentRelative = "../" + relativePath;
    std::ifstream f2(parentRelative);
    if (f2.good()) return parentRelative;

    return relativePath;
}

} // namespace

RacingHud::RacingHud() = default;

RacingHud::~RacingHud() = default;

RacingHud::RacingHud(RacingHud&& other) noexcept
    : m_uiRenderer(std::move(other.m_uiRenderer))
    , m_texSpeedGaugeFrame(std::move(other.m_texSpeedGaugeFrame))
    , m_texSpeedBarContainer(std::move(other.m_texSpeedBarContainer))
    , m_texSpeedBarFillYellow(std::move(other.m_texSpeedBarFillYellow))
    , m_texSpeedBarFillRed(std::move(other.m_texSpeedBarFillRed))
    , m_texSpeedBarContainerL(std::move(other.m_texSpeedBarContainerL))
    , m_texSpeedBarContainerM(std::move(other.m_texSpeedBarContainerM))
    , m_texSpeedBarContainerR(std::move(other.m_texSpeedBarContainerR))
    , m_texSpeedBarFillYellowL(std::move(other.m_texSpeedBarFillYellowL))
    , m_texSpeedBarFillYellowM(std::move(other.m_texSpeedBarFillYellowM))
    , m_texSpeedBarFillYellowR(std::move(other.m_texSpeedBarFillYellowR))
    , m_texSpeedBarFillRedL(std::move(other.m_texSpeedBarFillRedL))
    , m_texSpeedBarFillRedM(std::move(other.m_texSpeedBarFillRedM))
    , m_texSpeedBarFillRedR(std::move(other.m_texSpeedBarFillRedR))
    , m_texLapTimerFrame(std::move(other.m_texLapTimerFrame))
    , m_texLapBadge(std::move(other.m_texLapBadge))
    , m_texTurboBoostBanner(std::move(other.m_texTurboBoostBanner))
    , m_initialized(other.m_initialized)
    , m_pulseTimer(other.m_pulseTimer)
{
    other.m_initialized = false;
    other.m_pulseTimer = 0.0f;
}

RacingHud& RacingHud::operator=(RacingHud&& other) noexcept {
    if (this != &other) {
        m_uiRenderer = std::move(other.m_uiRenderer);
        m_texSpeedGaugeFrame = std::move(other.m_texSpeedGaugeFrame);
        m_texSpeedBarContainer = std::move(other.m_texSpeedBarContainer);
        m_texSpeedBarFillYellow = std::move(other.m_texSpeedBarFillYellow);
        m_texSpeedBarFillRed = std::move(other.m_texSpeedBarFillRed);
        m_texSpeedBarContainerL = std::move(other.m_texSpeedBarContainerL);
        m_texSpeedBarContainerM = std::move(other.m_texSpeedBarContainerM);
        m_texSpeedBarContainerR = std::move(other.m_texSpeedBarContainerR);
        m_texSpeedBarFillYellowL = std::move(other.m_texSpeedBarFillYellowL);
        m_texSpeedBarFillYellowM = std::move(other.m_texSpeedBarFillYellowM);
        m_texSpeedBarFillYellowR = std::move(other.m_texSpeedBarFillYellowR);
        m_texSpeedBarFillRedL = std::move(other.m_texSpeedBarFillRedL);
        m_texSpeedBarFillRedM = std::move(other.m_texSpeedBarFillRedM);
        m_texSpeedBarFillRedR = std::move(other.m_texSpeedBarFillRedR);
        m_texLapTimerFrame = std::move(other.m_texLapTimerFrame);
        m_texLapBadge = std::move(other.m_texLapBadge);
        m_texTurboBoostBanner = std::move(other.m_texTurboBoostBanner);
        m_initialized = other.m_initialized;
        m_pulseTimer = other.m_pulseTimer;

        other.m_initialized = false;
        other.m_pulseTimer = 0.0f;
    }
    return *this;
}

bool RacingHud::init(const std::string& vertPath, const std::string& fragPath) {
    if (m_initialized) return true;

    // Inicializar renderizador 2D por lotes
    bool shaderOk = m_uiRenderer.init(vertPath, fragPath);
    if (!shaderOk && vertPath != "shaders/ui.vert") {
        shaderOk = m_uiRenderer.init("shaders/ui.vert", "shaders/ui.frag");
    }
    if (!shaderOk && vertPath != "shaders/hud.vert") {
        shaderOk = m_uiRenderer.init("shaders/hud.vert", "shaders/hud.frag");
    }
    if (!shaderOk) {
        std::cerr << "[ERROR] [RacingHud] Fallo al compilar shaders para UIRenderer ("
                  << vertPath << ", " << fragPath << ")" << std::endl;
        return false;
    }

    // Cargar texturas de Kenney Space Expansion Pack (sin volteo vertical para UI 2D)
    // Se intenta primero la versión 2x de alta resolución (Double) con fallback a Default
    const std::string basePath = "assets/kenney_ui-pack-space-expansion/PNG/";

    auto loadTexture = [&](renderer::Texture& tex, const std::string& relPath) -> bool {
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

    // Marcos principales
    loadTexture(m_texSpeedGaugeFrame, "Blue/Default/button_square_header_blade_rectangle_screws.png");
    loadTexture(m_texLapTimerFrame, "Blue/Default/button_square_header_blade_rectangle.png");
    loadTexture(m_texLapBadge, "Yellow/Default/button_square_header_small_square.png");
    loadTexture(m_texTurboBoostBanner, "Red/Default/bar_square_gloss_large.png");

    // Piezas modulares 3-Slice para la barra de velocidad (Contenedor Azul)
    loadTexture(m_texSpeedBarContainerL, "Blue/Default/bar_round_gloss_large_l.png");
    loadTexture(m_texSpeedBarContainerM, "Blue/Default/bar_round_gloss_large_m.png");
    loadTexture(m_texSpeedBarContainerR, "Blue/Default/bar_round_gloss_large_r.png");

    // Piezas modulares 3-Slice para llenado nominal (Amarillo brillante)
    loadTexture(m_texSpeedBarFillYellowL, "Yellow/Default/bar_round_gloss_large_l.png");
    loadTexture(m_texSpeedBarFillYellowM, "Yellow/Default/bar_round_gloss_large_m.png");
    loadTexture(m_texSpeedBarFillYellowR, "Yellow/Default/bar_round_gloss_large_r.png");

    // Piezas modulares 3-Slice para llenado en Turbo/Boost (Rojo fuego)
    loadTexture(m_texSpeedBarFillRedL, "Red/Default/bar_round_gloss_large_l.png");
    loadTexture(m_texSpeedBarFillRedM, "Red/Default/bar_round_gloss_large_m.png");
    loadTexture(m_texSpeedBarFillRedR, "Red/Default/bar_round_gloss_large_r.png");

    // Fallbacks monocatenarios
    loadTexture(m_texSpeedBarContainer, "Blue/Default/bar_round_gloss_large.png");
    loadTexture(m_texSpeedBarFillYellow, "Yellow/Default/bar_round_gloss_large.png");
    loadTexture(m_texSpeedBarFillRed, "Red/Default/bar_round_gloss_large.png");

    m_initialized = true;
    std::cout << "[OK] [RacingHud] HUD de Carreras (Kenney Space Expansion Pack - Modular 3-Slice & 9-Slice) inicializado exitosamente."
              << std::endl;
    return true;
}

bool RacingHud::areTexturesLoaded() const {
    return m_texSpeedGaugeFrame.isLoaded() &&
           m_texSpeedBarContainerL.isLoaded() &&
           m_texSpeedBarContainerM.isLoaded() &&
           m_texSpeedBarContainerR.isLoaded() &&
           m_texSpeedBarFillYellowL.isLoaded() &&
           m_texSpeedBarFillYellowM.isLoaded() &&
           m_texSpeedBarFillYellowR.isLoaded() &&
           m_texLapTimerFrame.isLoaded() &&
           m_texLapBadge.isLoaded() &&
           m_texTurboBoostBanner.isLoaded();
}

void RacingHud::flush() {
    // Implementado directamente por m_uiRenderer
}

void RacingHud::addQuad(float x, float y, float w, float h, const glm::vec4& color) {
    m_uiRenderer.drawRect(x, y, w, h, color);
}

void RacingHud::addBorder(float x, float y, float w, float h, float thickness, const glm::vec4& color) {
    m_uiRenderer.drawBorder(x, y, w, h, thickness, color);
}

void RacingHud::drawText(float x, float y, const std::string& text, float scale, const glm::vec4& color) {
    m_uiRenderer.drawText(x, y, text, scale, color);
}

void RacingHud::draw7SegmentDigit(float x, float y, int digit, float w, float h, float thickness,
                                  const glm::vec4& activeColor, const glm::vec4& inactiveColor) {
    // Mascara de 7 segmentos: a, b, c, d, e, f, g
    static const uint8_t SEGMENTS[10] = {
        0b00111111, // 0: a, b, c, d, e, f
        0b00000110, // 1: b, c
        0b01011011, // 2: a, b, d, e, g
        0b01001111, // 3: a, b, c, d, g
        0b01100110, // 4: b, c, f, g
        0b01101101, // 5: a, c, d, f, g
        0b01111101, // 6: a, c, d, e, f, g
        0b00000111, // 7: a, b, c
        0b01111111, // 8: a, b, c, d, e, f, g
        0b01101111  // 9: a, b, c, d, f, g
    };

    uint8_t mask = (digit >= 0 && digit <= 9) ? SEGMENTS[digit] : 0b01000000; // guion para negativos

    float halfH = h * 0.5f;
    float gap = thickness * 0.35f;

    auto segColor = [&](int bit) {
        return (mask & (1 << bit)) ? activeColor : inactiveColor;
    };

    // Segmento A: horizontal superior
    m_uiRenderer.drawRect(x + thickness + gap, y, w - 2.0f * (thickness + gap), thickness, segColor(0));
    // Segmento B: vertical superior derecho
    m_uiRenderer.drawRect(x + w - thickness, y + thickness + gap, thickness, halfH - thickness - 2.0f * gap, segColor(1));
    // Segmento C: vertical inferior derecho
    m_uiRenderer.drawRect(x + w - thickness, y + halfH + gap, thickness, halfH - thickness - 2.0f * gap, segColor(2));
    // Segmento D: horizontal inferior
    m_uiRenderer.drawRect(x + thickness + gap, y + h - thickness, w - 2.0f * (thickness + gap), thickness, segColor(3));
    // Segmento E: vertical inferior izquierdo
    m_uiRenderer.drawRect(x, y + halfH + gap, thickness, halfH - thickness - 2.0f * gap, segColor(4));
    // Segmento F: vertical superior izquierdo
    m_uiRenderer.drawRect(x, y + thickness + gap, thickness, halfH - thickness - 2.0f * gap, segColor(5));
    // Segmento G: horizontal central
    m_uiRenderer.drawRect(x + thickness + gap, y + halfH - thickness * 0.5f, w - 2.0f * (thickness + gap), thickness, segColor(6));
}

void RacingHud::drawColon(float x, float y, float scale, const glm::vec4& color) {
    float size = 3.0f * scale;
    m_uiRenderer.drawRect(x, y + 6.0f * scale, size, size, color);
    m_uiRenderer.drawRect(x, y + 16.0f * scale, size, size, color);
}

void RacingHud::drawDot(float x, float y, float scale, const glm::vec4& color) {
    float size = 3.0f * scale;
    m_uiRenderer.drawRect(x, y + 19.0f * scale, size, size, color);
}

void RacingHud::draw7SegmentTime(float x, float y, float timeSeconds, float scale, const glm::vec4& color) {
    glm::vec4 inactiveColor(color.r, color.g, color.b, 0.08f);

    float digitW = 14.0f * scale;
    float digitH = 26.0f * scale;
    float thick = 3.0f * scale;
    float space = 4.0f * scale;

    float curX = x;

    if (timeSeconds < 0.0f) {
        // No hay tiempo establecido: mostrar -- : -- . --
        for (int i = 0; i < 6; ++i) {
            draw7SegmentDigit(curX, y, -1, digitW, digitH, thick, color, inactiveColor);
            curX += digitW + space;
            if (i == 1) {
                drawColon(curX, y, scale, color);
                curX += 7.0f * scale;
            } else if (i == 3) {
                drawDot(curX, y, scale, color);
                curX += 7.0f * scale;
            }
        }
        return;
    }

    int totalHundredths = static_cast<int>(timeSeconds * 100.0f);
    int minutes = (totalHundredths / 6000) % 100;
    int seconds = (totalHundredths / 100) % 60;
    int hundredths = totalHundredths % 100;

    int d[6] = {
        minutes / 10, minutes % 10,
        seconds / 10, seconds % 10,
        hundredths / 10, hundredths % 10
    };

    for (int i = 0; i < 6; ++i) {
        draw7SegmentDigit(curX, y, d[i], digitW, digitH, thick, color, inactiveColor);
        curX += digitW + space;
        if (i == 1) {
            drawColon(curX, y, scale, color);
            curX += 7.0f * scale;
        } else if (i == 3) {
            drawDot(curX, y, scale, color);
            curX += 7.0f * scale;
        }
    }
}

void RacingHud::renderSpeedometer(float screenWidth, float screenHeight, const TelemetryData& t, const LapTimingData& timing) {
    float w = 320.0f;
    float h = 160.0f;
    float x = screenWidth - w - 24.0f;
    float y = screenHeight - h - 24.0f;

    // 1. Capa de fondo oscura ahumada para maximo contraste
    m_uiRenderer.drawRect(x + 4.0f, y + 4.0f, w - 8.0f, h - 8.0f, glm::vec4(0.02f, 0.04f, 0.08f, 0.90f));

    // 2. Marco sci-fi con tornillos de fijacion mediante 9-Slice para esquinas no deformadas
    glm::vec4 frameTint(1.0f, 1.0f, 1.0f, 0.95f);
    if (t.isBoosting) {
        frameTint = glm::vec4(1.0f, 0.65f, 0.25f, 0.98f);
    } else if (t.isSkidding) {
        frameTint = glm::vec4(1.0f, 0.88f, 0.35f, 0.95f);
    }
    m_uiRenderer.drawNineSlice(m_texSpeedGaugeFrame, x, y, w, h, 28.0f, 28.0f, frameTint);

    // Panel interior oscuro del cockpit
    float innerX = x + 16.0f;
    float innerY = y + 16.0f;
    float innerW = w - 32.0f;
    float innerH = h - 32.0f;
    m_uiRenderer.drawRect(innerX, innerY, innerW, innerH, glm::vec4(0.01f, 0.02f, 0.05f, 0.78f));
    m_uiRenderer.drawBorder(innerX, innerY, innerW, innerH, 1.5f,
                            t.isBoosting ? glm::vec4(1.0f, 0.55f, 0.10f, 0.70f)
                                         : glm::vec4(0.0f, 0.75f, 0.95f, 0.45f));

    // 3. Contenedor de barra de velocidad modular 3-Slice con relleno progresivo dinamico
    float barX = innerX + 12.0f;
    float barY = innerY + 10.0f;
    float barW = innerW - 24.0f;
    float barH = 18.0f;

    // Contenedor azul traslucido modular 3-Slice
    m_uiRenderer.drawThreeSliceHorizontal(m_texSpeedBarContainerL, m_texSpeedBarContainerM, m_texSpeedBarContainerR,
                                          barX, barY, barW, barH, 12.0f, glm::vec4(0.40f, 0.65f, 0.95f, 0.50f));

    // Relleno progresivo dinamico modular 3-Slice basado en speedKmH / maxSpeedKmH
    float fillRatio = std::clamp(t.speedKmH / std::max(t.maxSpeedKmH, 1.0f), 0.0f, 1.0f);
    const auto& fillL = t.isBoosting ? m_texSpeedBarFillRedL : m_texSpeedBarFillYellowL;
    const auto& fillM = t.isBoosting ? m_texSpeedBarFillRedM : m_texSpeedBarFillYellowM;
    const auto& fillR = t.isBoosting ? m_texSpeedBarFillRedR : m_texSpeedBarFillYellowR;
    m_uiRenderer.drawThreeSliceProgress(fillL, fillM, fillR, barX, barY, barW, barH, fillRatio, 12.0f, glm::vec4(1.0f));

    // 4. Indicador tacometro LED interactivo (18 segmentos)
    int segCount = 18;
    float segGap = 2.0f;
    float segW = (barW - (segCount - 1) * segGap) / segCount;
    int activeSegs = static_cast<int>(std::round(fillRatio * segCount));

    for (int i = 0; i < segCount; ++i) {
        glm::vec4 segCol;
        if (i < 10) {
            segCol = glm::vec4(0.0f, 0.95f, 0.55f, 1.0f); // Verde neon
        } else if (i < 14) {
            segCol = glm::vec4(1.0f, 0.80f, 0.0f, 1.0f);  // Ambar
        } else {
            segCol = glm::vec4(1.0f, 0.15f, 0.25f, 1.0f);  // Rojo corte
        }

        if (i >= activeSegs) {
            segCol = glm::vec4(segCol.r * 0.2f, segCol.g * 0.2f, segCol.b * 0.2f, 0.15f);
        }
        m_uiRenderer.drawRect(barX + i * (segW + segGap), barY + barH + 3.0f, segW, 4.0f, segCol);
    }

    // 5. Numero de velocidad digital (7 segmentos)
    int spd = std::clamp(static_cast<int>(std::round(t.speedKmH)), 0, 999);
    int d0 = (spd / 100) % 10;
    int d1 = (spd / 10) % 10;
    int d2 = spd % 10;

    glm::vec4 digitColor = glm::vec4(0.92f, 0.98f, 1.0f, 1.0f);
    if (t.isBoosting) {
        digitColor = glm::vec4(1.0f, 0.88f, 0.35f, 1.0f);
    }
    glm::vec4 inactiveDigit(digitColor.r, digitColor.g, digitColor.b, 0.07f);

    float digW = 20.0f;
    float digH = 40.0f;
    float digThick = 4.2f;
    float digGap = 7.0f;
    float digStartX = innerX + 18.0f;
    float digStartY = innerY + 38.0f;

    // Primer digito (centenas)
    draw7SegmentDigit(digStartX, digStartY, (spd >= 100) ? d0 : 0, digW, digH, digThick,
                      (spd >= 100) ? digitColor : inactiveDigit, inactiveDigit);
    // Segundo digito (decenas)
    draw7SegmentDigit(digStartX + digW + digGap, digStartY, (spd >= 10) ? d1 : 0, digW, digH, digThick,
                      (spd >= 10) ? digitColor : inactiveDigit, inactiveDigit);
    // Tercer digito (unidades)
    draw7SegmentDigit(digStartX + 2.0f * (digW + digGap), digStartY, d2, digW, digH, digThick,
                      digitColor, inactiveDigit);

    // Unidad "KM/H"
    m_uiRenderer.drawText(digStartX + 3.0f * (digW + digGap) + 10.0f, digStartY + 14.0f, "KM/H", 2.0f,
                          glm::vec4(0.0f, 0.85f, 1.0f, 0.95f));

    // 6. Selector de Marcha [ D ] / [ R ] e insignias de estado
    float badgeY = innerY + 86.0f;

    // Marcha
    if (t.isReversing) {
        m_uiRenderer.drawRect(innerX + 18.0f, badgeY, 32.0f, 22.0f, glm::vec4(0.85f, 0.15f, 0.15f, 0.85f));
        m_uiRenderer.drawText(innerX + 27.0f, badgeY + 4.0f, "R", 2.0f, glm::vec4(1.0f));
    } else {
        m_uiRenderer.drawRect(innerX + 18.0f, badgeY, 32.0f, 22.0f, glm::vec4(0.0f, 0.50f, 0.75f, 0.85f));
        m_uiRenderer.drawText(innerX + 27.0f, badgeY + 4.0f, "D", 2.0f, glm::vec4(1.0f));
    }

    // Insignia TURBO
    if (t.isBoosting) {
        m_uiRenderer.drawRect(innerX + 58.0f, badgeY, 68.0f, 22.0f, glm::vec4(1.0f, 0.45f, 0.0f, 0.90f));
        m_uiRenderer.drawText(innerX + 64.0f, badgeY + 4.0f, "TURBO", 1.8f, glm::vec4(1.0f));
    }

    // Insignia DERRAPE / OFFROAD
    if (t.isSkidding) {
        m_uiRenderer.drawRect(innerX + 134.0f, badgeY, 68.0f, 22.0f, glm::vec4(0.95f, 0.75f, 0.0f, 0.90f));
        m_uiRenderer.drawText(innerX + 140.0f, badgeY + 4.0f, "DRIFT", 1.8f, glm::vec4(0.05f, 0.05f, 0.05f, 1.0f));
    } else if (t.isOffRoad) {
        m_uiRenderer.drawRect(innerX + 134.0f, badgeY, 82.0f, 22.0f, glm::vec4(0.65f, 0.35f, 0.15f, 0.85f));
        m_uiRenderer.drawText(innerX + 138.0f, badgeY + 4.0f, "OFFROAD", 1.5f, glm::vec4(1.0f));
    }

    // 7. Banner pulsante de alerta "¡TURBO BOOST!" sobre el velocimetro
    if (t.isBoosting) {
        float bannerW = w;
        float bannerH = 36.0f;
        float bannerX = x;
        float bannerY = y - bannerH - 8.0f;

        float animTime = (timing.totalRaceTime > 0.0f) ? timing.totalRaceTime : m_pulseTimer;
        float pulse = 0.75f + 0.25f * std::sin(animTime * 12.0f);

        m_uiRenderer.drawSprite(m_texTurboBoostBanner, bannerX, bannerY, bannerW, bannerH,
                                glm::vec4(1.0f, 1.0f, 1.0f, pulse));
        m_uiRenderer.drawTextCentered(bannerX, bannerY, bannerW, bannerH,
                                      "! TURBO BOOST !", 2.0f,
                                      glm::vec4(1.0f, 1.0f, 0.25f, pulse));
    }
}

void RacingHud::renderLapTimer(float screenWidth, float screenHeight, const LapTimingData& timing) {
    (void)screenHeight;

    // Posicion en la esquina superior derecha
    float w = 320.0f;
    float h = 160.0f;
    float x = screenWidth - w - 24.0f;
    float y = 24.0f;

    // 1. Placa de fondo transparente ahumada para legibilidad
    m_uiRenderer.drawRect(x + 4.0f, y + 4.0f, w - 8.0f, h - 8.0f, glm::vec4(0.02f, 0.04f, 0.08f, 0.90f));

    // 2. Marco sci-fi blade rectangle mediante 9-Slice para esquinas y muescas sin deformacion
    m_uiRenderer.drawNineSlice(m_texLapTimerFrame, x, y, w, h, 28.0f, 28.0f, glm::vec4(1.0f, 1.0f, 1.0f, 0.95f));

    // Panel interior oscuro
    float innerX = x + 16.0f;
    float innerY = y + 16.0f;
    float innerW = w - 32.0f;
    float innerH = h - 32.0f;
    m_uiRenderer.drawRect(innerX, innerY, innerW, innerH, glm::vec4(0.01f, 0.02f, 0.05f, 0.78f));
    m_uiRenderer.drawBorder(innerX, innerY, innerW, innerH, 1.5f, glm::vec4(0.0f, 0.75f, 0.95f, 0.45f));

    // 3. Fila superior: Insignia de vuelta (Kenney Space Expansion) + Contador de Vuelta [LAP 1 / 3]
    float badgeSize = 34.0f;
    float badgeX = innerX + 8.0f;
    float badgeY = innerY + 6.0f;
    m_uiRenderer.drawSprite(m_texLapBadge, badgeX, badgeY, badgeSize, badgeSize, glm::vec4(1.0f));

    // Texto de vuelta al lado de la insignia
    m_uiRenderer.drawText(badgeX + badgeSize + 10.0f, badgeY + 8.0f, "LAP", 2.0f,
                          glm::vec4(0.0f, 0.85f, 1.0f, 1.0f));

    std::string lapStr = std::to_string(std::max(timing.currentLap, 1)) + " / " + std::to_string(timing.totalLaps);
    m_uiRenderer.drawText(badgeX + badgeSize + 56.0f, badgeY + 8.0f, lapStr, 2.0f,
                          glm::vec4(1.0f, 0.95f, 0.35f, 1.0f));

    // Separador horizontal
    m_uiRenderer.drawRect(innerX + 8.0f, badgeY + badgeSize + 6.0f, innerW - 16.0f, 1.0f,
                          glm::vec4(0.18f, 0.35f, 0.55f, 0.50f));

    // 4. Fila 2: Tiempo de Vuelta Actual (Cronometro digital 7-segmentos)
    float row1Y = innerY + 54.0f;
    m_uiRenderer.drawText(innerX + 10.0f, row1Y + 3.0f, "TIME", 1.5f, glm::vec4(0.70f, 0.88f, 0.98f, 0.90f));
    draw7SegmentTime(innerX + 68.0f, row1Y - 2.0f, timing.currentLapTime, 0.85f,
                     glm::vec4(0.0f, 0.95f, 1.0f, 1.0f));

    // 5. Fila 3: Mejor Vuelta (Best Lap)
    float row2Y = innerY + 82.0f;
    m_uiRenderer.drawText(innerX + 10.0f, row2Y + 3.0f, "BEST", 1.5f, glm::vec4(1.0f, 0.80f, 0.20f, 0.95f));
    draw7SegmentTime(innerX + 68.0f, row2Y - 2.0f, (timing.bestLapTime > 0.0f) ? timing.bestLapTime : -1.0f, 0.85f,
                     glm::vec4(1.0f, 0.82f, 0.15f, 1.0f));

    // 6. Fila 4: Tiempo Total de Carrera
    float row3Y = innerY + 110.0f;
    m_uiRenderer.drawText(innerX + 10.0f, row3Y + 3.0f, "TOTAL", 1.5f, glm::vec4(0.65f, 0.75f, 0.85f, 0.85f));
    draw7SegmentTime(innerX + 68.0f, row3Y - 2.0f, timing.totalRaceTime, 0.85f,
                     glm::vec4(0.85f, 0.92f, 0.98f, 0.85f));
}

void RacingHud::renderLapNotice(float screenWidth, float screenHeight, const LapTimingData& timing) {
    if (timing.lapNoticeTimer <= 0.0f) return;

    float w = 480.0f;
    float h = 68.0f;
    float x = (screenWidth - w) * 0.5f;
    float y = screenHeight * 0.18f;

    float alpha = std::clamp(timing.lapNoticeTimer / 0.5f, 0.0f, 1.0f);
    glm::vec4 bgCol(0.02f, 0.04f, 0.08f, 0.92f * alpha);
    glm::vec4 borderCol = timing.isNewRecord ? glm::vec4(1.0f, 0.80f, 0.10f, alpha)
                                            : glm::vec4(0.0f, 0.85f, 1.0f, alpha);

    m_uiRenderer.drawRect(x, y, w, h, bgCol);
    m_uiRenderer.drawBorder(x, y, w, h, 2.5f, borderCol);

    if (timing.isNewRecord) {
        m_uiRenderer.drawText(x + 55.0f, y + 14.0f, "! NUEVO RECORD DE VUELTA !", 2.2f,
                              glm::vec4(1.0f, 0.85f, 0.15f, alpha));
    } else {
        m_uiRenderer.drawText(x + 80.0f, y + 14.0f, "! VUELTA COMPLETADA !", 2.2f,
                              glm::vec4(0.0f, 0.90f, 1.0f, alpha));
    }

    m_uiRenderer.drawText(x + 105.0f, y + 42.0f, "TIEMPO: ", 1.8f, glm::vec4(0.85f, 0.90f, 0.95f, alpha));
    draw7SegmentTime(x + 205.0f, y + 36.0f, timing.lastLapTime, 0.90f, borderCol);
}

void RacingHud::render(float screenWidth, float screenHeight,
                       const TelemetryData& telemetry,
                       const LapTimingData& timing) {
    if (!m_initialized) return;

    m_pulseTimer += 0.016f;

    // Comenzar el pase de renderizado 2D por lotes
    m_uiRenderer.begin(screenWidth, screenHeight);

    // Renderizar elementos de interfaz de carreras
    renderSpeedometer(screenWidth, screenHeight, telemetry, timing);
    renderLapTimer(screenWidth, screenHeight, timing);
    renderLapNotice(screenWidth, screenHeight, timing);

    // Finalizar el pase y vaciar los lotes a la GPU con restauracion de estados
    m_uiRenderer.end();
}

} // namespace ui
