#pragma once

#include "lighting/LightManager.hpp"
#include <glm/glm.hpp>

namespace lighting {

/**
 * @brief Controlador interactivo para la iluminacion de la escena y faros del kart.
 * Responsabilidades:
 * 1. Sincroniza en cada fotograma el Spotlight movil con el morro delantero del kart.
 * 2. Captura las teclas 1, 2 y 3 para conmutar instantaneamente entre Flat, Lambert y Blinn-Phong.
 * 3. Captura la tecla L para alternar el encendido/apagado de los faros del kart.
 */
class LightingController {
public:
    LightingController();
    explicit LightingController(LightManager* lightManager);
    ~LightingController() = default;

    // Vincula el gestor de luces al controlador
    void init(LightManager* lightManager);

    // Sincroniza la posición y dirección del Spotlight con el kart
    // Fórmula: spot.position = kartPos + kartForward * frontOffset + (0.0, 0.4, 0.0)
    //          spot.direction = kartForward
    void update(const glm::vec3& kartPosition, const glm::vec3& kartForward, float deltaTime = 0.0f);

    // Procesa las pulsaciones de teclado (Teclas 1, 2, 3 y Tecla L)
    void handleInput();

    // Control de faros delanteros (Spotlight)
    void toggleHeadlights();
    void setHeadlights(bool enabled);
    bool areHeadlightsOn() const;

    // Control de farolas de pista y ciclo dia/noche
    void togglePointLights();
    void toggleDayNight();

    // Control de modelo de sombreado
    void setLightingModel(LightingModel model);
    LightingModel getLightingModel() const;
    const char* getLightingModelName() const;

    float getFrontOffset() const { return m_frontOffset; }
    void setFrontOffset(float offset) { m_frontOffset = offset; }

    LightManager* getLightManager() { return m_lightManager; }
    const LightManager* getLightManager() const { return m_lightManager; }

private:
    LightManager* m_lightManager{nullptr};
    float m_frontOffset{0.75f};
    bool m_headlightsOn{true};
};

} // namespace lighting
