#include <iostream>
#include <cassert>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vfx/KartParticleEmitter.hpp"
#include "physics/KartPhysics.hpp"
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
    std::cout << "  TEST: Emisores de Particulas para Kart y Turbo (VFX)   " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // TEST 1: Transformación geométrica de escapes al espacio del mundo
    // -------------------------------------------------------------------------
    {
        vfx::KartParticleEmitter emitter;
        glm::vec3 kartPos(10.0f, 0.0f, 20.0f);

        // Con yaw = 0 grados (avance hacia +Z), el escape trasero debe situarse en Z < 20.0
        glm::vec3 exhaustLeft = emitter.getExhaustWorldPosition(kartPos, 0.0f, true);
        glm::vec3 exhaustRight = emitter.getExhaustWorldPosition(kartPos, 0.0f, false);

        assert(exhaustLeft.z < kartPos.z && "Escape trasero debe ubicarse detrás del centro del kart en Z");
        assert(exhaustLeft.x < exhaustRight.x && "Escape izquierdo debe tener menor X que el derecho");
        assert(std::abs(exhaustLeft.y - 0.22f) < 0.001f);

        // Con yaw = 90 grados (avance hacia +X), el escape trasero debe situarse en X < 10.0
        glm::vec3 exhaustYaw90 = emitter.getExhaustWorldPosition(kartPos, 90.0f, true);
        assert(exhaustYaw90.x < kartPos.x && "Escape trasero a 90 grados debe ubicarse detrás en el eje X");

        std::cout << "  [CHECK 1] Coordenadas de escapes transformadas al espacio del mundo correctamente." << std::endl;
    }

    // -------------------------------------------------------------------------
    // TEST 2: Cadencia reactiva de humo (aceleración vs ralentí)
    // -------------------------------------------------------------------------
    {
        vfx::KartParticleEmitter emitter;
        Engine::Physics::KartPhysics physics;
        physics.setPosition(glm::vec3(0.0f));

        // 1. Simulación en ralentí durante 0.5s
        for (int i = 0; i < 50; ++i) {
            emitter.update(0.01f, physics, false);
        }
        size_t idleCount = emitter.getSmokeSystem().getActiveParticleCount();
        assert(idleCount >= 1 && idleCount <= 3 && "En ralentí la emisión debe ser pausada y baja");

        // 2. Simulación en aceleración durante 0.5s
        physics.update(0.5f, true, false, 0.0f);
        emitter.getSmokeSystem().clear();

        for (int i = 0; i < 50; ++i) {
            emitter.update(0.01f, physics, true);
        }
        size_t accelCount = emitter.getSmokeSystem().getActiveParticleCount();
        assert(accelCount > idleCount && "Acelerando debe emitir considerablemente más partículas de humo");

        std::cout << "  [CHECK 2] Cadencia de humo reactiva a la aceleracion (" << accelCount 
                  << " particulas acelerando vs " << idleCount << " en ralenti)." << std::endl;
    }

    // -------------------------------------------------------------------------
    // TEST 3: Ráfaga de Turbo (Boost Burst) y desacople del espacio del mundo
    // -------------------------------------------------------------------------
    {
        vfx::KartParticleEmitter emitter;
        Engine::Physics::KartPhysics physics;
        physics.setPosition(glm::vec3(0.0f));

        assert(emitter.getBoostSystem().getActiveParticleCount() == 0);

        // Activar impulso de booster
        physics.applyBoost(2.0f, 1.6f, 8.0f);
        assert(physics.isBoosting());

        // Al procesar el update debe activarse la ráfaga explosiva
        emitter.update(0.016f, physics, true);
        size_t boostParticles = emitter.getBoostSystem().getActiveParticleCount();
        assert(boostParticles >= 30 && "La rafaga de turbo debe disparar una explosion de particulas");

        // Verificar que las partículas se desprenden en coordenadas del mundo
        const auto& particles = emitter.getBoostSystem().getParticles();
        glm::vec3 particlePosBefore = particles[0].position;

        // Desplazar el kart bruscamente 20 metros hacia adelante
        physics.setPosition(glm::vec3(0.0f, 0.0f, 20.0f));
        emitter.update(0.016f, physics, true);

        // La partícula previa no debe haberse movido rígidamente con el kart
        assert(std::abs(particles[0].position.z - particlePosBefore.z) < 1.0f &&
               "Las particulas deben permanecer libres en el espacio global del mundo");

        std::cout << "  [CHECK 3] Rafaga de turbo explosiva (" << boostParticles 
                  << " particulas) y desacople del espacio global del mundo verificados." << std::endl;
    }

    // -------------------------------------------------------------------------
    // TEST 4: Inicialización GPU y pase de renderizado completo
    // -------------------------------------------------------------------------
    if (!glfwInit()) {
        std::cerr << "[ERROR] Falló inicialización de GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Test Kart Emitter", nullptr, nullptr);
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

    {
        vfx::KartParticleEmitter emitter;
        assert(emitter.init());
        assert(emitter.loadShaders("shaders/particle.vert", "shaders/particle.frag"));
        checkGlError("Inicialización de shaders y buffers de KartParticleEmitter");

        Engine::Physics::KartPhysics physics;
        physics.applyBoost(2.0f);

        for (int i = 0; i < 10; ++i) {
            emitter.update(0.016f, physics, true);
        }

        glm::mat4 proj = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, -5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        emitter.render(view, proj);
        checkGlError("Pase de renderizado integrado de humo y turbo de KartParticleEmitter");

        std::cout << "  [CHECK 4] Renderizado dual (humo difuso + turbo aditivo) ejecutado con exito." << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Emisores de Particulas del Kart validados!       " << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
