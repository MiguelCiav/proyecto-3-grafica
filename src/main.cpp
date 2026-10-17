#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int main() {
    std::cout << "=====================================================" << std::endl;
    std::cout << "  Prototipo Mario Kart - CGI Proyecto #3 (UCV)      " << std::endl;
    std::cout << "  Fase 1 - Issue #1: Setup Inicial y Dependencias   " << std::endl;
    std::cout << "=====================================================" << std::endl;

    // Verificar matemáticas con GLM
    glm::vec3 pos(1.0f, 2.0f, 3.0f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
    std::cout << "[OK] GLM math initialized: translation test vector (" 
              << model[3][0] << ", " << model[3][1] << ", " << model[3][2] << ")" << std::endl;

    // Inicializar GLFW para validar enlace
    if (!glfwInit()) {
        std::cerr << "[ERROR] Fallo al inicializar GLFW." << std::endl;
        return -1;
    }
    std::cout << "[OK] GLFW inicializado correctamente: version " << glfwGetVersionString() << std::endl;

    // Configurar hints de OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Ventana oculta para el test headless/base

    GLFWwindow* window = glfwCreateWindow(640, 480, "Prototipo Mario Kart - Init Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "[AVISO] No se pudo crear contexto con ventana (posible entorno headless sin display X11)." << std::endl;
        glfwTerminate();
        std::cout << "[OK] Enlace de GLFW y GLM completado exitosamente." << std::endl;
        return 0;
    }

    glfwMakeContextCurrent(window);

    // Inicializar GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "[ERROR] Fallo al inicializar GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "[OK] Contexto OpenGL creado y GLAD inicializado:" << std::endl;
    std::cout << "     - Fabricante: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "     - Renderer:   " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "     - Version:    " << glGetString(GL_VERSION) << std::endl;

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "=====================================================" << std::endl;
    std::cout << "  Criterios de Aceptacion Issue #1 Superados!       " << std::endl;
    std::cout << "=====================================================" << std::endl;
    return 0;
}
