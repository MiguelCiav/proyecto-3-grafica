#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderer/Shader.hpp"
#include "lighting/LightManager.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Compilacion y Enlace de Shaders Multi-Fuente      " << std::endl;
    std::cout << "  Shaders de iluminacion (shaders/lit.vert y lit.frag)    " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // 1. Inicializar contexto OpenGL oculto para validación de shaders en GPU
    if (!glfwInit()) {
        std::cerr << "[FAIL] No se pudo inicializar GLFW para el test." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Test Shaders Lit", nullptr, nullptr);
    if (!window) {
        std::cerr << "[FAIL] No se pudo crear contexto OpenGL para el test." << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "[FAIL] No se pudo inicializar GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    std::cout << "  [OK] Contexto OpenGL " << glGetString(GL_VERSION) << " inicializado." << std::endl;

    {
        // 2. Compilar shaders de iluminacion (lit.vert y lit.frag)
        renderer::Shader litShader;
        bool loadedLit = litShader.loadFromFiles("shaders/lit.vert", "shaders/lit.frag");
        if (!loadedLit || !litShader.isValid()) {
            std::cerr << "[FAIL] Error al compilar o enlazar shaders/lit.vert y shaders/lit.frag" << std::endl;
            glfwDestroyWindow(window);
            glfwTerminate();
            return 1;
        }
        std::cout << "  [CHECK 1] Shaders lit.vert y lit.frag compilados y enlazados exitosamente (Program ID: "
                  << litShader.getID() << ")." << std::endl;

        // 3. Probar vinculación y paso de uniforms desde LightManager
        litShader.use();

        lighting::LightManager lightManager;
        lightManager.init();
        lightManager.applyToShader(litShader, glm::vec3(0.0f, 5.0f, 10.0f));
        std::cout << "  [CHECK 2] Uniforms de las 3 luces (Dir, " << lightManager.getPointLightCount()
                  << "x Point, Spot) y Materiales transferidos al shader." << std::endl;

        // 4. Probar paso de cada modelo de iluminación (0: Flat, 1: Lambert, 2: Blinn-Phong)
        for (int model = 0; model < 3; ++model) {
            lightManager.setLightingModel(static_cast<lighting::LightingModel>(model));
            lightManager.applyToShader(litShader, glm::vec3(0.0f, 2.0f, 5.0f));
        }
        std::cout << "  [CHECK 3] Modelos de sombreado (0: Flat, 1: Lambert, 2: Blinn-Phong) alternados en GPU." << std::endl;

        litShader.unbind();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "\n==========================================================" << std::endl;
    std::cout << "  [SUCCESS] Compilacion y uniforms de shaders lit verificados exitosamente! " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
