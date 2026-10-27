#include "ui/RacingHud.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>

namespace ui {

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
    , m_texGearDrive(std::move(other.m_texGearDrive))
    , m_texGearReverse(std::move(other.m_texGearReverse))
    , m_texBadgeTurbo(std::move(other.m_texBadgeTurbo))
    , m_texBadgeDrift(std::move(other.m_texBadgeDrift))
    , m_texBadgeOffroad(std::move(other.m_texBadgeOffroad))
    , m_texLapNoticeNormal(std::move(other.m_texLapNoticeNormal))
    , m_texLapNoticeRecord(std::move(other.m_texLapNoticeRecord))
    , m_texPauseModalFrame(std::move(other.m_texPauseModalFrame))
    , m_texPauseBtnIdle(std::move(other.m_texPauseBtnIdle))
    , m_texPauseBtnHover(std::move(other.m_texPauseBtnHover))
    , m_texArrowPointer(std::move(other.m_texArrowPointer))
    , m_texCheckOn(std::move(other.m_texCheckOn))
    , m_texCheckOff(std::move(other.m_texCheckOff))
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
        m_texGearDrive = std::move(other.m_texGearDrive);
        m_texGearReverse = std::move(other.m_texGearReverse);
        m_texBadgeTurbo = std::move(other.m_texBadgeTurbo);
        m_texBadgeDrift = std::move(other.m_texBadgeDrift);
        m_texBadgeOffroad = std::move(other.m_texBadgeOffroad);
        m_texLapNoticeNormal = std::move(other.m_texLapNoticeNormal);
        m_texLapNoticeRecord = std::move(other.m_texLapNoticeRecord);
        m_texPauseModalFrame = std::move(other.m_texPauseModalFrame);
        m_texPauseBtnIdle = std::move(other.m_texPauseBtnIdle);
        m_texPauseBtnHover = std::move(other.m_texPauseBtnHover);
        m_texArrowPointer = std::move(other.m_texArrowPointer);
        m_texCheckOn = std::move(other.m_texCheckOn);
        m_texCheckOff = std::move(other.m_texCheckOff);
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

    // Insignias Sci-Fi de marcha y telemetria (Kenney Space Expansion)
    loadTexture(m_texGearDrive, "Blue/Default/button_square_header_small_square.png");
    loadTexture(m_texGearReverse, "Red/Default/button_square_header_small_square.png");
    loadTexture(m_texBadgeTurbo, "Red/Default/button_square_header_small_rectangle.png");
    loadTexture(m_texBadgeDrift, "Yellow/Default/button_square_header_small_rectangle.png");
    loadTexture(m_texBadgeOffroad, "Grey/Default/button_square_header_small_rectangle.png");

    // Marcos de notificacion de vuelta (Kenney Space Expansion)
    loadTexture(m_texLapNoticeNormal, "Blue/Default/button_square_header_blade_rectangle_screws.png");
    loadTexture(m_texLapNoticeRecord, "Yellow/Default/button_square_header_blade_rectangle_screws.png");

    // Marco exterior para modal de pausa (Kenney Space Expansion)
    loadTexture(m_texPauseModalFrame, "Blue/Default/button_square_header_large_rectangle_screws.png");

    // Texturas para botones, controles y selectores del Modal de Pausa (Kenney UI Pack estándar)
    const std::string uiPackBasePath = "assets/kenney_ui-pack/PNG/";
    auto loadUiTexture = [&](renderer::Texture& tex, const std::string& relPath) -> bool {
        std::string doubleRel = relPath;
        size_t idx = doubleRel.find("/Default/");
        if (idx != std::string::npos) {
            doubleRel.replace(idx, 9, "/Double/");
        }
        std::string resolvedDouble = resolveAssetPath(uiPackBasePath + doubleRel);
        if (tex.loadFromFile(resolvedDouble, false)) {
            return true;
        }
        std::string resolvedDefault = resolveAssetPath(uiPackBasePath + relPath);
        return tex.loadFromFile(resolvedDefault, false);
    };

    loadUiTexture(m_texPauseBtnIdle, "Blue/Default/button_rectangle_depth_flat.png");
    loadUiTexture(m_texPauseBtnHover, "Yellow/Default/button_rectangle_depth_gradient.png");
    loadUiTexture(m_texArrowPointer, "Yellow/Default/arrow_decorative_e.png");
    loadUiTexture(m_texCheckOn, "Yellow/Default/check_square_color_checkmark.png");
    loadUiTexture(m_texCheckOff, "Grey/Default/check_square_grey.png");

