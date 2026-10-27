#include "world/TrackEnvironmentConfigurator.hpp"
#include <iostream>
#include <glm/geometric.hpp>

namespace world {

void TrackEnvironmentConfigurator::setupColliders(
    const TrackGrid& trackGrid,
    const PropSpawner& propSpawner,
    physics::CollisionSystem& collisionSystem
) {
    collisionSystem.clear();

    for (const auto& prop : propSpawner.getProps()) {
        if (!prop.hasCollision) continue;
        collisionSystem.addStaticBox(physics::BoundingBox(prop.aabbMin, prop.aabbMax));
    }

    float halfWidth = (trackGrid.getCols() * TrackGrid::TILE_SIZE) * 0.5f + 4.0f;
    float halfDepth = (trackGrid.getRows() * TrackGrid::TILE_SIZE) * 0.5f + 4.0f;
    float wallHeight = 6.0f;

    collisionSystem.addStaticBox(physics::BoundingBox(
        glm::vec3(-halfWidth, 0.0f, halfDepth),
        glm::vec3(halfWidth, wallHeight, halfDepth + 2.0f)
    ));
    collisionSystem.addStaticBox(physics::BoundingBox(
        glm::vec3(-halfWidth, 0.0f, -halfDepth - 2.0f),
        glm::vec3(halfWidth, wallHeight, -halfDepth)
    ));
    collisionSystem.addStaticBox(physics::BoundingBox(
        glm::vec3(halfWidth, 0.0f, -halfDepth),
        glm::vec3(halfWidth + 2.0f, wallHeight, halfDepth)
    ));
    collisionSystem.addStaticBox(physics::BoundingBox(
        glm::vec3(-halfWidth - 2.0f, 0.0f, -halfDepth),
        glm::vec3(-halfWidth, wallHeight, halfDepth)
    ));

    std::cout << "[OK] [CollisionSystem] Registrados " << collisionSystem.getStaticBoxes().size()
              << " colisionadores estaticos adaptados al circuito (" << trackGrid.getCols()
              << "x" << trackGrid.getRows() << ")." << std::endl;
}

void TrackEnvironmentConfigurator::setupTrackLighting(
    const TrackProfile* activeTrack,
    const PropSpawner& propSpawner,
    lighting::LightManager& lightManager,
    lighting::LightingController& lightingController,
    renderer::TrackRenderer* trackRenderer
) {
    lightManager.clearPointLights();

    const bool isNight   = activeTrack ? activeTrack->isNight : false;
    const bool isDesert  = activeTrack ? (activeTrack->theme == TrackTheme::DESERT_DUNES) : false;
    const bool isSpace   = activeTrack ? (activeTrack->theme == TrackTheme::COSMIC_SPACE) : false;
    const bool isAquatic = activeTrack ? (activeTrack->theme == TrackTheme::AQUATIC_REEF) : false;
    const bool isVolcano = activeTrack ? (activeTrack->theme == TrackTheme::VOLCANO_CALDERA) : false;

    if (trackRenderer && activeTrack) {
        trackRenderer->setGroundColor(TrackEnvironmentRegistry::getGroundColorForTrack(*activeTrack));
    }

    if (isSpace) {
        for (const auto& star : propSpawner.getStarLights()) {
            if (lightManager.getPointLightCount() >= lighting::LightManager::MAX_POINT_LIGHTS) break;
            lighting::PointLight pl;
            pl.position  = star.position;
            pl.ambient   = star.diffuseColor * 0.25f;
            pl.diffuse   = star.diffuseColor * 4.2f;
            pl.specular  = star.diffuseColor * 3.2f;
            pl.constant  = 1.0f;
            pl.linear    = 0.07f;
            pl.quadratic = 0.020f;
            pl.enabled   = true;
            lightManager.addPointLight(pl);
        }
    } else if (isAquatic) {
        for (const auto& pos : propSpawner.getLampPositions()) {
            if (lightManager.getPointLightCount() >= lighting::LightManager::MAX_POINT_LIGHTS) break;
            lighting::PointLight pearl;
            pearl.position  = pos;
            pearl.ambient   = glm::vec3(0.06f, 0.16f, 0.18f);
            pearl.diffuse   = glm::vec3(1.40f, 2.60f, 2.40f);
            pearl.specular  = glm::vec3(1.20f, 2.20f, 2.00f);
            pearl.constant  = 1.0f;
            pearl.linear    = 0.06f;
            pearl.quadratic = 0.016f;
            pearl.enabled   = true;
            lightManager.addPointLight(pearl);
        }
    } else if (isVolcano) {
        for (const auto& pos : propSpawner.getLampPositions()) {
            if (lightManager.getPointLightCount() >= lighting::LightManager::MAX_POINT_LIGHTS) break;
            lighting::PointLight fire;
            fire.position  = pos;
            fire.ambient   = glm::vec3(0.18f, 0.06f, 0.01f);
            fire.diffuse   = glm::vec3(2.80f, 1.20f, 0.25f);
            fire.specular  = glm::vec3(2.20f, 0.90f, 0.15f);
            fire.constant  = 1.0f;
            fire.linear    = 0.055f;
            fire.quadratic = 0.014f;
            fire.enabled   = true;
            lightManager.addPointLight(fire);
        }
    } else if (!isDesert) {
        for (const auto& pos : propSpawner.getLampPositions()) {
            lighting::PointLight lamp;
            lamp.position  = pos;
            lamp.ambient   = glm::vec3(0.12f, 0.09f, 0.04f);
            lamp.diffuse   = glm::vec3(2.60f, 2.20f, 1.20f);
            lamp.specular  = glm::vec3(1.80f, 1.50f, 0.80f);
            lamp.constant  = 1.0f;
            lamp.linear    = 0.055f;
            lamp.quadratic = 0.014f;
            lamp.enabled   = isNight;
            lightManager.addPointLight(lamp);
        }
    }

    lightingController.setDayNight(isNight);
}

void TrackEnvironmentConfigurator::applyDayNightState(
    bool isNight,
    const TrackProfile* activeTrack,
    const PropSpawner& propSpawner,
    lighting::LightManager& lightManager,
    renderer::TrackRenderer* trackRenderer,
    lighting::LightingController* lightingController
) {
    const TrackTheme theme = activeTrack ? activeTrack->theme : TrackTheme::DAY_GRASSLAND;

    auto& sun = lightManager.getDirectionalLight();
    auto& spot = lightManager.getSpotLight();
    auto& points = lightManager.getPointLights();

    auto setHeadlightsState = [&](bool enabled) {
        if (lightingController) {
            lightingController->setHeadlights(enabled);
        } else {
            spot.enabled = enabled;
        }
    };

    switch (theme) {
        case TrackTheme::VOLCANO_CALDERA: {
            if (isNight) {
                sun.direction = glm::normalize(glm::vec3(-0.35f, -0.65f, -0.45f));
                sun.ambient   = glm::vec3(0.10f, 0.04f, 0.02f);
                sun.diffuse   = glm::vec3(0.35f, 0.12f, 0.05f);
                sun.specular  = glm::vec3(0.40f, 0.15f, 0.05f);

                for (auto& p : points) {
                    p.ambient   = glm::vec3(0.18f, 0.06f, 0.01f);
                    p.diffuse   = glm::vec3(3.20f, 1.40f, 0.28f);
                    p.specular  = glm::vec3(2.40f, 1.00f, 0.18f);
                    p.constant  = 1.0f;
                    p.linear    = 0.055f;
                    p.quadratic = 0.014f;
                    p.enabled   = true;
                }

                spot.diffuse     = glm::vec3(2.40f, 2.00f, 1.60f);
                spot.specular    = glm::vec3(1.80f, 1.40f, 1.00f);
                spot.cutOff      = 0.9135f;
                spot.outerCutOff = 0.8480f;
                setHeadlightsState(true);

                if (trackRenderer) {
                    trackRenderer->setGroundColor(glm::vec4(0.06f, 0.05f, 0.06f, 1.0f));
                }
            } else {
                sun.direction = glm::normalize(glm::vec3(0.20f, -0.80f, 0.30f));
                sun.ambient   = glm::vec3(0.32f, 0.20f, 0.15f);
                sun.diffuse   = glm::vec3(1.10f, 0.65f, 0.35f);
                sun.specular  = glm::vec3(0.80f, 0.45f, 0.20f);

                for (auto& p : points) {
                    p.ambient   = glm::vec3(0.12f, 0.05f, 0.01f);
                    p.diffuse   = glm::vec3(2.40f, 1.05f, 0.20f);
                    p.specular  = glm::vec3(1.80f, 0.75f, 0.12f);
                    p.constant  = 1.0f;
                    p.linear    = 0.055f;
                    p.quadratic = 0.014f;
                    p.enabled   = true;
                }

                spot.diffuse     = glm::vec3(0.80f, 0.60f, 0.40f);
                spot.specular    = glm::vec3(0.50f, 0.35f, 0.20f);
                spot.cutOff      = 0.9135f;
                spot.outerCutOff = 0.8480f;
                setHeadlightsState(false);

                if (trackRenderer) {
                    trackRenderer->setGroundColor(glm::vec4(0.09f, 0.08f, 0.09f, 1.0f));
                }
            }
            sun.enabled = true;
            break;
        }

        case TrackTheme::AQUATIC_REEF: {
            if (isNight) {
                sun.direction = glm::normalize(glm::vec3(-0.20f, -0.75f, -0.40f));
                sun.ambient   = glm::vec3(0.03f, 0.08f, 0.12f);
                sun.diffuse   = glm::vec3(0.08f, 0.22f, 0.32f);
                sun.specular  = glm::vec3(0.10f, 0.25f, 0.30f);

                for (auto& p : points) {
                    p.ambient   = glm::vec3(0.06f, 0.16f, 0.18f);
                    p.diffuse   = glm::vec3(1.60f, 2.80f, 2.60f);
                    p.specular  = glm::vec3(1.40f, 2.40f, 2.20f);
                    p.constant  = 1.0f;
                    p.linear    = 0.06f;
                    p.quadratic = 0.016f;
                    p.enabled   = true;
                }

                spot.diffuse     = glm::vec3(2.20f, 2.40f, 2.60f);
                spot.specular    = glm::vec3(1.60f, 1.80f, 2.00f);
                spot.cutOff      = 0.9135f;
                spot.outerCutOff = 0.8480f;
                setHeadlightsState(true);

                if (trackRenderer) {
                    trackRenderer->setGroundColor(glm::vec4(0.02f, 0.09f, 0.14f, 1.0f));
                }
            } else {
                sun.direction = glm::normalize(glm::vec3(0.15f, -0.85f, 0.25f));
                sun.ambient   = glm::vec3(0.12f, 0.28f, 0.32f);
                sun.diffuse   = glm::vec3(0.35f, 0.85f, 0.95f);
                sun.specular  = glm::vec3(0.50f, 0.80f, 0.90f);

                for (auto& p : points) {
                    p.ambient   = glm::vec3(0.04f, 0.12f, 0.14f);
                    p.diffuse   = glm::vec3(1.20f, 2.20f, 2.00f);
                    p.specular  = glm::vec3(1.00f, 1.80f, 1.60f);
                    p.constant  = 1.0f;
                    p.linear    = 0.06f;
                    p.quadratic = 0.016f;
                    p.enabled   = true;
                }

                spot.diffuse     = glm::vec3(0.60f, 0.80f, 0.85f);
                spot.specular    = glm::vec3(0.40f, 0.60f, 0.65f);
                spot.cutOff      = 0.9135f;
                spot.outerCutOff = 0.8480f;
                setHeadlightsState(false);

                if (trackRenderer) {
                    trackRenderer->setGroundColor(glm::vec4(0.04f, 0.16f, 0.22f, 1.0f));
                }
            }
            sun.enabled = true;
            break;
        }

        case TrackTheme::COSMIC_SPACE: {
            if (isNight) {
                sun.direction = glm::normalize(glm::vec3(-0.45f, -0.60f, -0.65f));
                sun.ambient   = glm::vec3(0.05f, 0.03f, 0.10f);
                sun.diffuse   = glm::vec3(0.28f, 0.22f, 0.45f);
                sun.specular  = glm::vec3(0.35f, 0.30f, 0.50f);

                const auto& stars = propSpawner.getStarLights();
                for (size_t i = 0; i < points.size() && i < stars.size(); ++i) {
                    points[i].ambient   = stars[i].diffuseColor * 0.28f;
                    points[i].diffuse   = stars[i].diffuseColor * 4.8f;
                    points[i].specular  = stars[i].diffuseColor * 3.6f;
                    points[i].constant  = 1.0f;
                    points[i].linear    = 0.07f;
                    points[i].quadratic = 0.020f;
                    points[i].enabled   = true;
                }

                spot.diffuse     = glm::vec3(2.40f, 2.40f, 2.80f);
                spot.specular    = glm::vec3(1.80f, 1.80f, 2.00f);
                spot.cutOff      = 0.9135f;
                spot.outerCutOff = 0.8480f;
                setHeadlightsState(true);
            } else {
                sun.direction = glm::normalize(glm::vec3(-0.45f, -0.60f, -0.65f));
                sun.ambient   = glm::vec3(0.16f, 0.12f, 0.26f);
                sun.diffuse   = glm::vec3(1.05f, 0.98f, 1.25f);
                sun.specular  = glm::vec3(1.20f, 1.10f, 1.40f);

                const auto& stars = propSpawner.getStarLights();
                for (size_t i = 0; i < points.size() && i < stars.size(); ++i) {
                    points[i].ambient   = stars[i].diffuseColor * 0.22f;
                    points[i].diffuse   = stars[i].diffuseColor * 3.8f;
                    points[i].specular  = stars[i].diffuseColor * 2.8f;
                    points[i].constant  = 1.0f;
                    points[i].linear    = 0.07f;
                    points[i].quadratic = 0.020f;
                    points[i].enabled   = true;
                }

                spot.diffuse     = glm::vec3(0.60f, 0.60f, 0.70f);
                spot.specular    = glm::vec3(0.40f);
                spot.cutOff      = 0.9135f;
                spot.outerCutOff = 0.8480f;
                setHeadlightsState(false);
            }
            sun.enabled = true;
            if (trackRenderer) {
                trackRenderer->setGroundColor(glm::vec4(0.012f, 0.010f, 0.024f, 1.0f));
            }
            break;
        }

        case TrackTheme::DESERT_DUNES: {
            if (isNight) {
                sun.direction = glm::normalize(glm::vec3(0.40f, -0.72f, 0.56f));
                sun.ambient   = glm::vec3(0.07f, 0.08f, 0.12f);
                sun.diffuse   = glm::vec3(0.14f, 0.16f, 0.22f);
                sun.specular  = glm::vec3(0.15f, 0.15f, 0.20f);

                for (auto& p : points) {
                    p.enabled = false;
                }

                spot.diffuse     = glm::vec3(1.80f, 1.70f, 1.45f);
                spot.specular    = glm::vec3(1.20f);
                spot.cutOff      = 0.9135f;
                spot.outerCutOff = 0.8480f;
                setHeadlightsState(true);

                if (trackRenderer) {
                    trackRenderer->setGroundColor(glm::vec4(0.20f, 0.19f, 0.23f, 1.0f));
                }
            } else {
                sun.direction = glm::normalize(glm::vec3(0.40f, -0.72f, 0.56f));
                sun.ambient   = glm::vec3(0.45f, 0.40f, 0.32f);
                sun.diffuse   = glm::vec3(1.15f, 1.06f, 0.90f);
                sun.specular  = glm::vec3(0.70f, 0.65f, 0.52f);

                for (auto& p : points) {
                    p.enabled = false;
                }

                spot.diffuse     = glm::vec3(0.40f, 0.40f, 0.35f);
                spot.specular    = glm::vec3(0.25f);
                spot.cutOff      = 0.9135f;
                spot.outerCutOff = 0.8480f;
                setHeadlightsState(false);

                if (trackRenderer) {
                    trackRenderer->setGroundColor(glm::vec4(0.89f, 0.74f, 0.48f, 1.0f));
                }
            }
            sun.enabled = true;
            break;
        }

        default: {
            if (isNight) {
                sun.direction = glm::normalize(glm::vec3(-0.25f, -0.65f, -0.50f));
                sun.ambient   = glm::vec3(0.06f, 0.07f, 0.10f);
                sun.diffuse   = glm::vec3(0.08f, 0.08f, 0.12f);
                sun.specular  = glm::vec3(0.06f);

                for (auto& p : points) {
                    p.ambient   = glm::vec3(0.12f, 0.09f, 0.04f);
                    p.diffuse   = glm::vec3(2.60f, 2.20f, 1.20f);
                    p.specular  = glm::vec3(1.80f, 1.50f, 0.80f);
                    p.constant  = 1.0f;
                    p.linear    = 0.055f;
                    p.quadratic = 0.014f;
                    p.enabled   = true;
                }

                spot.diffuse     = glm::vec3(1.80f, 1.70f, 1.45f);
                spot.specular    = glm::vec3(1.20f);
                spot.cutOff      = 0.9135f;
                spot.outerCutOff = 0.8480f;
                setHeadlightsState(true);

                if (trackRenderer) {
                    trackRenderer->setGroundColor(glm::vec4(0.08f, 0.12f, 0.10f, 1.0f));
                }
            } else {
                sun.direction = glm::normalize(glm::vec3(-0.35f, -0.85f, -0.40f));
                sun.ambient   = glm::vec3(0.28f, 0.28f, 0.30f);
                sun.diffuse   = glm::vec3(0.82f, 0.80f, 0.76f);
                sun.specular  = glm::vec3(0.40f);

                for (auto& p : points) {
                    p.ambient  = glm::vec3(0.0f);
                    p.diffuse  = glm::vec3(0.0f);
                    p.specular = glm::vec3(0.0f);
                    p.enabled  = false;
                }

                spot.diffuse     = glm::vec3(0.40f, 0.40f, 0.35f);
                spot.specular    = glm::vec3(0.25f);
                spot.cutOff      = 0.9135f;
                spot.outerCutOff = 0.8480f;
                setHeadlightsState(false);

                if (trackRenderer) {
                    trackRenderer->setGroundColor(glm::vec4(0.28f, 0.62f, 0.28f, 1.0f));
                }
            }
            sun.enabled = true;
            break;
        }
    }
}

void TrackEnvironmentConfigurator::setupBoosterPads(
    const TrackGrid& trackGrid,
    ZoneTriggerManager& zoneManager,
    std::vector<BoosterPad>& boosterPads,
    float initialYaw
) {
    boosterPads.clear();

    auto placements = trackGrid.computeOptimalBoosterPlacements(initialYaw, 8);
    for (const auto& item : placements) {
        BoosterPad pad(item.worldPos, item.trafficYaw, item.width, item.length);
        pad.setBoostMultiplier(1.6f);
        pad.setBoostDuration(2.0f);
        pad.setInstantImpulse(8.0f);
        pad.registerTrigger(zoneManager);
        boosterPads.push_back(std::move(pad));
    }

    std::cout << "[OK] [Boosters] Desplegados " << boosterPads.size()
              << " pads de aceleracion alineados con el sentido de carrera." << std::endl;
}

void TrackEnvironmentConfigurator::setupBoosterPads(
    const TrackGrid& trackGrid,
    const ZoneTriggerManager& zoneManager,
    std::vector<BoosterPad>& boosterPads
) {
    float initialYaw = 90.0f;
    for (const auto& tile : trackGrid.getAllTiles()) {
        if (tile.type == TileType::START_FINISH) {
            initialYaw = tile.rotationDegrees;
            break;
        }
    }
    setupBoosterPads(trackGrid, const_cast<ZoneTriggerManager&>(zoneManager), boosterPads, initialYaw);
}

} // namespace world
