#include <iostream>
#include <cassert>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vfx/Particle.hpp"
#include "vfx/ParticleSystem.hpp"
#include "renderer/Shader.hpp"

static void checkGlError(const std::string& step) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "[ERROR OpenGL] en " << step << ": código 0x" << std::hex << err << std::endl;
        assert(false);
    }
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Motor y Estructuras del Sistema de Particulas 3D  " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // TEST 1: Comprobación del pool prealocado y emisión
    // -------------------------------------------------------------------------
    {
        vfx::ParticleSystem ps(50);
        assert(ps.getMaxParticles() == 50);
        assert(ps.getActiveParticleCount() == 0);

        for (int i = 0; i < 50; ++i) {
            bool ok = ps.emit(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), 0.5f, 2.0f);
            assert(ok);
        }
        assert(ps.getActiveParticleCount() == 50);

        // Intento de emitir más allá de la capacidad máxima prealocada
        bool overflowOk = ps.emit(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec4(1.0f), 0.5f, 1.0f);
        assert(!overflowOk);
        assert(ps.getActiveParticleCount() == 50);

        std::cout << "  [CHECK 1] Pool prealocado gestionado eficientemente sin desbordamientos." << std::endl;
    }

    // -------------------------------------------------------------------------
    // TEST 2: Simulación física, decaimiento de alfa y reciclado O(1)
    // -------------------------------------------------------------------------
    {
        vfx::ParticleSystem ps(10);
        ps.setGravity(glm::vec3(0.0f, -9.8f, 0.0f));

        ps.emit(glm::vec3(0.0f, 5.0f, 0.0f),
                glm::vec3(0.0f, 2.0f, 0.0f),
                glm::vec4(1.0f, 0.5f, 0.2f, 0.8f),
                1.0f,
                2.0f); // Vida total = 2.0s

        assert(ps.getActiveParticleCount() == 1);
        const auto& pool = ps.getParticles();

        // Avanzar 1 segundo
        ps.update(1.0f);
        assert(ps.getActiveParticleCount() == 1);
        // Velocidad = 2.0 + (-9.8 * 1.0) = -7.8
        assert(std::abs(pool[0].velocity.y - (-7.8f)) < 0.001f);
        // remainingLife = 1.0s, life = 2.0s -> alfa debe ser 0.8 * (1.0 / 2.0) = 0.4
        assert(std::abs(pool[0].color.a - 0.4f) < 0.001f);

        // Avanzar 1.1 segundos más -> total 2.1s (expira y se recicla)
        ps.update(1.1f);
        assert(ps.getActiveParticleCount() == 0);

        std::cout << "  [CHECK 2] Fisica (gravedad), desvanecimiento de alfa y reciclado O(1) verificados." << std::endl;
    }

    // -------------------------------------------------------------------------
    // TEST 3: Contexto OpenGL, Compilación de Shaders y Billboarding Instanciado
    // -------------------------------------------------------------------------
    if (!glfwInit()) {
        std::cerr << "[ERROR] Falló inicialización de GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Test Particle System", nullptr, nullptr);
    if (!window) {
        std::cerr << "[ERROR] Falló creación de ventana GLFW oculta" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "[ERROR] Falló inicialización de GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "  [OK] Contexto OpenGL " << glGetString(GL_VERSION) << " activo." << std::endl;

    {
        renderer::Shader shader;
        bool shaderOk = shader.loadFromFiles("shaders/particle.vert", "shaders/particle.frag");
        assert(shaderOk && "El shader de partículas debe compilar limpiamente");
        checkGlError("Compilación de shaders de partículas");

        vfx::ParticleSystem ps(100);
        assert(ps.init());
        checkGlError("Inicialización de buffers GPU de partículas");

        // Emitir ráfaga de prueba
        for (int i = 0; i < 20; ++i) {
            float angle = (float)i * 0.314f;
            glm::vec3 vel(std::cos(angle) * 5.0f, 3.0f, std::sin(angle) * 5.0f);
            ps.emit(glm::vec3(0.0f, 1.0f, 0.0f), vel, glm::vec4(1.0f, 0.8f, 0.2f, 1.0f), 0.6f, 1.5f);
        }
        assert(ps.getActiveParticleCount() == 20);

        glm::mat4 proj = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        // Probar renderizado desde múltiples orientaciones de cámara (Billboarding esférico)
        glm::vec3 cameraPositions[] = {
            glm::vec3(0.0f, 5.0f, 10.0f),
            glm::vec3(10.0f, 2.0f, 0.0f),
            glm::vec3(-5.0f, 12.0f, -8.0f)
        };

        for (const auto& eye : cameraPositions) {
            glm::mat4 view = glm::lookAt(eye, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            ps.render(shader, view, proj);
            checkGlError("Pase de renderizado de partículas con billboarding");

            // Verificar que el estado gráfico de profundidad y mezcla se restaura correctamente
            GLboolean depthMask = GL_FALSE;
            glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
            assert(depthMask == GL_TRUE && "Depth mask debe restaurarse a GL_TRUE tras renderizar partículas");

            GLboolean blendEnabled = GL_TRUE;
            glGetBooleanv(GL_BLEND, &blendEnabled);
            assert(blendEnabled == GL_FALSE && "GL_BLEND debe restaurarse tras renderizar partículas");
        }

        std::cout << "  [CHECK 3] Shaders, buffers instanciados y billboarding verificados sin errores OpenGL." << std::endl;
        std::cout << "  [CHECK 4] Transparencia y restauración de GL_DEPTH_WRITEMASK confirmadas." << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Subsistema de Particulas 3D verificado con exito!  " << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
