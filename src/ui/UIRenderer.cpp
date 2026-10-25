#include "ui/UIRenderer.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cctype>
#include <cstddef>
#include <algorithm>

namespace ui {

namespace {

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

UIRenderer::UIRenderer() {
    m_vertices.reserve(4096);
}

UIRenderer::~UIRenderer() {
    destroy();
}

UIRenderer::UIRenderer(UIRenderer&& other) noexcept
    : m_shader(std::move(other.m_shader))
    , m_vao(other.m_vao)
    , m_vbo(other.m_vbo)
    , m_whiteTextureID(other.m_whiteTextureID)
    , m_currentTextureID(other.m_currentTextureID)
    , m_vertices(std::move(other.m_vertices))
    , m_screenWidth(other.m_screenWidth)
    , m_screenHeight(other.m_screenHeight)
    , m_initialized(other.m_initialized)
    , m_inBegin(other.m_inBegin)
{
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_whiteTextureID = 0;
    other.m_currentTextureID = 0;
    other.m_initialized = false;
    other.m_inBegin = false;
}

UIRenderer& UIRenderer::operator=(UIRenderer&& other) noexcept {
    if (this != &other) {
        destroy();

        m_shader = std::move(other.m_shader);
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_whiteTextureID = other.m_whiteTextureID;
        m_currentTextureID = other.m_currentTextureID;
        m_vertices = std::move(other.m_vertices);
        m_screenWidth = other.m_screenWidth;
        m_screenHeight = other.m_screenHeight;
        m_initialized = other.m_initialized;
        m_inBegin = other.m_inBegin;

        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_whiteTextureID = 0;
        other.m_currentTextureID = 0;
        other.m_initialized = false;
        other.m_inBegin = false;
    }
    return *this;
}

bool UIRenderer::init(const std::string& vertPath, const std::string& fragPath) {
    if (m_initialized) {
        return true;
    }

    if (!m_shader.loadFromFiles(vertPath, fragPath)) {
        std::cerr << "[ERROR] [UIRenderer] Fallo al cargar shaders desde: "
                  << vertPath << " y " << fragPath << std::endl;
        return false;
    }

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 8192 * sizeof(UIVertex), nullptr, GL_DYNAMIC_DRAW);

    // Layout 0: pos (vec2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex),
                          reinterpret_cast<void*>(offsetof(UIVertex, pos)));

    // Layout 1: uv (vec2)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex),
                          reinterpret_cast<void*>(offsetof(UIVertex, uv)));

    // Layout 2: color (vec4)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex),
                          reinterpret_cast<void*>(offsetof(UIVertex, color)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Crear textura blanca 1x1 RGBA para geometria sin textura
    glGenTextures(1, &m_whiteTextureID);
    glBindTexture(GL_TEXTURE_2D, m_whiteTextureID);
    const uint32_t whitePixel = 0xFFFFFFFF;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_currentTextureID = m_whiteTextureID;
    m_initialized = true;

    std::cout << "[OK] [UIRenderer] Renderizador 2D por lotes inicializado correctamente." << std::endl;
    return true;
}

void UIRenderer::destroy() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_whiteTextureID != 0) {
        glDeleteTextures(1, &m_whiteTextureID);
        m_whiteTextureID = 0;
    }
    m_vertices.clear();
    m_currentTextureID = 0;
    m_initialized = false;
    m_inBegin = false;
}

void UIRenderer::begin(float screenWidth, float screenHeight) {
    if (!m_initialized) return;

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_inBegin = true;
    m_vertices.clear();
    m_currentTextureID = m_whiteTextureID;

    glViewport(0, 0, static_cast<GLsizei>(screenWidth), static_cast<GLsizei>(screenHeight));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    m_shader.use();
    glm::mat4 orthoProj = glm::ortho(0.0f, screenWidth, screenHeight, 0.0f, -1.0f, 1.0f);
    m_shader.setMat4("u_OrthoProj", orthoProj);
    m_shader.setInt("u_Texture", 0);
}

void UIRenderer::end() {
    if (!m_initialized || !m_inBegin) return;

    flush();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDisable(GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, 0);
    m_shader.unbind();

    m_inBegin = false;
}

