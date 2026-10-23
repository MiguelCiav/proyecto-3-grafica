#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderer/Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cassert>

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Shader de Texturizado Procedural de Boosters      " << std::endl;
    std::cout << "==========================================================" << std::endl;

    if (!glfwInit()) {
        std::cerr << "[FAIL] No se pudo inicializar GLFW para el test." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Test Booster Procedural Shader", nullptr, nullptr);
    if (!window) {
        std::cerr << "[FAIL] No se pudo crear contexto OpenGL." << std::endl;
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

    // 1. Cargar y compilar shaders procedimentales
    renderer::Shader boosterShader;
    bool loaded = boosterShader.loadFromFiles("shaders/booster_procedural.vert", "shaders/booster_procedural.frag");
    if (!loaded || !boosterShader.isValid()) {
        std::cerr << "[FAIL] Error al compilar o enlazar shaders de booster procedural." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    std::cout << "  [CHECK 1] Shaders booster_procedural.vert y booster_procedural.frag compilados con exito (Program ID: "
              << boosterShader.getID() << ")." << std::endl;

    // 2. Validar asignación de uniforms de configuración y animación
    boosterShader.use();
    boosterShader.setFloat("u_Time", 1.5f);
    boosterShader.setFloat("u_Speed", 3.2f);
    boosterShader.setFloat("u_Repetitions", 4.0f);
    boosterShader.setFloat("u_ArrowThickness", 0.22f);
    boosterShader.setFloat("u_EmissionIntensity", 2.5f);
    boosterShader.setFloat("u_PulseSpeed", 4.5f);
    boosterShader.setVec3("u_BaseColor", glm::vec3(0.1f, 0.1f, 0.12f));
    boosterShader.setVec3("u_EnergyColor", glm::vec3(0.0f, 0.95f, 1.0f));
    boosterShader.setVec3("u_CoreColor", glm::vec3(1.0f, 1.0f, 1.0f));

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    boosterShader.setMat4("u_Model", model);
    boosterShader.setMat4("u_View", view);
    boosterShader.setMat4("u_Projection", projection);

    GLenum err = glGetError();
    assert(err == GL_NO_ERROR);
    std::cout << "  [CHECK 2] Uniforms matematicos de configuracion y matrices transferidos sin errores." << std::endl;

    // 3. Probar renderizado de un quad con coordenadas UV
    float quadVertices[] = {
        // Posición            // Normal             // UV
        -1.0f, 0.0f, -2.0f,    0.0f, 1.0f, 0.0f,     0.0f, 0.0f,
         1.0f, 0.0f, -2.0f,    0.0f, 1.0f, 0.0f,     1.0f, 0.0f,
         1.0f, 0.0f,  2.0f,    0.0f, 1.0f, 0.0f,     1.0f, 1.0f,

        -1.0f, 0.0f, -2.0f,    0.0f, 1.0f, 0.0f,     0.0f, 0.0f,
         1.0f, 0.0f,  2.0f,    0.0f, 1.0f, 0.0f,     1.0f, 1.0f,
        -1.0f, 0.0f,  2.0f,    0.0f, 1.0f, 0.0f,     0.0f, 1.0f
    };

    unsigned int vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // a_Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0));
    // a_Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    // a_TexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    err = glGetError();
    assert(err == GL_NO_ERROR);
    std::cout << "  [CHECK 3] Dibujo de quad procedural ejecutado correctamente en GPU." << std::endl;

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Todos los checks del shader procedural aprobados " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
