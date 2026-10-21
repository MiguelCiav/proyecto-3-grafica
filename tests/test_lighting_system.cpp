#include "lighting/LightingSystem.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Sistema de Iluminacion y Estructuras Multi-Fuente  " << std::endl;
    std::cout << "  Fase 7 - Issue #25 (Directriz UCV)                      " << std::endl;
    std::cout << "==========================================================" << std::endl;

    lighting::LightingSystem lights;
    lights.init();

    // 1. Validar Luz Direccional (Sol)
    const auto& dir = lights.getDirectionalLight();
    assert(dir.enabled == true);
    assert(glm::length(dir.direction) > 0.99f && glm::length(dir.direction) < 1.01f);
    assert(dir.diffuse.r > 0.0f && dir.ambient.r > 0.0f);
    std::cout << "  [OK] 1. Luz Direccional (Sol) inicializada correctamente." << std::endl;

    // 2. Validar Luz Puntual (Farola)
    const auto& pt = lights.getPointLight();
    assert(pt.enabled == true);
    assert(pt.constant == 1.0f);
    assert(pt.linear > 0.0f);
    assert(pt.quadratic > 0.0f);
    std::cout << "  [OK] 2. Luz Puntual (Farola) inicializada con atenuacion cuadratica." << std::endl;

    // 3. Validar Luz Focal (Spot / Faros Kart)
    const auto& spot = lights.getSpotLight();
    assert(spot.enabled == true);
    assert(spot.cutOff > spot.outerCutOff); // cutOff es coseno de angulo mas agudo -> mayor valor
    assert(spot.cutOff > 0.9f);
    std::cout << "  [OK] 3. Luz Focal (Faros Kart) con conos interior/exterior validos." << std::endl;

    // 4. Validar Modelos de Iluminacion y Conmutacion
    assert(lights.getLightingModel() == lighting::LightingModel::BLINN_PHONG);
    lights.cycleNextModel();
    assert(lights.getLightingModel() == lighting::LightingModel::FLAT);
    lights.cycleNextModel();
    assert(lights.getLightingModel() == lighting::LightingModel::LAMBERT);
    lights.cycleNextModel();
    assert(lights.getLightingModel() == lighting::LightingModel::BLINN_PHONG);
    std::cout << "  [OK] 4. Conmutacion circular de modelos de iluminacion verificada (Blinn-Phong -> Flat -> Lambert)." << std::endl;

    // 5. Validar actualizacion dinamica de faros segun movimiento del kart
    glm::vec3 kartPos(10.0f, 0.0f, 15.0f);
    glm::vec3 kartForward(0.0f, 0.0f, 1.0f);
    lights.updateKartHeadlights(kartPos, kartForward);
    assert(lights.getSpotLight().position.y > 0.3f);
    assert(lights.getSpotLight().position.z > kartPos.z);
    std::cout << "  [OK] 5. Seguimiento cinematico de faros del kart verificado." << std::endl;

    // 6. Validar toggles individuales de fuentes luminosas
    lights.toggleDirectionalLight();
    assert(lights.getDirectionalLight().enabled == false);
    lights.toggleDirectionalLight();
    assert(lights.getDirectionalLight().enabled == true);

    lights.togglePointLight();
    assert(lights.getPointLight().enabled == false);
    lights.togglePointLight();
    assert(lights.getPointLight().enabled == true);

    lights.toggleSpotLight();
    assert(lights.getSpotLight().enabled == false);
    lights.toggleSpotLight();
    assert(lights.getSpotLight().enabled == true);
    std::cout << "  [OK] 6. Toggles individuales de fuentes luminosas funcionales." << std::endl;

    std::cout << "\n[SUCCESS] Issue #25 completado exitosamente: 3 fuentes de luz y modelos estructurados." << std::endl;
    return 0;
}