void UIRenderer::flush() {
    if (m_vertices.empty()) return;

    m_shader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_currentTextureID != 0 ? m_currentTextureID : m_whiteTextureID);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(UIVertex), m_vertices.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_vertices.clear();
}

void UIRenderer::setTexture(unsigned int textureID) {
    unsigned int target = (textureID != 0) ? textureID : m_whiteTextureID;
    if (m_currentTextureID != target) {
        if (!m_vertices.empty()) {
            flush();
        }
        m_currentTextureID = target;
    }
}

void UIRenderer::addQuad(float x, float y, float w, float h,
                         float u0, float v0, float u1, float v1,
                         const glm::vec4& color) {
    if (m_vertices.size() + 6 >= 8192) {
        flush();
    }

    float x1 = x;
    float y1 = y;
    float x2 = x + w;
    float y2 = y + h;

    m_vertices.push_back({{x1, y1}, {u0, v0}, color});
    m_vertices.push_back({{x2, y1}, {u1, v0}, color});
    m_vertices.push_back({{x2, y2}, {u1, v1}, color});

    m_vertices.push_back({{x1, y1}, {u0, v0}, color});
    m_vertices.push_back({{x2, y2}, {u1, v1}, color});
    m_vertices.push_back({{x1, y2}, {u0, v1}, color});
}

void UIRenderer::drawSprite(const renderer::Texture& texture, float x, float y, float width, float height,
                            const glm::vec4& tint) {
    if (!m_initialized || !m_inBegin) return;
    if (width <= 0.0f || height <= 0.0f) return;
    drawSpritePart(texture, x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, tint);
}

void UIRenderer::drawSpritePart(const renderer::Texture& texture, float x, float y, float width, float height,
                                float u0, float v0, float u1, float v1,
                                const glm::vec4& tint) {
    if (!m_initialized || !m_inBegin) return;
    if (width <= 0.0f || height <= 0.0f) return;
    setTexture(texture.getID());
    addQuad(x, y, width, height, u0, v0, u1, v1, tint);
}

void UIRenderer::drawSpriteProgress(const renderer::Texture& texture, float x, float y, float width, float height,
                                    float fillAmount, bool horizontal,
                                    const glm::vec4& tint) {
    if (!m_initialized || !m_inBegin) return;
    float clampedFill = std::clamp(fillAmount, 0.0f, 1.0f);
    if (clampedFill <= 0.0f) return;

    if (horizontal) {
        float partW = width * clampedFill;
        drawSpritePart(texture, x, y, partW, height, 0.0f, 0.0f, clampedFill, 1.0f, tint);
    } else {
        float partH = height * clampedFill;
        float partY = y + height - partH;
        drawSpritePart(texture, x, partY, width, partH, 0.0f, 1.0f - clampedFill, 1.0f, 1.0f, tint);
    }
}

void UIRenderer::drawNineSlice(const renderer::Texture& texture, float x, float y, float width, float height,
                               float cornerWidth, float cornerHeight, const glm::vec4& tint) {
    if (!m_initialized || !m_inBegin) return;
    if (width <= 0.0f || height <= 0.0f) return;

    float texW = static_cast<float>(texture.getWidth());
    float texH = static_cast<float>(texture.getHeight());
    if (texW <= 0.0f || texH <= 0.0f) {
        drawSprite(texture, x, y, width, height, tint);
        return;
    }

    float maxCw = std::min(cornerWidth, width * 0.5f);
    float maxCh = std::min(cornerHeight, height * 0.5f);

    float uCorner = std::clamp(cornerWidth / texW, 0.0f, 0.5f);
    float vCorner = std::clamp(cornerHeight / texH, 0.0f, 0.5f);

    float u0 = 0.0f;
    float u1 = uCorner;
    float u2 = 1.0f - uCorner;
    float u3 = 1.0f;

    float v0 = 0.0f;
    float v1 = vCorner;
    float v2 = 1.0f - vCorner;
    float v3 = 1.0f;

    float x0 = x;
    float x1 = x + maxCw;
    float x2 = x + width - maxCw;
    float wMid = std::max(0.0f, width - 2.0f * maxCw);

    float y0 = y;
    float y1 = y + maxCh;
    float y2 = y + height - maxCh;
    float hMid = std::max(0.0f, height - 2.0f * maxCh);

    setTexture(texture.getID());

    // Fila Superior
    addQuad(x0, y0, maxCw, maxCh, u0, v0, u1, v1, tint);
    if (wMid > 0.0f) {
        addQuad(x1, y0, wMid, maxCh, u1, v0, u2, v1, tint);
    }
    addQuad(x2, y0, maxCw, maxCh, u2, v0, u3, v1, tint);

    // Fila Central
    if (hMid > 0.0f) {
        addQuad(x0, y1, maxCw, hMid, u0, v1, u1, v2, tint);
        if (wMid > 0.0f) {
            addQuad(x1, y1, wMid, hMid, u1, v1, u2, v2, tint);
        }
        addQuad(x2, y1, maxCw, hMid, u2, v1, u3, v2, tint);
    }

    // Fila Inferior
    addQuad(x0, y2, maxCw, maxCh, u0, v2, u1, v3, tint);
    if (wMid > 0.0f) {
        addQuad(x1, y2, wMid, maxCh, u1, v2, u2, v3, tint);
    }
    addQuad(x2, y2, maxCw, maxCh, u2, v2, u3, v3, tint);
}

