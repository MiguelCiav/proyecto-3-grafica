#include "lighting/LightingController.hpp"
#include "core/Input.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/geometric.hpp>

namespace lighting {

LightingController::LightingController()
    : m_lightManager(nullptr),
      m_frontOffset(0.75f),
      m_headlightsOn(true) {
}

LightingController::LightingController(LightManager* lightManager)
    : m_lightManager(lightManager),
      m_frontOffset(0.75f),
      m_headlightsOn(true) {
    if (m_lightManager) {
        m_lightManager->getSpotLight().enabled = m_headlightsOn;
    }
}

void LightingController::init(LightManager* lightManager) {
    m_lightManager = lightManager;
    if (m_lightManager) {
        m_headlightsOn = m_lightManager->getSpotLight().enabled;
    }
}

void LightingController::update(const glm::vec3& kartPosition, const glm::vec3& kartForward, float /*deltaTime*/) {
    if (!m_lightManager) return;

    glm::vec3 forward = kartForward;
    if (glm::length(forward) < 0.0001f) {
        forward = glm::vec3(0.0f, 0.0f, 1.0f);
    } else {
        forward = glm::normalize(forward);
    }

    // Posicionamiento de faros frontales:
    // spotLight.position = kart.getPosition() + kart.getForward() * frontOffset + glm::vec3(0.0f, 0.4f, 0.0f)
    // spotLight.direction = kart.getForward()
    SpotLight& spot = m_lightManager->getSpotLight();
    spot.position = kartPosition + forward * m_frontOffset + glm::vec3(0.0f, 0.4f, 0.0f);
    spot.direction = forward;
    spot.enabled = m_headlightsOn;
}

void LightingController::handleInput() {
    if (!m_lightManager) return;

    // -------------------------------------------------------------
    // Conmutador del Modelo de Iluminación (Teclas 1, 2, 3)
    // -------------------------------------------------------------
    if (core::Input::isKeyJustPressed(GLFW_KEY_1) || core::Input::isKeyJustPressed(GLFW_KEY_KP_1)) {
        setLightingModel(LightingModel::FLAT);
    } else if (core::Input::isKeyJustPressed(GLFW_KEY_2) || core::Input::isKeyJustPressed(GLFW_KEY_KP_2)) {
        setLightingModel(LightingModel::LAMBERT);
    } else if (core::Input::isKeyJustPressed(GLFW_KEY_3) || core::Input::isKeyJustPressed(GLFW_KEY_KP_3)) {
        setLightingModel(LightingModel::BLINN_PHONG);
    }

    // -------------------------------------------------------------
    // -------------------------------------------------------------
    // Alternancia de Faros del Kart (Tecla L)
    // -------------------------------------------------------------
    if (core::Input::isKeyJustPressed(GLFW_KEY_L)) {
        toggleHeadlights();
    }

    // -------------------------------------------------------------
    // Alternancia de Farolas de Pista (Tecla P)
    // -------------------------------------------------------------
    if (core::Input::isKeyJustPressed(GLFW_KEY_P)) {
        togglePointLights();
    }

    // -------------------------------------------------------------
    // Conmutador de Ciclo Dia / Noche (Tecla N)
    // -------------------------------------------------------------
    if (core::Input::isKeyJustPressed(GLFW_KEY_N)) {
        toggleDayNight();
    }
}

void LightingController::toggleHeadlights() {
    setHeadlights(!m_headlightsOn);
}

void LightingController::setHeadlights(bool enabled) {
    m_headlightsOn = enabled;
    if (m_lightManager) {
        m_lightManager->getSpotLight().enabled = m_headlightsOn;
    }
    std::cout << "[Faros Kart] Estado: " << (m_headlightsOn ? "[ENCENDIDOS - SPOT ACTIVO]" : "[APAGADOS]") << std::endl;
}

void LightingController::togglePointLights() {
    if (!m_lightManager) return;
    m_lightManager->togglePointLights();
    bool on = !m_lightManager->getPointLights().empty() && m_lightManager->getPointLights()[0].enabled;
    std::cout << "\n>>> [FAROLAS DE PISTA]: " << (on ? "[ENCENDIDAS - 8 FUENTES DE LUZ ACTIVAS]" : "[APAGADAS]") << " <<<\n" << std::endl;
}

void LightingController::toggleDayNight() {
    if (!m_lightManager) return;
    auto& sun = m_lightManager->getDirectionalLight();
    if (sun.diffuse.x > 0.35f) {
        sun.ambient = glm::vec3(0.06f, 0.07f, 0.10f);
        sun.diffuse = glm::vec3(0.10f, 0.10f, 0.14f);
        sun.specular = glm::vec3(0.08f);
        std::cout << "\n>>> [AMBIENTE]: Modo [NOCHE / ATARDECER ACTIVADO (Tecla N)] - Farolas y Faros al maximo contraste! <<<\n" << std::endl;
    } else {
        sun.ambient = glm::vec3(0.20f, 0.20f, 0.23f);
        sun.diffuse = glm::vec3(0.55f, 0.52f, 0.48f);
        sun.specular = glm::vec3(0.35f);
        std::cout << "\n>>> [AMBIENTE]: Modo [DIA ILUMINADO ACTIVADO (Tecla N)] <<<\n" << std::endl;
    }
}

bool LightingController::areHeadlightsOn() const {
    return m_headlightsOn;
}

void LightingController::setLightingModel(LightingModel model) {
    if (!m_lightManager) return;
    m_lightManager->setLightingModel(model);
}

LightingModel LightingController::getLightingModel() const {
    if (!m_lightManager) return LightingModel::BLINN_PHONG;
    return m_lightManager->getLightingModel();
}

const char* LightingController::getLightingModelName() const {
    if (!m_lightManager) return "Desconocido";
    return m_lightManager->getCurrentModelName();
}

} // namespace lighting
