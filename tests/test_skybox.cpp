#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderer/Skybox.hpp"
#include "renderer/Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cassert>

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Sistema de Skybox con Cubemap 3D                  " << std::endl;
    std::cout << "==========================================================" << std::endl;

    if (!glfwInit()) {
        std::cerr << "[FAIL] No se pudo inicializar GLFW para el test." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Test Skybox", nullptr, nullptr);
    if (!window) {
        std::cerr << "[FAIL] No se pudo crear contexto OpenGL para el test." << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "[FAIL] No se pudo inicializar GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    std::cout << "  [OK] Contexto OpenGL " << glGetString(GL_VERSION) << " activo." << std::endl;

    // 1. Validar carga y compilación de shaders skybox
    renderer::Skybox skybox;
    bool shaderLoaded = skybox.loadShader("shaders/skybox.vert", "shaders/skybox.frag");
    assert(shaderLoaded);
    assert(skybox.getShader() != nullptr && skybox.getShader()->isValid());
    std::cout << "  [CHECK 1] Shaders skybox.vert y skybox.frag compilados con éxito." << std::endl;

    // 2. Cargar las 6 caras del cubemap desde assets/textures/skybox
    bool cubemapLoaded = skybox.loadFromDirectory("assets/textures/skybox", ".png");
    assert(cubemapLoaded);
    assert(skybox.isLoaded());
    assert(skybox.getTextureID() != 0);
    assert(skybox.getVAO() != 0);
    std::cout << "  [CHECK 2] Cubemap 3D cargado correctamente (Texture ID: "
              << skybox.getTextureID() << ", VAO: " << skybox.getVAO() << ")." << std::endl;

    // 3. Validar parámetros de textura GL_CLAMP_TO_EDGE y GL_LINEAR
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.getTextureID());
    GLint wrapS = 0, wrapT = 0, wrapR = 0;
    glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, &wrapS);
    glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, &wrapT);
    glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, &wrapR);
    assert(wrapS == GL_CLAMP_TO_EDGE);
    assert(wrapT == GL_CLAMP_TO_EDGE);
    assert(wrapR == GL_CLAMP_TO_EDGE);
    std::cout << "  [CHECK 3] Parametros GL_CLAMP_TO_EDGE validados en ejes S, T, R." << std::endl;

    // 4. Probar pase de dibujo con matrices de camara simuladas
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(10.0f, 5.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    skybox.draw(view, projection);

    GLint currentDepthFunc = 0;
    glGetIntegerv(GL_DEPTH_FUNC, &currentDepthFunc);
    assert(currentDepthFunc == GL_LESS);
    std::cout << "  [CHECK 4] Pase de dibujo ejecutado y depth function restaurada a GL_LESS." << std::endl;

    // 5. Validar semántica de movimiento
    renderer::Skybox movedSkybox = std::move(skybox);
    assert(movedSkybox.isLoaded());
    assert(!skybox.isLoaded());
    std::cout << "  [CHECK 5] Semantica de movimiento completada correctamente." << std::endl;

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Todos los tests del Skybox fueron superados      " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
