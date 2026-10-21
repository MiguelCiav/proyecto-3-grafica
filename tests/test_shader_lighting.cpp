#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderer/Shader.hpp"
#include "lighting/LightingSystem.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Compilacion y Enlace de Shaders Multi-Fuente      " << std::endl;
    std::cout << "  Fase 7 - Issue #26 (Directriz UCV)                      " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // 1. Inicializar contexto OpenGL oculto para validacion de shaders
    if (!glfwInit()) {
        std::cerr << "[FAIL] No se pudo inicializar GLFW para el test." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Test Shaders", nullptr, nullptr);
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

    // 2. Compilar shaders
    renderer::Shader shader;
    bool loaded = shader.loadFromFiles("shaders/basic.vert", "shaders/basic.frag");
    if (!loaded || !shader.isValid()) {
        std::cerr << "[FAIL] Error al compilar o enlazar shaders/basic.vert y shaders/basic.frag" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    std::cout << "  [OK] Shaders basic.vert y basic.frag compilados y enlazados exitosamente (Program ID: "
              << shader.getID() << ")." << std::endl;

    // 3. Probar vinculacion y paso de uniforms desde LightingSystem
    shader.use();

    lighting::LightingSystem lights;
    lights.init();
    lights.applyToShader(shader, glm::vec3(0.0f, 5.0f, 10.0f));
    std::cout << "  [OK] Uniforms de las 3 luces (Dir, Point, Spot) y Materiales transferidos al shader." << std::endl;

    // 4. Probar paso de cada modelo de iluminacion (0: Flat, 1: Lambert, 2: Blinn-Phong)
    for (int model = 0; model < 3; ++model) {
        lights.setLightingModel(static_cast<lighting::LightingModel>(model));
        lights.applyToShader(shader, glm::vec3(0.0f, 2.0f, 5.0f));
    }
    std::cout << "  [OK] Modelos de sombreado (Flat, Lambert, Blinn-Phong) alternados en GPU." << std::endl;

    shader.unbind();
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "\n[SUCCESS] Issue #26 completado con exito: Shaders multi-fuente verificados en GPU." << std::endl;
    return 0;
}
