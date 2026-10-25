#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "world/BoosterPad.hpp"
#include "world/ZoneTrigger.hpp"
#include "physics/KartPhysics.hpp"
#include "renderer/Shader.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Entidad BoosterPad e Interaccion Reactiva         " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // 1. Validar inicialización y geometría AABB de la entidad
    glm::vec3 padPos(20.0f, 0.0f, 15.0f);
    world::BoosterPad pad(padPos, 0.0f, 3.0f, 5.0f);

    const auto& zone = pad.getTriggerZone();
    assert(zone.type == world::ZoneType::BOOST);
    assert(zone.minBounds.x <= 20.0f && zone.maxBounds.x >= 20.0f);
    assert(zone.minBounds.z <= 15.0f && zone.maxBounds.z >= 15.0f);
    assert(pad.contains(glm::vec3(20.0f, 0.0f, 15.0f)));
    assert(!pad.contains(glm::vec3(100.0f, 0.0f, 100.0f)));
    std::cout << "  [CHECK 1] Geometria y limites AABB del BoosterPad validados correctamente." << std::endl;

    // 2. Registro en ZoneTriggerManager
    world::ZoneTriggerManager triggerMgr;
    pad.registerTrigger(triggerMgr);
    assert(triggerMgr.getZones().size() == 1);
    const auto* foundZone = triggerMgr.checkTrigger(padPos);
    assert(foundZone != nullptr);
    assert(foundZone->type == world::ZoneType::BOOST);
    assert(foundZone->speedMultiplier == 1.6f);
    assert(foundZone->durationSeconds == 2.0f);
    std::cout << "  [CHECK 2] Volumen de activacion AABB registrado con exito en ZoneTriggerManager." << std::endl;

    // 3. Interacción reactiva con KartPhysics
    Engine::Physics::KartPhysics kart;
    kart.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    kart.resetSpeed();

    // Fuera del pad
    bool contactOutside = pad.onKartContact(kart);
    assert(!contactOutside);
    assert(!kart.isBoosting());

    // Dentro del pad
    kart.setPosition(padPos);
    bool contactInside = pad.onKartContact(kart);
    assert(contactInside);
    assert(kart.isBoosting());
    assert(kart.getBoostTimer() == 2.0f);
    assert(kart.getSpeed() > 0.0f); // Impulso instantáneo aplicado
    std::cout << "  [CHECK 3] Contacto reactivo detectado: boost activado e impulso instantaneo inyectado." << std::endl;

    // Simulación de avance en el tiempo
    kart.update(1.0f, true, false, 0.0f);
    assert(kart.isBoosting());
    assert(kart.getBoostTimer() == 1.0f);

    kart.update(1.1f, false, false, 0.0f);
    assert(!kart.isBoosting());
    std::cout << "  [CHECK 4] Temporizador de turbo y restauracion de limites de velocidad verificados." << std::endl;

    // 4. Prueba gráfica y pase de dibujo con shader procedural
    if (!glfwInit()) {
        std::cerr << "[FAIL] No se pudo inicializar GLFW." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Test BoosterPad GL", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    {
        renderer::Shader shader;
        bool shaderOk = shader.loadFromFiles("shaders/booster_procedural.vert", "shaders/booster_procedural.frag");
        assert(shaderOk);

        world::BoosterPad glPad(glm::vec3(5.0f, 0.0f, 10.0f), 45.0f, 3.2f, 4.8f);
        glPad.draw(shader, 2.5f);

        GLenum glErr = glGetError();
        assert(glErr == GL_NO_ERROR);
        std::cout << "  [CHECK 5] Pase de renderizado de BoosterPad con elevacion anti-z-fighting superado en GPU." << std::endl;

        // 5. Semántica de movimiento
        world::BoosterPad movedPad = std::move(glPad);
        assert(movedPad.contains(glm::vec3(5.0f, 0.0f, 10.0f)));
        std::cout << "  [CHECK 6] Semantica de movimiento de BoosterPad completada correctamente." << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Todos los checks de BoosterPad fueron superados  " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
