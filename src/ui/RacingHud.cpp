#include "ui/RacingHud.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace ui {

namespace {

const char* HUD_VERT_SRC = R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;

uniform mat4 u_OrthoProj;

out vec4 vColor;

void main() {
    vColor = aColor;
    gl_Position = u_OrthoProj * vec4(aPos, 0.0, 1.0);
}
)";

const char* HUD_FRAG_SRC = R"(#version 330 core
in vec4 vColor;
out vec4 FragColor;

void main() {
    FragColor = vColor;
}
)";

// Fuente matricial 5x7 para caracteres ASCII 32 (' ') hasta 90 ('Z')
const uint8_t FONT_5X7[][7] = {
    // 32 ' '
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 33 '!'
    {0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00},
    // 34 '"'
    {0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 35 '#'
    {0x0A, 0x1F, 0x0A, 0x0A, 0x1F, 0x0A, 0x00},
    // 36 '$'
    {0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04},
    // 37 '%'
    {0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03},
    // 38 '&'
    {0x08, 0x14, 0x14, 0x08, 0x15, 0x12, 0x0D},
    // 39 '\''
    {0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00},
    // 40 '('
    {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02},
    // 41 ')'
    {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08},
    // 42 '*'
    {0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00},
    // 43 '+'
    {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00},
    // 44 ','
    {0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08},
    // 45 '-'
    {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00},
    // 46 '.'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00},
    // 47 '/'
    {0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00},
    // 48 '0'
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E},
    // 49 '1'
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
    // 50 '2'
    {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F},
    // 51 '3'
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E},
    // 52 '4'
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
    // 53 '5'
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
    // 54 '6'
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E},
    // 55 '7'
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
    // 56 '8'
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
    // 57 '9'
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C},
    // 58 ':'
    {0x00, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00},
    // 59 ';'
    {0x00, 0x04, 0x00, 0x00, 0x04, 0x04, 0x08},
    // 60 '<'
    {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02},
    // 61 '='
    {0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00},
    // 62 '>'
    {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08},
    // 63 '?'
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04},
    // 64 '@'
    {0x0E, 0x11, 0x01, 0x0D, 0x15, 0x15, 0x0E},
    // 65 'A'
    {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
    // 66 'B'
    {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E},
    // 67 'C'
    {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E},
    // 68 'D'
    {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C},
    // 69 'E'
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F},
    // 70 'F'
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10},
    // 71 'G'
    {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F},
    // 72 'H'
    {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},
    // 73 'I'
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E},
    // 74 'J'
    {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C},
    // 75 'K'
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},
    // 76 'L'
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F},
    // 77 'M'
    {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11},
    // 78 'N'
    {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11},
    // 79 'O'
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
    // 80 'P'
    {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
    // 81 'Q'
    {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D},
    // 82 'R'
    {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11},
    // 83 'S'
    {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E},
    // 84 'T'
    {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
    // 85 'U'
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},
    // 86 'V'
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04},
    // 87 'W'
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A},
    // 88 'X'
    {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11},
    // 89 'Y'
    {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04},
    // 90 'Z'
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}
};

} // namespace

RacingHud::RacingHud() {
    m_vertices.reserve(4096);
}

RacingHud::~RacingHud() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