void UIRenderer::drawThreeSliceHorizontal(const renderer::Texture& texture, float x, float y, float width, float height,
                                          float capWidth, const glm::vec4& tint) {
    if (!m_initialized || !m_inBegin) return;
    if (width <= 0.0f || height <= 0.0f) return;

    float texW = static_cast<float>(texture.getWidth());
    if (texW <= 0.0f) {
        drawSprite(texture, x, y, width, height, tint);
        return;
    }

    float maxCap = std::min(capWidth, width * 0.5f);
    float uCap = std::clamp(capWidth / texW, 0.0f, 0.5f);
    float midW = std::max(0.0f, width - 2.0f * maxCap);

    setTexture(texture.getID());
    addQuad(x, y, maxCap, height, 0.0f, 0.0f, uCap, 1.0f, tint);
    if (midW > 0.0f) {
        addQuad(x + maxCap, y, midW, height, uCap, 0.0f, 1.0f - uCap, 1.0f, tint);
    }
    addQuad(x + width - maxCap, y, maxCap, height, 1.0f - uCap, 0.0f, 1.0f, 1.0f, tint);
}

void UIRenderer::drawThreeSliceHorizontal(const renderer::Texture& leftTex,
                                          const renderer::Texture& midTex,
                                          const renderer::Texture& rightTex,
                                          float x, float y, float width, float height,
                                          float capWidth, const glm::vec4& tint) {
    if (!m_initialized || !m_inBegin) return;
    if (width <= 0.0f || height <= 0.0f) return;

    float capW = capWidth;
    if (capW < 0.0f) {
        float nativeH = static_cast<float>(leftTex.getHeight());
        if (nativeH > 0.0f) {
            capW = static_cast<float>(leftTex.getWidth()) * (height / nativeH);
        } else {
            capW = 12.0f;
        }
    }
    capW = std::min(capW, width * 0.5f);
    float midW = std::max(0.0f, width - 2.0f * capW);

    drawSprite(leftTex, x, y, capW, height, tint);
    if (midW > 0.0f) {
        drawSprite(midTex, x + capW, y, midW, height, tint);
    }
    drawSprite(rightTex, x + width - capW, y, capW, height, tint);
}

