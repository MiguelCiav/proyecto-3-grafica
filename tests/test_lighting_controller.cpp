#include "lighting/LightManager.hpp"
#include "lighting/LightingController.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: LightingController y Faros Dinamicos del Kart     " << std::endl;
    std::cout << "  Seguimiento de faros y conmutacion de modelos           " << std::endl;
    std::cout << "==========================================================" << std::endl;

    lighting::LightManager lightManager;
    lightManager.init();

    lighting::LightingController controller;
    controller.init(&lightManager);

    // -------------------------------------------------------------
    // Faros del kart (Spotlight): seguimiento de trayectoria y giros
    // -------------------------------------------------------------
    glm::vec3 kartPos(15.0f, 0.0f, 25.0f);
    glm::vec3 kartForward(0.0f, 0.0f, 1.0f); // Mirando al norte

    controller.update(kartPos, kartForward, 0.016f);
    const auto& spot = lightManager.getSpotLight();

    // spot.position = kartPos + forward * 0.75 + (0.0, 0.4, 0.0)
    assert(spot.position.x == kartPos.x);
    assert(spot.position.y == kartPos.y + 0.4f);
    assert(spot.position.z == kartPos.z + 0.75f);
    assert(spot.direction == kartForward);
    std::cout << "  [CHECK 1.1] Faros del kart correctamente posicionados en morro frontal (+0.75 Z, +0.4 Y)." << std::endl;

    // Giro hacia el este (1, 0, 0)
    glm::vec3 kartTurning(1.0f, 0.0f, 0.0f);
    controller.update(kartPos, kartTurning, 0.016f);
    assert(lightManager.getSpotLight().position.x == kartPos.x + 0.75f);
    assert(lightManager.getSpotLight().direction == kartTurning);
    std::cout << "  [CHECK 1.2] Faros del kart giran reactivamente con la orientacion del vehiculo." << std::endl;

    // -------------------------------------------------------------
    // Conmutacion de modelos de sombreado (Flat, Lambert, Blinn-Phong)
    // -------------------------------------------------------------
    controller.setLightingModel(lighting::LightingModel::FLAT);
    assert(lightManager.getLightingModel() == lighting::LightingModel::FLAT);

    controller.setLightingModel(lighting::LightingModel::LAMBERT);
    assert(lightManager.getLightingModel() == lighting::LightingModel::LAMBERT);

    controller.setLightingModel(lighting::LightingModel::BLINN_PHONG);
    assert(lightManager.getLightingModel() == lighting::LightingModel::BLINN_PHONG);
    std::cout << "  [CHECK 2] Conmutacion instantanea entre Flat (0), Lambert (1) y Blinn-Phong (2) verificada." << std::endl;

    // -------------------------------------------------------------
    // Alternancia de faros encendidos y apagados (Tecla L)
    // -------------------------------------------------------------
    assert(controller.areHeadlightsOn() == true);
    assert(lightManager.getSpotLight().enabled == true);

    controller.toggleHeadlights(); // Apagar
    assert(controller.areHeadlightsOn() == false);
    assert(lightManager.getSpotLight().enabled == false);

    controller.toggleHeadlights(); // Encender
    assert(controller.areHeadlightsOn() == true);
    assert(lightManager.getSpotLight().enabled == true);
    std::cout << "  [CHECK 3] Alternancia de faros encendidos/apagados (Tecla L) verificada." << std::endl;

    std::cout << "\n==========================================================" << std::endl;
    std::cout << "  [SUCCESS] Pruebas de LightingController superadas exitosamente! " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
