#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "renderer/Shader.hpp"
#include "renderer/Texture.hpp"

namespace ui {

struct UIVertex {
    glm::vec2 pos;
    glm::vec2 uv;
    glm::vec4 color;
};

/**
 * @brief Renderizador de interfaz 2D por lotes (Batch 2D Sprite & Shape Renderer).
 * Diseñado bajo OpenGL 3.3 Core Profile con proyección ortográfica.
 * Permite renderizar sprites de textura con canal alfa, recortes UV (barras de progreso),
 * rectángulos coloreados y tipografía digital integrada.
 */
class UIRenderer {
public:
    UIRenderer();
    ~UIRenderer();

    UIRenderer(const UIRenderer&) = delete;
    UIRenderer& operator=(const UIRenderer&) = delete;

    UIRenderer(UIRenderer&& other) noexcept;
    UIRenderer& operator=(UIRenderer&& other) noexcept;

    // Inicializa shaders, buffers VAO/VBO y recursos gráficos
    bool init(const std::string& vertPath = "shaders/ui.vert",
              const std::string& fragPath = "shaders/ui.frag");
    void destroy();

    // Comienza un pase de renderizado 2D configurando viewport y proyección ortográfica
    void begin(float screenWidth, float screenHeight);

    // Finaliza el pase y vacía los vértices pendientes a la GPU
    void end();

    // Dibuja un sprite texturizado completo
    void drawSprite(const renderer::Texture& texture, float x, float y, float width, float height,
                    const glm::vec4& tint = glm::vec4(1.0f));

    // Dibuja una sub-región de un sprite especificando coordenadas UV [0, 1]
    void drawSpritePart(const renderer::Texture& texture, float x, float y, float width, float height,
                        float u0, float v0, float u1, float v1,
                        const glm::vec4& tint = glm::vec4(1.0f));

    // Dibuja un sprite recortado como barra de progreso (fillAmount entre 0.0f y 1.0f)
    void drawSpriteProgress(const renderer::Texture& texture, float x, float y, float width, float height,
                            float fillAmount, bool horizontal = true,
                            const glm::vec4& tint = glm::vec4(1.0f));

    // Dibuja un sprite utilizando la tecnica de 9-Slice (9 cuadrantes) para preservar esquinas sin deformacion
    void drawNineSlice(const renderer::Texture& texture, float x, float y, float width, float height,
                       float cornerWidth, float cornerHeight, const glm::vec4& tint = glm::vec4(1.0f));

    // Dibuja una barra utilizando 3-Slice horizontal a partir de una unica textura (tapas izquierda y derecha + centro)
    void drawThreeSliceHorizontal(const renderer::Texture& texture, float x, float y, float width, float height,
                                  float capWidth, const glm::vec4& tint = glm::vec4(1.0f));

    // Dibuja una barra utilizando 3-Slice horizontal a partir de 3 texturas modulares separadas (Left, Middle, Right)
    void drawThreeSliceHorizontal(const renderer::Texture& leftTex,
                                  const renderer::Texture& midTex,
                                  const renderer::Texture& rightTex,
                                  float x, float y, float width, float height,
                                  float capWidth = -1.0f,
                                  const glm::vec4& tint = glm::vec4(1.0f));

    // Dibuja una barra de progreso utilizando 3-Slice horizontal modular (Left, Middle, Right) con llenado progresivo
    void drawThreeSliceProgress(const renderer::Texture& leftTex,
                                const renderer::Texture& midTex,
                                const renderer::Texture& rightTex,
                                float x, float y, float width, float height,
                                float fillRatio,
                                float capWidth = -1.0f,
                                const glm::vec4& tint = glm::vec4(1.0f));

    // Dibuja un rectángulo de color sólido
    void drawRect(float x, float y, float width, float height, const glm::vec4& color);

    // Dibuja un borde de rectángulo sin relleno
    void drawBorder(float x, float y, float width, float height, float thickness, const glm::vec4& color);

    // Dibuja texto digital utilizando la fuente vectorial integrada
    void drawText(float x, float y, const std::string& text, float scale = 2.0f,
                  const glm::vec4& color = glm::vec4(1.0f));

    // Dibuja texto centrado horizontalmente dentro de un cuadro
    void drawTextCentered(float boxX, float boxY, float boxWidth, float boxHeight,
                          const std::string& text, float scale = 2.0f,
                          const glm::vec4& color = glm::vec4(1.0f));

    // Utilidades de métricas de texto
    float getTextWidth(const std::string& text, float scale = 2.0f) const;
    float getTextHeight(float scale = 2.0f) const;

    bool isInitialized() const { return m_initialized; }

private:
    void flush();
    void setTexture(unsigned int textureID);
    void addQuad(float x, float y, float w, float h,
                 float u0, float v0, float u1, float v1,
                 const glm::vec4& color);

    renderer::Shader m_shader;
    unsigned int m_vao{0};
    unsigned int m_vbo{0};
    unsigned int m_whiteTextureID{0};
    unsigned int m_currentTextureID{0};

    std::vector<UIVertex> m_vertices;
    float m_screenWidth{1280.0f};
    float m_screenHeight{720.0f};
    bool m_initialized{false};
    bool m_inBegin{false};
};

} // namespace ui
