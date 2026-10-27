#pragma once

#include <vector>
#include "world/TrackGrid.hpp"
#include "world/PropSpawner.hpp"
#include "world/ZoneTrigger.hpp"
#include "world/BoosterPad.hpp"
#include "world/TrackEnvironmentRegistry.hpp"
#include "physics/CollisionSystem.hpp"
#include "lighting/LightManager.hpp"
#include "lighting/LightingController.hpp"
#include "renderer/TrackRenderer.hpp"

namespace world {

/**
 * @brief Configurador cohesivo del entorno de pista: colisionadores estaticos,
 *        iluminacion espectral por bioma, ciclo dia/noche y despliegue de boosters.
 */
class TrackEnvironmentConfigurator {
public:
    static void setupColliders(
        const TrackGrid& trackGrid,
        const PropSpawner& propSpawner,
        physics::CollisionSystem& collisionSystem
    );

    static void setupTrackLighting(
        const TrackProfile* activeTrack,
        const PropSpawner& propSpawner,
        lighting::LightManager& lightManager,
        lighting::LightingController& lightingController,
        renderer::TrackRenderer* trackRenderer = nullptr
    );

    static void applyDayNightState(
        bool isNight,
        const TrackProfile* activeTrack,
        const PropSpawner& propSpawner,
        lighting::LightManager& lightManager,
        renderer::TrackRenderer* trackRenderer,
        lighting::LightingController* lightingController = nullptr
    );

    static void setupBoosterPads(
        const TrackGrid& trackGrid,
        const ZoneTriggerManager& zoneManager,
        std::vector<BoosterPad>& boosterPads
    );

    static void setupBoosterPads(
        const TrackGrid& trackGrid,
        ZoneTriggerManager& zoneManager,
        std::vector<BoosterPad>& boosterPads,
        float initialYaw
    );
};

} // namespace world
