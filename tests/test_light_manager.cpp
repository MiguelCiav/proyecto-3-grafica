#include "lighting/LightManager.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: LightManager y Estructuras de Iluminacion         " << std::endl;
    std::cout << "  Fase 7 - Issue #25 (Criterios Oficiales de Aceptacion)  " << std::endl;
    std::cout << "==========================================================" << std::endl;

    lighting::LightManager lightManager;
    lightManager.init();

    // -------------------------------------------------------------
    // Criterio 1: Parámetros fotométricos completos en todos los structs
    // -------------------------------------------------------------
    // Luz Direccional (Sol)
    const auto& dir = lightManager.getDirectionalLight();
    assert(dir.enabled == true);
    assert(glm::length(dir.direction) > 0.99f && glm::length(dir.direction) < 1.01f);
    assert(dir.diffuse.r > 0.0f && dir.ambient.r > 0.0f && dir.specular.r > 0.0f);
    std::cout << "  [CHECK 1.1] DirectionalLight: Direccion, ambient, diffuse, specular fotometricamente correctos." << std::endl;

    // Luz Focal / Spotlight (Faros del kart)
    const auto& spot = lightManager.getSpotLight();
    assert(spot.enabled == true);
    assert(spot.cutOff > spot.outerCutOff); // corte interior > corte exterior en cosenos
    assert(spot.constant == 1.0f);
    assert(spot.linear > 0.0f && spot.quadratic > 0.0f);
    std::cout << "  [CHECK 1.2] SpotLight: Conos interior/exterior con penumbra suave y atenuacion validos." << std::endl;

    // Material
    const auto& mat = lightManager.getDefaultMaterial();
    assert(mat.shininess >= 1.0f);
    std::cout << "  [CHECK 1.3] Material: Propiedades de brillo especular inicializadas." << std::endl;

    // -------------------------------------------------------------
    // Criterio 2: Soporte para registrar MULTIPLES fuentes puntuales con atenuación calculada
    // -------------------------------------------------------------
    assert(lightManager.getPointLightCount() >= 4); // Al menos 4 postes iniciales
    std::cout << "  [CHECK 2.1] LightManager contiene " << lightManager.getPointLightCount()
              << " fuentes puntuales inicializadas." << std::endl;

    // Registrar una fuente puntual adicional dinámica con atenuación calibrada para 65m
    lighting::PointLight customPoint;
    customPoint.position = glm::vec3(12.0f, 5.0f, -8.0f);
    customPoint.diffuse = glm::vec3(0.3f, 0.7f, 1.0f); // Luz cian
    customPoint.constant = 1.0f;
    customPoint.linear = 0.045f;
    customPoint.quadratic = 0.0075f;
    customPoint.enabled = true;

    size_t prevCount = lightManager.getPointLightCount();
    lightManager.addPointLight(customPoint);
    assert(lightManager.getPointLightCount() == prevCount + 1);

    const auto& registeredLight = lightManager.getPointLight(prevCount);
    assert(registeredLight.position == customPoint.position);
    assert(registeredLight.constant == 1.0f);
    assert(registeredLight.linear == 0.045f);
    assert(registeredLight.quadratic == 0.0075f);
    std::cout << "  [CHECK 2.2] Registro dinamico de multiples fuentes puntuales con atenuacion verificado." << std::endl;

    // -------------------------------------------------------------
    // Criterio 3: Conmutación de modelos y cinemática de faros
    // -------------------------------------------------------------
    assert(lightManager.getLightingModel() == lighting::LightingModel::BLINN_PHONG);
    lightManager.cycleNextModel();
    assert(lightManager.getLightingModel() == lighting::LightingModel::FLAT);
    lightManager.cycleNextModel();
    assert(lightManager.getLightingModel() == lighting::LightingModel::LAMBERT);
    lightManager.cycleNextModel();
    assert(lightManager.getLightingModel() == lighting::LightingModel::BLINN_PHONG);
    std::cout << "  [CHECK 3.1] Conmutacion circular de modelos de sombreado (Flat, Lambert, Blinn-Phong)." << std::endl;

    glm::vec3 kartPos(5.0f, 0.0f, 20.0f);
    glm::vec3 kartForward(0.0f, 0.0f, 1.0f);
    lightManager.updateKartHeadlights(kartPos, kartForward);
    assert(lightManager.getSpotLight().position.z > kartPos.z);
    assert(lightManager.getSpotLight().position.y > 0.4f);
    std::cout << "  [CHECK 3.2] Seguimiento dinamico de los faros del kart verificado." << std::endl;

    // Toggle de todas las luces puntuales
    lightManager.togglePointLights();
    assert(lightManager.getPointLight(0).enabled == false);
    lightManager.togglePointLights();
    assert(lightManager.getPointLight(0).enabled == true);
    std::cout << "  [CHECK 3.3] Toggle global de fuentes puntuales verificado." << std::endl;

    std::cout << "\n==========================================================" << std::endl;
    std::cout << "  [SUCCESS] Todos los criterios del Issue #25 cumplidos!   " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