bool RacingHud::init() {
    if (m_initialized) return true;

    if (!m_shader.loadFromSource(HUD_VERT_SRC, HUD_FRAG_SRC)) {
        std::cerr << "[ERROR] [RacingHud] Error compilando shader procedural de interfaz 2D." << std::endl;
        return false;
    }

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 8192 * sizeof(HudVertex), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(HudVertex), reinterpret_cast<void*>(offsetof(HudVertex, pos)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(HudVertex), reinterpret_cast<void*>(offsetof(HudVertex, color)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_initialized = true;
    std::cout << "[OK] [RacingHud] Interfaz de usuario HUD (Velocimetro + Cronometro) inicializada." << std::endl;
    return true;
}

void RacingHud::addQuad(float x, float y, float w, float h, const glm::vec4& color) {
    float x1 = x;
    float y1 = y;
    float x2 = x + w;
    float y2 = y + h;

    m_vertices.push_back({{x1, y1}, color});
    m_vertices.push_back({{x2, y1}, color});
    m_vertices.push_back({{x2, y2}, color});

    m_vertices.push_back({{x1, y1}, color});
    m_vertices.push_back({{x2, y2}, color});
    m_vertices.push_back({{x1, y2}, color});
}

void RacingHud::addBorder(float x, float y, float w, float h, float thickness, const glm::vec4& color) {
    addQuad(x, y, w, thickness, color); // Arriba
    addQuad(x, y + h - thickness, w, thickness, color); // Abajo
    addQuad(x, y + thickness, thickness, h - 2.0f * thickness, color); // Izquierda
    addQuad(x + w - thickness, y + thickness, thickness, h - 2.0f * thickness, color); // Derecha
}

void RacingHud::drawText(float x, float y, const std::string& text, float scale, const glm::vec4& color) {
    float curX = x;
    for (char ch : text) {
        char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        if (upper >= 32 && upper <= 90) {
            int glyphIdx = upper - 32;
            for (int r = 0; r < 7; ++r) {
                uint8_t row = FONT_5X7[glyphIdx][r];
                for (int c = 0; c < 5; ++c) {
                    if (row & (1 << (4 - c))) {
                        addQuad(curX + c * scale, y + r * scale, scale, scale, color);
                    }
                }
            }
        }
        curX += 6.0f * scale;
    }
}

void RacingHud::draw7SegmentDigit(float x, float y, int digit, float w, float h, float thickness,
                                  const glm::vec4& activeColor, const glm::vec4& inactiveColor) {
    // 7 segmentos: a, b, c, d, e, f, g
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
    addQuad(x + thickness + gap, y, w - 2.0f * (thickness + gap), thickness, segColor(0));
    // Segmento B: vertical superior derecho
    addQuad(x + w - thickness, y + thickness + gap, thickness, halfH - thickness - 2.0f * gap, segColor(1));
    // Segmento C: vertical inferior derecho
    addQuad(x + w - thickness, y + halfH + gap, thickness, halfH - thickness - 2.0f * gap, segColor(2));
    // Segmento D: horizontal inferior
    addQuad(x + thickness + gap, y + h - thickness, w - 2.0f * (thickness + gap), thickness, segColor(3));
    // Segmento E: vertical inferior izquierdo
    addQuad(x, y + halfH + gap, thickness, halfH - thickness - 2.0f * gap, segColor(4));
    // Segmento F: vertical superior izquierdo
    addQuad(x, y + thickness + gap, thickness, halfH - thickness - 2.0f * gap, segColor(5));
    // Segmento G: horizontal central
    addQuad(x + thickness + gap, y + halfH - thickness * 0.5f, w - 2.0f * (thickness + gap), thickness, segColor(6));
}

void RacingHud::drawColon(float x, float y, float scale, const glm::vec4& color) {
    float size = 3.0f * scale;
    addQuad(x, y + 6.0f * scale, size, size, color);
    addQuad(x, y + 16.0f * scale, size, size, color);
}

void RacingHud::drawDot(float x, float y, float scale, const glm::vec4& color) {
    float size = 3.0f * scale;
    addQuad(x, y + 19.0f * scale, size, size, color);
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

void RacingHud::renderSpeedometer(float screenWidth, float screenHeight, const TelemetryData& t) {
    float w = 270.0f;
    float h = 150.0f;
    float x = screenWidth - w - 24.0f;
    float y = screenHeight - h - 24.0f;

    // 1. Placa de fondo con efecto vidrio ahumado
    addQuad(x, y, w, h, glm::vec4(0.03f, 0.05f, 0.08f, 0.88f));

    // Borde iluminado con respuesta a Turbo / Derrape
    glm::vec4 borderColor = glm::vec4(0.0f, 0.80f, 1.0f, 0.55f);
    if (t.isBoosting) {
        borderColor = glm::vec4(1.0f, 0.55f, 0.0f, 0.95f); // Brillo anaranjado en turbo
    } else if (t.isSkidding) {
        borderColor = glm::vec4(1.0f, 0.85f, 0.0f, 0.85f); // Brillo dorado en derrape
    }
    addBorder(x, y, w, h, 2.0f, borderColor);

    // Barra de acento superior
    addQuad(x, y, w, 3.5f, borderColor);

    // 2. Tacometro / Barra de RPM LED interactiva
    float barX = x + 18.0f;
    float barY = y + 16.0f;
    float barW = w - 36.0f;
    float barH = 10.0f;
    int segCount = 18;
    float gap = 2.5f;
    float segW = (barW - (segCount - 1) * gap) / segCount;

    float ratio = std::clamp(t.speedKmH / std::max(t.maxSpeedKmH, 1.0f), 0.0f, 1.0f);
    int activeSegs = static_cast<int>(std::round(ratio * segCount));

    for (int i = 0; i < segCount; ++i) {
        glm::vec4 activeCol;
        if (i < 10) {
            activeCol = glm::vec4(0.0f, 0.95f, 0.55f, 1.0f); // Verde neon
        } else if (i < 14) {
            activeCol = glm::vec4(1.0f, 0.80f, 0.0f, 1.0f); // Ambar de competicion
        } else {
            activeCol = glm::vec4(1.0f, 0.15f, 0.25f, 1.0f); // Rojo limite de corte (redline)
        }
        glm::vec4 inactiveCol = glm::vec4(activeCol.r * 0.2f, activeCol.g * 0.2f, activeCol.b * 0.2f, 0.15f);

        glm::vec4 c = (i < activeSegs) ? activeCol : inactiveCol;
        addQuad(barX + i * (segW + gap), barY, segW, barH, c);
    }

    // 3. Velocidad digital grande (7 segmentos)
    int spd = std::clamp(static_cast<int>(std::round(t.speedKmH)), 0, 999);
    int d0 = (spd / 100) % 10;
    int d1 = (spd / 10) % 10;
    int d2 = spd % 10;

    glm::vec4 digitColor = glm::vec4(0.90f, 0.98f, 1.0f, 1.0f);
    if (t.isBoosting) {
        digitColor = glm::vec4(1.0f, 0.85f, 0.35f, 1.0f);
    }
    glm::vec4 inactiveDigit(digitColor.r, digitColor.g, digitColor.b, 0.06f);

    float digW = 22.0f;
    float digH = 44.0f;
    float digThick = 4.8f;
    float digGap = 8.0f;
    float digStartX = x + 24.0f;
    float digStartY = y + 38.0f;

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
    drawText(digStartX + 3.0f * (digW + digGap) + 8.0f, digStartY + 14.0f, "KM/H", 2.2f,
             glm::vec4(0.0f, 0.85f, 1.0f, 0.90f));

    // 4. Insignias de Estado (Badges): Marcha, Turbo, Derrape, Offroad
    float badgeY = y + 104.0f;

    // Selector de Marcha [ D ] o [ R ]
    if (t.isReversing) {
        addQuad(x + 18.0f, badgeY, 34.0f, 24.0f, glm::vec4(0.85f, 0.15f, 0.15f, 0.85f));
        drawText(x + 28.0f, badgeY + 5.0f, "R", 2.0f, glm::vec4(1.0f));
    } else {
        addQuad(x + 18.0f, badgeY, 34.0f, 24.0f, glm::vec4(0.0f, 0.50f, 0.70f, 0.85f));
        drawText(x + 28.0f, badgeY + 5.0f, "D", 2.0f, glm::vec4(1.0f));
    }

    // Badge TURBO
    if (t.isBoosting) {
        addQuad(x + 60.0f, badgeY, 68.0f, 24.0f, glm::vec4(1.0f, 0.45f, 0.0f, 0.90f));
        drawText(x + 66.0f, badgeY + 5.0f, "TURBO", 1.8f, glm::vec4(1.0f));
    }

    // Badge DERRAPE
    if (t.isSkidding) {
        addQuad(x + 136.0f, badgeY, 68.0f, 24.0f, glm::vec4(0.85f, 0.70f, 0.0f, 0.90f));
        drawText(x + 144.0f, badgeY + 5.0f, "DRIFT", 1.8f, glm::vec4(0.05f, 0.05f, 0.05f, 1.0f));
    } else if (t.isOffRoad) {
        addQuad(x + 136.0f, badgeY, 80.0f, 24.0f, glm::vec4(0.65f, 0.35f, 0.15f, 0.85f));
        drawText(x + 140.0f, badgeY + 5.0f, "OFFROAD", 1.5f, glm::vec4(1.0f));
    }
}

void RacingHud::renderLapTimer(float screenWidth, float screenHeight, const LapTimingData& timing) {
    (void)screenWidth;
    (void)screenHeight;

    float w = 295.0f;
    float h = 155.0f;
    float x = 24.0f;
    float y = 24.0f;

    // 1. Placa de fondo transparente ahumada
    addQuad(x, y, w, h, glm::vec4(0.03f, 0.05f, 0.08f, 0.88f));
    addBorder(x, y, w, h, 2.0f, glm::vec4(0.0f, 0.75f, 0.95f, 0.50f));

    // Acento superior brillante
    addQuad(x, y, w, 3.5f, glm::vec4(0.0f, 0.85f, 1.0f, 0.90f));

    // 2. Fila 1: Contador de Vuelta [VUELTA  1 / 3]
    drawText(x + 16.0f, y + 14.0f, "VUELTA", 2.0f, glm::vec4(0.0f, 0.85f, 1.0f, 1.0f));

    std::string lapStr = std::to_string(std::max(timing.currentLap, 1)) + " / " + std::to_string(timing.totalLaps);
    drawText(x + 115.0f, y + 14.0f, lapStr, 2.0f, glm::vec4(1.0f, 0.95f, 0.40f, 1.0f));

    // Separador
    addQuad(x + 16.0f, y + 36.0f, w - 32.0f, 1.0f, glm::vec4(0.2f, 0.35f, 0.5f, 0.45f));

    // 3. Fila 2: Tiempo de Vuelta Actual (Cronometro digital)
    drawText(x + 16.0f, y + 48.0f, "TIEMPO", 1.6f, glm::vec4(0.70f, 0.85f, 0.95f, 0.90f));
    draw7SegmentTime(x + 92.0f, y + 42.0f, timing.currentLapTime, 0.95f, glm::vec4(0.0f, 0.95f, 1.0f, 1.0f));

    // 4. Fila 3: Mejor Vuelta (Best Lap)
    drawText(x + 16.0f, y + 84.0f, "MEJOR", 1.6f, glm::vec4(1.0f, 0.80f, 0.20f, 0.95f));
    draw7SegmentTime(x + 92.0f, y + 78.0f, (timing.bestLapTime > 0.0f) ? timing.bestLapTime : -1.0f, 0.95f,
                     glm::vec4(1.0f, 0.82f, 0.15f, 1.0f));

    // 5. Fila 4: Tiempo Total de Carrera
    drawText(x + 16.0f, y + 120.0f, "TOTAL", 1.6f, glm::vec4(0.65f, 0.75f, 0.85f, 0.85f));
    draw7SegmentTime(x + 92.0f, y + 114.0f, timing.totalRaceTime, 0.95f, glm::vec4(0.85f, 0.92f, 0.98f, 0.85f));
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

    addQuad(x, y, w, h, bgCol);
    addBorder(x, y, w, h, 2.5f, borderCol);

    if (timing.isNewRecord) {
        drawText(x + 55.0f, y + 14.0f, "! NUEVO RECORD DE VUELTA !", 2.2f, glm::vec4(1.0f, 0.85f, 0.15f, alpha));
    } else {
        drawText(x + 80.0f, y + 14.0f, "! VUELTA COMPLETADA !", 2.2f, glm::vec4(0.0f, 0.90f, 1.0f, alpha));
    }

    drawText(x + 105.0f, y + 42.0f, "TIEMPO: ", 1.8f, glm::vec4(0.85f, 0.90f, 0.95f, alpha));
    draw7SegmentTime(x + 205.0f, y + 36.0f, timing.lastLapTime, 0.90f, borderCol);
}

void RacingHud::render(float screenWidth, float screenHeight,
                       const TelemetryData& telemetry,
                       const LapTimingData& timing) {
    if (!m_initialized) return;

    m_vertices.clear();

    // Dibujar elementos de interfaz
    renderSpeedometer(screenWidth, screenHeight, telemetry);
    renderLapTimer(screenWidth, screenHeight, timing);
    renderLapNotice(screenWidth, screenHeight, timing);

    if (m_vertices.empty()) return;

    // Configurar estado 2D
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_orthoProj = glm::ortho(0.0f, screenWidth, screenHeight, 0.0f, -1.0f, 1.0f);

    m_shader.use();
    m_shader.setMat4("u_OrthoProj", m_orthoProj);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(HudVertex), m_vertices.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

} // namespace ui