void UIRenderer::drawThreeSliceProgress(const renderer::Texture& leftTex,
                                        const renderer::Texture& midTex,
                                        const renderer::Texture& rightTex,
                                        float x, float y, float width, float height,
                                        float fillRatio,
                                        float capWidth, const glm::vec4& tint) {
    if (!m_initialized || !m_inBegin) return;
    float clampedFill = std::clamp(fillRatio, 0.0f, 1.0f);
    if (clampedFill <= 0.0f) return;

    float capW = capWidth;
    if (capW < 0.0f) {
        float nativeH = static_cast<float>(leftTex.getHeight());
        if (nativeH > 0.0f) {
            capW = static_cast<float>(leftTex.getWidth()) * (height / nativeH);
        } else {
            capW = 12.0f;
        }
    }
    capW = std::min(capW, width * 0.5f);

    float currentW = width * clampedFill;

    // Caso 1: El llenado aún no completa la tapa izquierda
    if (currentW <= capW) {
        float frac = (capW > 0.0f) ? (currentW / capW) : 1.0f;
        drawSpritePart(leftTex, x, y, currentW, height, 0.0f, 0.0f, frac, 1.0f, tint);
        return;
    }

    // Tapa izquierda completa
    drawSprite(leftTex, x, y, capW, height, tint);

    // Caso 2: El llenado está en la sección media
    float rightStart = width - capW;
    if (currentW <= rightStart) {
        float currentMidW = currentW - capW;
        if (currentMidW > 0.0f) {
            drawSprite(midTex, x + capW, y, currentMidW, height, tint);
        }
        return;
    }

    // Sección media completa
    float totalMidW = std::max(0.0f, width - 2.0f * capW);
    if (totalMidW > 0.0f) {
        drawSprite(midTex, x + capW, y, totalMidW, height, tint);
    }

    // Caso 3: El llenado entra en la tapa derecha
    float rightFilled = currentW - rightStart;
    float rightFrac = (capW > 0.0f) ? std::clamp(rightFilled / capW, 0.0f, 1.0f) : 1.0f;
    drawSpritePart(rightTex, x + rightStart, y, rightFilled, height, 0.0f, 0.0f, rightFrac, 1.0f, tint);
}

void UIRenderer::drawRect(float x, float y, float width, float height, const glm::vec4& color) {
    if (!m_initialized || !m_inBegin) return;
    if (width <= 0.0f || height <= 0.0f) return;
    setTexture(m_whiteTextureID);
    addQuad(x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, color);
}

void UIRenderer::drawBorder(float x, float y, float width, float height, float thickness, const glm::vec4& color) {
    if (!m_initialized || !m_inBegin) return;
    if (width <= 0.0f || height <= 0.0f || thickness <= 0.0f) return;

    setTexture(m_whiteTextureID);

    float t = std::min(thickness, std::min(width * 0.5f, height * 0.5f));

    // Borde superior
    addQuad(x, y, width, t, 0.0f, 0.0f, 1.0f, 1.0f, color);
    // Borde inferior
    addQuad(x, y + height - t, width, t, 0.0f, 0.0f, 1.0f, 1.0f, color);

    float innerH = height - 2.0f * t;
    if (innerH > 0.0f) {
        // Borde izquierdo
        addQuad(x, y + t, t, innerH, 0.0f, 0.0f, 1.0f, 1.0f, color);
        // Borde derecho
        addQuad(x + width - t, y + t, t, innerH, 0.0f, 0.0f, 1.0f, 1.0f, color);
    }
}

void UIRenderer::drawText(float x, float y, const std::string& text, float scale,
                          const glm::vec4& color) {
    if (!m_initialized || !m_inBegin) return;
    if (scale <= 0.0f || text.empty()) return;

    setTexture(m_whiteTextureID);

    float curX = x;
    for (char ch : text) {
        char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        if (upper >= 32 && upper <= 90) {
            int glyphIdx = upper - 32;
            for (int r = 0; r < 7; ++r) {
                uint8_t row = FONT_5X7[glyphIdx][r];
                for (int c = 0; c < 5; ++c) {
                    if (row & (1 << (4 - c))) {
                        addQuad(curX + c * scale, y + r * scale, scale, scale, 0.0f, 0.0f, 1.0f, 1.0f, color);
                    }
                }
            }
        }
        curX += 6.0f * scale;
    }
}

void UIRenderer::drawTextCentered(float boxX, float boxY, float boxWidth, float boxHeight,
                                  const std::string& text, float scale,
                                  const glm::vec4& color) {
    if (!m_initialized || !m_inBegin) return;
    float textW = getTextWidth(text, scale);
    float textH = getTextHeight(scale);
    float textX = boxX + (boxWidth - textW) * 0.5f;
    float textY = boxY + (boxHeight - textH) * 0.5f;
    drawText(textX, textY, text, scale, color);
}

float UIRenderer::getTextWidth(const std::string& text, float scale) const {
    if (text.empty()) return 0.0f;
    return static_cast<float>(text.length()) * 6.0f * scale;
}

float UIRenderer::getTextHeight(float scale) const {
    return 7.0f * scale;
}

} // namespace ui