    m_initialized = true;
    std::cout << "[OK] [RacingHud] HUD de Carreras y Modal de Pausa (Kenney Space Expansion & UI Pack) inicializados exitosamente."
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
           m_texTurboBoostBanner.isLoaded() &&
           m_texGearDrive.isLoaded() &&
           m_texGearReverse.isLoaded() &&
           m_texBadgeTurbo.isLoaded() &&
           m_texBadgeDrift.isLoaded() &&
           m_texBadgeOffroad.isLoaded() &&
           m_texLapNoticeNormal.isLoaded() &&
           m_texLapNoticeRecord.isLoaded() &&
           m_texPauseModalFrame.isLoaded() &&
           m_texPauseBtnIdle.isLoaded() &&
           m_texPauseBtnHover.isLoaded() &&
           m_texArrowPointer.isLoaded() &&
           m_texCheckOn.isLoaded() &&
           m_texCheckOff.isLoaded();
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
    glm::vec2 pos = computeAnchoredPosition(UIAnchor::BOTTOM_RIGHT, w, h, screenWidth, screenHeight, -24.0f, -24.0f);
    float x = pos.x;
    float y = pos.y;

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

    // 6. Selector de Marcha [ D ] / [ R ] e insignias de estado con piezas modulares Kenney
    float badgeY = innerY + 84.0f;

    // Marcha con insignia Kenney
    float gearW = 32.0f;
    float gearH = 24.0f;
    const auto& gearTex = t.isReversing ? m_texGearReverse : m_texGearDrive;
    m_uiRenderer.drawNineSlice(gearTex, innerX + 16.0f, badgeY, gearW, gearH, 8.0f, 8.0f);
    m_uiRenderer.drawTextCentered(innerX + 16.0f, badgeY, gearW, gearH, t.isReversing ? "R" : "D", 2.0f, glm::vec4(1.0f));

    // Insignia TURBO con marco Kenney
    if (t.isBoosting) {
        float turboW = 72.0f;
        float turboH = 24.0f;
        float turboX = innerX + 54.0f;
        m_uiRenderer.drawNineSlice(m_texBadgeTurbo, turboX, badgeY, turboW, turboH, 8.0f, 8.0f);
        m_uiRenderer.drawTextCentered(turboX, badgeY, turboW, turboH, "TURBO", 1.8f, glm::vec4(1.0f));
    }

