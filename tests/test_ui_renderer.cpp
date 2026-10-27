#include <iostream>
#include <cassert>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "ui/UIRenderer.hpp"
#include "ui/UIUtils.hpp"
#include "renderer/Texture.hpp"

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Validacion de Motor 2D Batch UI Renderer          " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // 1. Inicializacion de ventana GLFW invisible y contexto OpenGL 3.3 Core
    // -------------------------------------------------------------------------
    if (!glfwInit()) {
        std::cerr << "Fallo al inicializar GLFW" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Test UIRenderer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Fallo al crear ventana GLFW" << std::endl;
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Fallo al cargar extensiones GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // -------------------------------------------------------------------------
    // 2. Verificacion de inicializacion de UIRenderer
    // -------------------------------------------------------------------------
    {
        ui::UIRenderer renderer;
        bool ok = renderer.init("shaders/ui.vert", "shaders/ui.frag");
        assert(ok && "UIRenderer::init() debe retornar true");
        (void)ok;
        assert(renderer.isInitialized() && "isInitialized() debe ser true tras init()");
        assert(glGetError() == GL_NO_ERROR && "No debe haber errores de GL tras inicializar UIRenderer");

        // ---------------------------------------------------------------------
        // 3. Verificacion de metricas de texto tipografico
        // ---------------------------------------------------------------------
        assert(renderer.getTextWidth("", 1.0f) == 0.0f);
        assert(renderer.getTextWidth("A", 1.0f) == 6.0f);
        assert(renderer.getTextWidth("MARIO", 2.0f) == 60.0f); // 5 chars * 6 px * 2.0 = 60.0f
        assert(renderer.getTextHeight(1.0f) == 7.0f);
        assert(renderer.getTextHeight(2.0f) == 14.0f);
        std::cout << "  [CHECK 1] Inicializacion de VAO/VBO/Shader y metricas de texto OK." << std::endl;

        // ---------------------------------------------------------------------
        // 4. Pase 2D con geometrias solidas (drawRect, drawBorder, drawText, drawTextWithShadow)
        // ---------------------------------------------------------------------
        renderer.begin(1280.0f, 720.0f);
        renderer.drawRect(100.0f, 100.0f, 300.0f, 150.0f, glm::vec4(0.1f, 0.2f, 0.4f, 0.85f));
        renderer.drawBorder(100.0f, 100.0f, 300.0f, 150.0f, 3.0f, glm::vec4(1.0f, 0.8f, 0.0f, 1.0f));
        renderer.drawText(120.0f, 120.0f, "MARIO KART CGI", 2.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        renderer.drawTextCentered(100.0f, 180.0f, 300.0f, 40.0f, "PULSA START", 1.5f, glm::vec4(0.0f, 1.0f, 0.8f, 1.0f));

        // Verificacion de texto con sombra proyectada (drop shadow estandar y personalizada)
        renderer.drawTextWithShadow(120.0f, 210.0f, "SHADOW TEXT", 2.0f, glm::vec4(1.0f, 0.88f, 0.20f, 1.0f));
        renderer.drawTextWithShadow(120.0f, 230.0f, "CUSTOM SHADOW", 1.8f,
                                    glm::vec4(0.0f, 0.90f, 1.0f, 1.0f), glm::vec4(0.1f, 0.1f, 0.1f, 0.9f), 2.5f);
        renderer.drawTextCenteredWithShadow(100.0f, 260.0f, 300.0f, 30.0f, "CENTERED SHADOW", 1.6f,
                                            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        renderer.end();

        assert(glGetError() == GL_NO_ERROR && "Pase de rectangulos y texto sin errores de OpenGL");
        std::cout << "  [CHECK 2] Pase de renderizado de rectangulos, bordes, texto digital y texto con sombra OK." << std::endl;

        // ---------------------------------------------------------------------
        // 5. Pase 2D con sprites texturizados y barras de progreso
        // ---------------------------------------------------------------------
        renderer::Texture testTexture;
        bool texOk = testTexture.loadFromFile("assets/textures/colormap.png");
        assert(texOk && "Carga de assets/textures/colormap.png debe tener exito");
        (void)texOk;

        renderer.begin(1280.0f, 720.0f);
        // Sprite completo
        renderer.drawSprite(testTexture, 450.0f, 100.0f, 128.0f, 128.0f, glm::vec4(1.0f));

        // Sub-region de sprite (UV personalizado)
        renderer.drawSpritePart(testTexture, 600.0f, 100.0f, 64.0f, 64.0f, 0.1f, 0.1f, 0.6f, 0.6f, glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));

        // Barra de progreso horizontal (70%)
        renderer.drawSpriteProgress(testTexture, 450.0f, 250.0f, 200.0f, 24.0f, 0.70f, true);

        // Barra de progreso vertical (50%)
        renderer.drawSpriteProgress(testTexture, 680.0f, 100.0f, 32.0f, 128.0f, 0.50f, false);

        // Edge cases de progreso: 0% y 100%
        renderer.drawSpriteProgress(testTexture, 450.0f, 290.0f, 200.0f, 24.0f, 0.0f, true);
        renderer.drawSpriteProgress(testTexture, 450.0f, 330.0f, 200.0f, 24.0f, 1.0f, true);

        renderer.end();
        assert(glGetError() == GL_NO_ERROR && "Pase de sprites y progress bars sin errores de OpenGL");
        std::cout << "  [CHECK 3] Renderizado de sprites UV y barras de progreso horizontal/vertical OK." << std::endl;

        // ---------------------------------------------------------------------
        // 5.1 Verificacion de 9-Slice y 3-Slice Horizontal
        // ---------------------------------------------------------------------
        renderer.begin(1280.0f, 720.0f);
        // 9-Slice: marco escalado con esquinas fijas
        renderer.drawNineSlice(testTexture, 50.0f, 400.0f, 250.0f, 80.0f, 16.0f, 16.0f);

        // 3-Slice Horizontal de textura única
        renderer.drawThreeSliceHorizontal(testTexture, 320.0f, 400.0f, 200.0f, 32.0f, 12.0f);

        // 3-Slice Horizontal modular con 3 texturas (usando la misma textura para test)
        renderer.drawThreeSliceHorizontal(testTexture, testTexture, testTexture, 540.0f, 400.0f, 200.0f, 32.0f, 16.0f);

        // 3-Slice Progress: progreso 25%, 50%, 75%
        renderer.drawThreeSliceProgress(testTexture, testTexture, testTexture, 760.0f, 400.0f, 180.0f, 28.0f, 0.25f, 14.0f);
        renderer.drawThreeSliceProgress(testTexture, testTexture, testTexture, 760.0f, 440.0f, 180.0f, 28.0f, 0.50f, 14.0f);
        renderer.drawThreeSliceProgress(testTexture, testTexture, testTexture, 760.0f, 480.0f, 180.0f, 28.0f, 0.85f, 14.0f);

        renderer.end();
        assert(glGetError() == GL_NO_ERROR && "Pase de 9-Slice y 3-Slice sin errores de OpenGL");
        std::cout << "  [CHECK 3.1] Renderizado de 9-Slice y 3-Slice Horizontal (modular y progreso) OK." << std::endl;

        // ---------------------------------------------------------------------
        // 6. Verificacion de lotes entrelazados con cambios automaticos de textura
        // ---------------------------------------------------------------------
        renderer.begin(1280.0f, 720.0f);
        renderer.drawRect(10.0f, 10.0f, 50.0f, 50.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        renderer.drawSprite(testTexture, 70.0f, 10.0f, 50.0f, 50.0f);
        renderer.drawText(130.0f, 10.0f, "INTERLEAVED", 1.0f);
        renderer.drawSprite(testTexture, 250.0f, 10.0f, 50.0f, 50.0f);
        renderer.drawRect(310.0f, 10.0f, 50.0f, 50.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        renderer.end();

        assert(glGetError() == GL_NO_ERROR && "Cambio y vaciado automatico de lotes por textura OK");
        std::cout << "  [CHECK 4] Batching dinamico con cambios de textura y vaciado automatico OK." << std::endl;

        // ---------------------------------------------------------------------
        // 7. Verificacion de semantica de movimiento (Move semantics) y RAII
        // ---------------------------------------------------------------------
        ui::UIRenderer movedRenderer = std::move(renderer);
        assert(movedRenderer.isInitialized() && "El objeto receptor debe estar inicializado");
        assert(!renderer.isInitialized() && "El objeto donante debe quedar en estado no inicializado");

        movedRenderer.begin(1280.0f, 720.0f);
        movedRenderer.drawRect(0.0f, 0.0f, 64.0f, 64.0f, glm::vec4(1.0f));
        movedRenderer.end();
        assert(glGetError() == GL_NO_ERROR && "Pase con objeto movido exitoso");

        movedRenderer.destroy();
        assert(!movedRenderer.isInitialized() && "destroy() debe dejar isInitialized en false");
        std::cout << "  [CHECK 5] Semantica de movimiento RAII y destroy() validados con exito." << std::endl;

        // ---------------------------------------------------------------------
        // 8. Verificacion de UIUtils (UIAnchor, layout, sanitizacion y wrapText)
        // ---------------------------------------------------------------------
        {
            // Anchors
            glm::vec2 tl = ui::computeAnchoredPosition(ui::UIAnchor::TOP_LEFT, 100.0f, 50.0f, 1280.0f, 720.0f, 10.0f, 20.0f);
            assert(tl.x == 10.0f && tl.y == 20.0f);
            (void)tl;

            glm::vec2 c = ui::computeAnchoredPosition(ui::UIAnchor::CENTER, 200.0f, 100.0f, 1280.0f, 720.0f);
            assert(c.x == (1280.0f - 200.0f) * 0.5f && c.y == (720.0f - 100.0f) * 0.5f);
            (void)c;

            glm::vec2 br = ui::computeAnchoredPosition(ui::UIAnchor::BOTTOM_RIGHT, 300.0f, 150.0f, 1280.0f, 720.0f, -10.0f, -20.0f);
            assert(br.x == 1280.0f - 300.0f - 10.0f && br.y == 720.0f - 150.0f - 20.0f);
            (void)br;

            // Sanitize text
            std::string clean = ui::sanitizeText("Camión Rápido #1");
            assert(clean == "CAMION RAPIDO #1");
            (void)clean;

            // Wrap text
            auto wrapped = ui::wrapText("UN TEXTO MUY LARGO QUE DEBE AJUSTARSE", 120.0f, 1.0f);
            assert(wrapped.size() > 1);
            (void)wrapped;

            // Resolve path
            std::string resolved = ui::resolveAssetPath("assets/textures/colormap.png");
            assert(!resolved.empty());
            (void)resolved;

            std::cout << "  [CHECK 6] Utilidades de UI (UIAnchor, layout, sanitizacion y wrap) validadas." << std::endl;
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Todos los tests de UIRenderer completados 100%!  " << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
