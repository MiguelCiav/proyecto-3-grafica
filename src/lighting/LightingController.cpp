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
    auto& points = m_lightManager->getPointLights();
    if (points.empty()) return;

    bool wasOn = points[0].enabled && (glm::length(points[0].diffuse) > 0.05f);
    bool turnOn = !wasOn;

    for (auto& p : points) {
        p.enabled = turnOn;
        if (turnOn) {
            p.ambient  = glm::vec3(0.06f, 0.05f, 0.02f);
            p.diffuse  = m_isNight ? glm::vec3(2.2f, 1.8f, 0.75f) : glm::vec3(0.80f, 0.68f, 0.30f);
            p.specular = glm::vec3(1.0f, 0.8f, 0.4f);
        } else {
            p.ambient  = glm::vec3(0.0f);
            p.diffuse  = glm::vec3(0.0f);
            p.specular = glm::vec3(0.0f);
        }
    }
    std::cout << "\n>>> [FAROLAS DE PISTA]: " << (turnOn ? "[ENCENDIDAS (Tecla P)]" : "[APAGADAS (Tecla P)]") << " <<<\n" << std::endl;
}

void LightingController::setDayNight(bool night) {
    m_isNight = night;
    if (!m_lightManager) return;

    auto& sun = m_lightManager->getDirectionalLight();
    auto& spot = m_lightManager->getSpotLight();
    auto& points = m_lightManager->getPointLights();

    if (m_isNight) {
        // Noche: Luna azulada suave, farolas encendidas a brillo cálido ámbar, faros encendidos
        sun.ambient  = glm::vec3(0.06f, 0.07f, 0.10f);
        sun.diffuse  = glm::vec3(0.08f, 0.08f, 0.12f);
        sun.specular = glm::vec3(0.06f);

        for (auto& p : points) {
            p.ambient  = glm::vec3(0.06f, 0.05f, 0.02f);
            p.diffuse  = glm::vec3(2.2f, 1.8f, 0.75f);
            p.specular = glm::vec3(1.5f, 1.2f, 0.5f);
            p.enabled  = true;
        }

        spot.diffuse  = glm::vec3(1.20f, 1.15f, 1.00f);
        spot.specular = glm::vec3(1.0f);
        setHeadlights(true);

        std::cout << "\n>>> [AMBIENTE]: Modo [NOCHE / ATARDECER ACTIVADO (Tecla N)] - Farolas y Faros al maximo contraste! <<<\n" << std::endl;
    } else {
        // Dia: Sol resplandeciente natural, farolas en reposo para no quemar el asfalto, faros tenues
        sun.ambient  = glm::vec3(0.28f, 0.28f, 0.30f);
        sun.diffuse  = glm::vec3(0.82f, 0.80f, 0.76f);
        sun.specular = glm::vec3(0.40f);

        for (auto& p : points) {
            p.ambient  = glm::vec3(0.0f);
            p.diffuse  = glm::vec3(0.0f);
            p.specular = glm::vec3(0.0f);
            p.enabled  = false;
        }

        spot.diffuse  = glm::vec3(0.30f, 0.30f, 0.25f);
        spot.specular = glm::vec3(0.20f);
        setHeadlights(false);

        std::cout << "\n>>> [AMBIENTE]: Modo [DIA CLARO ACTIVADO (Tecla N)] - Sol brillante, iluminacion suave y limpia <<<\n" << std::endl;
    }
}

void LightingController::toggleDayNight() {
    setDayNight(!m_isNight);
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