    // Insignia DERRAPE / OFFROAD con marco Kenney
    if (t.isSkidding) {
        float driftW = 72.0f;
        float driftH = 24.0f;
        float driftX = innerX + 132.0f;
        m_uiRenderer.drawNineSlice(m_texBadgeDrift, driftX, badgeY, driftW, driftH, 8.0f, 8.0f);
        m_uiRenderer.drawTextCentered(driftX, badgeY, driftW, driftH, "DRIFT", 1.8f, glm::vec4(0.12f, 0.12f, 0.14f, 1.0f));
    } else if (t.isOffRoad) {
        float offroadW = 86.0f;
        float offroadH = 24.0f;
        float offroadX = innerX + 132.0f;
        m_uiRenderer.drawNineSlice(m_texBadgeOffroad, offroadX, badgeY, offroadW, offroadH, 8.0f, 8.0f);
        m_uiRenderer.drawTextCentered(offroadX, badgeY, offroadW, offroadH, "OFFROAD", 1.5f, glm::vec4(1.0f));
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
    glm::vec2 pos = computeAnchoredPosition(UIAnchor::TOP_RIGHT, w, h, screenWidth, screenHeight, -24.0f, 24.0f);
    float x = pos.x;
    float y = pos.y;

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

    float w = 500.0f;
    float h = 76.0f;
    glm::vec2 pos = computeAnchoredPosition(UIAnchor::TOP_CENTER, w, h, screenWidth, screenHeight, 0.0f, screenHeight * 0.16f);
    float x = pos.x;
    float y = pos.y;

    float alpha = std::clamp(timing.lapNoticeTimer / 0.5f, 0.0f, 1.0f);

    // 1. Capa de fondo oscura ahumada para maximo contraste
    m_uiRenderer.drawRect(x + 4.0f, y + 4.0f, w - 8.0f, h - 8.0f, glm::vec4(0.02f, 0.04f, 0.08f, 0.88f * alpha));

    // 2. Marco Sci-Fi 9-Slice Kenney (Amarillo/Dorado para nuevo récord, Azul/Cian para vuelta completada)
    const auto& frameTex = timing.isNewRecord ? m_texLapNoticeRecord : m_texLapNoticeNormal;
    glm::vec4 frameTint = timing.isNewRecord ? glm::vec4(1.0f, 0.95f, 0.30f, alpha)
                                            : glm::vec4(0.40f, 0.75f, 1.0f, alpha);
    m_uiRenderer.drawNineSlice(frameTex, x, y, w, h, 28.0f, 28.0f, frameTint);

    // 3. Insignia decorativa Kenney en el lateral izquierdo
    float badgeSize = 34.0f;
    m_uiRenderer.drawSprite(m_texLapBadge, x + 16.0f, y + (h - badgeSize) * 0.5f, badgeSize, badgeSize,
                            glm::vec4(1.0f, 1.0f, 1.0f, alpha));

    // 4. Texto principal y cronómetro digital
    if (timing.isNewRecord) {
        m_uiRenderer.drawText(x + 64.0f, y + 14.0f, "! NUEVO RECORD DE VUELTA !", 2.2f,
                              glm::vec4(1.0f, 0.88f, 0.20f, alpha));
    } else {
        m_uiRenderer.drawText(x + 64.0f, y + 14.0f, "! VUELTA COMPLETADA !", 2.2f,
                              glm::vec4(0.0f, 0.90f, 1.0f, alpha));
    }

    m_uiRenderer.drawText(x + 85.0f, y + 44.0f, "TIEMPO: ", 1.8f, glm::vec4(0.85f, 0.92f, 0.98f, alpha));
    draw7SegmentTime(x + 185.0f, y + 38.0f, timing.lastLapTime, 0.90f,
                     timing.isNewRecord ? glm::vec4(1.0f, 0.85f, 0.15f, alpha)
                                        : glm::vec4(0.0f, 0.85f, 1.0f, alpha));
}

void RacingHud::renderPauseModal(float screenWidth, float screenHeight, const PauseMenuData& data) {
    if (!m_initialized) return;

    m_uiRenderer.begin(screenWidth, screenHeight);

    // 1. Oscurecimiento de fondo (alpha = 0.70f)
    m_uiRenderer.drawRect(0.0f, 0.0f, screenWidth, screenHeight, glm::vec4(0.0f, 0.0f, 0.0f, 0.70f));

    // 2. Cuadro de diálogo modal centrado
    float modalW = 680.0f;
    float modalH = 500.0f;
    glm::vec2 modalPos = computeAnchoredPosition(UIAnchor::CENTER, modalW, modalH, screenWidth, screenHeight);
    float modalX = modalPos.x;
    float modalY = modalPos.y;

    // Fondo solido azul profundo
    m_uiRenderer.drawRect(modalX + 6.0f, modalY + 6.0f, modalW - 12.0f, modalH - 12.0f, glm::vec4(0.05f, 0.08f, 0.15f, 0.96f));
    // Marco exterior con textura Kenney mediante 9-Slice para remaches y esquinas nítidas
    m_uiRenderer.drawNineSlice(m_texPauseModalFrame, modalX, modalY, modalW, modalH, 28.0f, 28.0f);
    // Borde brillante de acento
    m_uiRenderer.drawBorder(modalX, modalY, modalW, modalH, 2.0f, glm::vec4(0.35f, 0.65f, 0.95f, 0.85f));

    // 3. Titulo de la ventana modal
    m_uiRenderer.drawTextCentered(modalX, modalY + 18.0f, modalW, 30.0f,
                                 "CARRERA EN PAUSA", 2.8f, glm::vec4(1.0f, 0.88f, 0.20f, 1.0f));

    // Linea divisoria estetica
    m_uiRenderer.drawRect(modalX + 30.0f, modalY + 54.0f, modalW - 60.0f, 2.0f, glm::vec4(0.3f, 0.5f, 0.8f, 0.6f));

    // 4. Filas de Opciones de Pausa / Configuración (7 opciones)
    float optStartY = modalY + 66.0f;
    float rowHeight = 48.0f;
    float rowW = modalW - 48.0f;
    float rowX = modalX + 24.0f;

    for (int i = 0; i < 7; ++i) {
        float rowY = optStartY + static_cast<float>(i) * rowHeight;
        bool isSelected = (data.selectedOption == i);

        // Boton 9-slice con Kenney UI Pack
        const auto& btnTex = isSelected ? m_texPauseBtnHover : m_texPauseBtnIdle;
        m_uiRenderer.drawNineSlice(btnTex, rowX, rowY, rowW, rowHeight - 6.0f, 16.0f, 16.0f);

        // Flecha animada pulsante al seleccionar
        if (isSelected) {
            float arrowBounce = std::sin(data.animTimer * 6.0f) * 4.0f;
            m_uiRenderer.drawSprite(m_texArrowPointer, rowX - 30.0f + arrowBounce,
                                    rowY + (rowHeight - 6.0f - 24.0f) * 0.5f, 24.0f, 24.0f);
        }

        glm::vec4 textColor = isSelected ? glm::vec4(0.12f, 0.12f, 0.14f, 1.0f)
                                         : glm::vec4(0.92f, 0.95f, 1.0f, 1.0f);

        switch (i) {
            case 0: { // 1) CONTINUAR CARRERA (Resume)
                m_uiRenderer.drawTextCentered(rowX, rowY, rowW, rowHeight - 6.0f,
                                              "CONTINUAR CARRERA", 2.0f, textColor);
                break;
            }
            case 1: { // 2) MODELO DE ILUMINACIÓN (Flat / Lambert / Blinn-Phong)
                m_uiRenderer.drawText(rowX + 20.0f, rowY + 11.0f, "MODELO DE ILUMINACION", 1.8f, textColor);
                std::string modelStr = "< " + sanitizeText(data.lightingModel) + " >";
                m_uiRenderer.drawText(rowX + rowW - 250.0f, rowY + 11.0f, modelStr, 1.8f,
                                      isSelected ? glm::vec4(0.10f, 0.35f, 0.15f, 1.0f)
                                                 : glm::vec4(0.35f, 1.0f, 0.65f, 1.0f));
                break;
            }
            case 2: { // 3) CICLO DÍA / NOCHE (Toggle)
                m_uiRenderer.drawText(rowX + 20.0f, rowY + 11.0f, "CICLO DIA / NOCHE", 1.8f, textColor);
                const auto& chkTex = data.isNight ? m_texCheckOn : m_texCheckOff;
                float chkSize = 24.0f;
                float chkX = rowX + rowW - 170.0f;
                float chkY = rowY + (rowHeight - 6.0f - chkSize) * 0.5f;
                m_uiRenderer.drawSprite(chkTex, chkX, chkY, chkSize, chkSize);
                m_uiRenderer.drawText(chkX + chkSize + 10.0f, rowY + 11.0f, data.isNight ? "NOCHE" : "DIA", 1.8f,
                                      isSelected ? glm::vec4(0.12f, 0.12f, 0.14f, 1.0f)
                                                 : (data.isNight ? glm::vec4(0.4f, 0.8f, 1.0f, 1.0f)
                                                                 : glm::vec4(1.0f, 0.9f, 0.4f, 1.0f)));
                break;
            }
            case 3: { // 4) FAROS DEL KART (ON / OFF)
                m_uiRenderer.drawText(rowX + 20.0f, rowY + 11.0f, "FAROS DEL KART (SPOT)", 1.8f, textColor);
                const auto& chkTex = data.areHeadlightsOn ? m_texCheckOn : m_texCheckOff;
                float chkSize = 24.0f;
                float chkX = rowX + rowW - 170.0f;
                float chkY = rowY + (rowHeight - 6.0f - chkSize) * 0.5f;
                m_uiRenderer.drawSprite(chkTex, chkX, chkY, chkSize, chkSize);
                m_uiRenderer.drawText(chkX + chkSize + 10.0f, rowY + 11.0f, data.areHeadlightsOn ? "ON" : "OFF", 1.8f,
                                      isSelected ? glm::vec4(0.12f, 0.12f, 0.14f, 1.0f)
                                                 : (data.areHeadlightsOn ? glm::vec4(0.35f, 1.0f, 0.65f, 1.0f)
                                                                         : glm::vec4(0.7f, 0.7f, 0.7f, 0.8f)));
                break;
            }
            case 4: { // 5) REINICIAR CARRERA (Restart)
                m_uiRenderer.drawTextCentered(rowX, rowY, rowW, rowHeight - 6.0f,
                                              "REINICIAR CARRERA", 2.0f, textColor);
                break;
            }
            case 5: { // 6) VOLVER AL MENÚ PRINCIPAL (Return to Main Menu)
                m_uiRenderer.drawTextCentered(rowX, rowY, rowW, rowHeight - 6.0f,
                                              "VOLVER AL MENU PRINCIPAL", 2.0f, textColor);
                break;
            }
            case 6: { // 7) SALIR AL ESCRITORIO (Quit)
                m_uiRenderer.drawTextCentered(rowX, rowY, rowW, rowHeight - 6.0f,
                                              "SALIR AL ESCRITORIO", 2.0f, textColor);
                break;
            }
        }
    }

    // 5. Pie de pagina con atajos de navegacion dentro del modal
    m_uiRenderer.drawTextCentered(modalX, modalY + modalH - 32.0f, modalW, 20.0f,
                                  "[W/S/FLECHAS] Navegar | [ENTER/A/D] Modificar | [ESC] Continuar",
                                  1.6f, glm::vec4(0.80f, 0.88f, 0.98f, 0.85f));

    m_uiRenderer.end();
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
