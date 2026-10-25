#include "core/GameContext.hpp"
#include "core/SceneManager.hpp"
#include "world/LevelLoader.hpp"
#include <iostream>
#include <algorithm>

namespace core {

const renderer::Skybox& GameContext::getCurrentSkybox() const {
    if (!tracks.empty() && selectedTrackIndex >= 0 && static_cast<size_t>(selectedTrackIndex) < tracks.size()) {
        const auto& track = tracks[selectedTrackIndex];
        if (track.theme == world::TrackTheme::COSMIC_SPACE && skyboxCosmic && skyboxCosmic->isLoaded()) {
            return *skyboxCosmic;
        }
        if (track.theme == world::TrackTheme::AQUATIC_REEF && skyboxAquatic && skyboxAquatic->isLoaded()) {
            return *skyboxAquatic;
        }
        if (track.theme == world::TrackTheme::VOLCANO_CALDERA && skyboxVolcano && skyboxVolcano->isLoaded()) {
            return *skyboxVolcano;
        }
        if (track.theme == world::TrackTheme::DESERT_DUNES) {
            if (lightingController && lightingController->isNight() && skyboxNight && skyboxNight->isLoaded()) {
                return *skyboxNight;
            }
            if (skyboxDesert && skyboxDesert->isLoaded()) {
                return *skyboxDesert;
            }
        }
    }
    if (lightingController && lightingController->isNight() && skyboxNight && skyboxNight->isLoaded()) {
        return *skyboxNight;
    }
    return *skyboxDay;
}

void GameContext::applySelection() {
    if (vehicles.empty() || drivers.empty() || !kartEntity || !kartPhysics) return;

    auto& vehicle = vehicles[selectedVehicleIndex];
    auto& driver = drivers[selectedDriverIndex];

    geometry::KartAssembly combinedAssembly = vehicle.assembly;
    combinedAssembly.driver = driver.driverMesh;
    combinedAssembly.driver.pivotOffset = vehicle.driverOffset;

    kartEntity->setup(combinedAssembly);

    kartPhysics->m_maxSpeed = vehicle.maxSpeed;
    kartPhysics->m_acceleration = vehicle.acceleration;
    kartPhysics->m_turnSpeed = vehicle.turnSpeed;
    kartPhysics->m_drag = 5.0f;
    kartPhysics->m_brakingRate = 22.0f;
    kartPhysics->m_maxReverseSpeed = 4.5f;
}

void GameContext::selectDriver(int index) {
    if (drivers.empty()) return;
    int count = static_cast<int>(drivers.size());
    selectedDriverIndex = (index % count + count) % count;

    applySelection();
    printSelectionHUD();
}

void GameContext::selectVehicle(int index) {
    if (vehicles.empty()) return;
    int count = static_cast<int>(vehicles.size());
    selectedVehicleIndex = (index % count + count) % count;

    applySelection();
    printSelectionHUD();
}

void GameContext::selectTrack(int index) {
    if (tracks.empty() || !trackGrid || !propSpawner || !zoneManager || !trackRenderer) return;
    int count = static_cast<int>(tracks.size());
    selectedTrackIndex = (index % count + count) % count;
    const auto& track = tracks[selectedTrackIndex];

    world::LevelLoader::loadFromFile(track.filePath, *trackGrid);

    world::BiomeType biome = world::TrackEnvironmentRegistry::getBiomeForTrack(track);
    propSpawner->generateForGrid(*trackGrid, biome);
    zoneManager->autoGenerateFromGrid(*trackGrid);

    if (kartEmitter) {
        kartEmitter->setUnderwater(track.theme == world::TrackTheme::AQUATIC_REEF);
    }

    trackRenderer->setGroundColor(world::TrackEnvironmentRegistry::getGroundColorForTrack(track));

    setupColliders();
    setupTrackLighting();

    kartInitialPos = glm::vec3(0.0f, 0.0f, 0.0f);
    kartInitialYaw = 90.0f;

    for (const auto& tile : trackGrid->getAllTiles()) {
        if (tile.type == world::TileType::START_FINISH) {
            kartInitialPos = tile.worldPos;
            kartInitialYaw = tile.rotationDegrees;
            break;
        }
    }

    setupBoosterPads();

    if (kartPhysics) {
        kartPhysics->setPosition(kartInitialPos);
        kartPhysics->setYaw(kartInitialYaw);
        kartPhysics->resetSpeed();
    }
    if (wheelAnimator) {
        wheelAnimator->reset();
    }
    if (chaseCamera && kartPhysics) {
        chaseCamera->reset(kartInitialPos, kartPhysics->getForwardVector());
    }
    if (skidmarkManager) {
        skidmarkManager->clear();
    }

    printSelectionHUD();
}

void GameContext::setupColliders() {
    if (!collisionSystem || !propSpawner || !trackGrid) return;
    collisionSystem->clear();

    for (const auto& prop : propSpawner->getProps()) {
        if (!prop.hasCollision) continue;
        collisionSystem->addStaticBox(Engine::Physics::BoundingBox(prop.aabbMin, prop.aabbMax));
    }

    float halfWidth = (trackGrid->getCols() * world::TrackGrid::TILE_SIZE) * 0.5f + 4.0f;
    float halfDepth = (trackGrid->getRows() * world::TrackGrid::TILE_SIZE) * 0.5f + 4.0f;
    float wallHeight = 6.0f;

    collisionSystem->addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(-halfWidth, 0.0f, halfDepth),
        glm::vec3(halfWidth, wallHeight, halfDepth + 2.0f)
    ));
    collisionSystem->addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(-halfWidth, 0.0f, -halfDepth - 2.0f),
        glm::vec3(halfWidth, wallHeight, -halfDepth)
    ));
    collisionSystem->addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(halfWidth, 0.0f, -halfDepth),
        glm::vec3(halfWidth + 2.0f, wallHeight, halfDepth)
    ));
    collisionSystem->addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(-halfWidth - 2.0f, 0.0f, -halfDepth),
        glm::vec3(-halfWidth, wallHeight, halfDepth)
    ));

    std::cout << "[OK] [CollisionSystem] Registrados " << collisionSystem->getStaticBoxes().size()
              << " colisionadores estaticos adaptados al circuito (" << trackGrid->getCols()
              << "x" << trackGrid->getRows() << ")." << std::endl;
}

void GameContext::setupTrackLighting() {
    if (!lightManager || !lightingController || !propSpawner) return;
    lightManager->clearPointLights();

    bool isNight = (!tracks.empty() && selectedTrackIndex >= 0 && static_cast<size_t>(selectedTrackIndex) < tracks.size())
                    ? tracks[selectedTrackIndex].isNight : false;
    bool isDesert = (!tracks.empty() && selectedTrackIndex >= 0 && static_cast<size_t>(selectedTrackIndex) < tracks.size())
                    ? (tracks[selectedTrackIndex].theme == world::TrackTheme::DESERT_DUNES) : false;
    bool isSpace = (!tracks.empty() && selectedTrackIndex >= 0 && static_cast<size_t>(selectedTrackIndex) < tracks.size())
                    ? (tracks[selectedTrackIndex].theme == world::TrackTheme::COSMIC_SPACE) : false;
    bool isAquatic = (!tracks.empty() && selectedTrackIndex >= 0 && static_cast<size_t>(selectedTrackIndex) < tracks.size())
                    ? (tracks[selectedTrackIndex].theme == world::TrackTheme::AQUATIC_REEF) : false;
    bool isVolcano = (!tracks.empty() && selectedTrackIndex >= 0 && static_cast<size_t>(selectedTrackIndex) < tracks.size())
                    ? (tracks[selectedTrackIndex].theme == world::TrackTheme::VOLCANO_CALDERA) : false;

    if (isSpace) {
        for (const auto& star : propSpawner->getStarLights()) {
            if (lightManager->getPointLightCount() >= lighting::LightManager::MAX_POINT_LIGHTS) break;
            lighting::PointLight pl;
            pl.position  = star.position;
            pl.ambient   = star.diffuseColor * 0.25f;
            pl.diffuse   = star.diffuseColor * 4.2f;
            pl.specular  = star.diffuseColor * 3.2f;
            pl.constant  = 1.0f;
            pl.linear    = 0.07f;
            pl.quadratic = 0.020f;
            pl.enabled   = true;
            lightManager->addPointLight(pl);
        }
    } else if (isAquatic) {
        for (const auto& pos : propSpawner->getLampPositions()) {
            if (lightManager->getPointLightCount() >= lighting::LightManager::MAX_POINT_LIGHTS) break;
            lighting::PointLight pearl;
            pearl.position  = pos;
            pearl.ambient   = glm::vec3(0.06f, 0.16f, 0.18f);
            pearl.diffuse   = glm::vec3(1.40f, 2.60f, 2.40f);
            pearl.specular  = glm::vec3(1.20f, 2.20f, 2.00f);
            pearl.constant  = 1.0f;
            pearl.linear    = 0.06f;
            pearl.quadratic = 0.016f;
            pearl.enabled   = true;
            lightManager->addPointLight(pearl);
        }
    } else if (isVolcano) {
        for (const auto& pos : propSpawner->getLampPositions()) {
            if (lightManager->getPointLightCount() >= lighting::LightManager::MAX_POINT_LIGHTS) break;
            lighting::PointLight fire;
            fire.position  = pos;
            fire.ambient   = glm::vec3(0.18f, 0.06f, 0.01f);
            fire.diffuse   = glm::vec3(2.80f, 1.20f, 0.25f);
            fire.specular  = glm::vec3(2.20f, 0.90f, 0.15f);
            fire.constant  = 1.0f;
            fire.linear    = 0.055f;
            fire.quadratic = 0.014f;
            fire.enabled   = true;
            lightManager->addPointLight(fire);
        }
    } else if (!isDesert) {
        for (const auto& pos : propSpawner->getLampPositions()) {
            lighting::PointLight lamp;
            lamp.position  = pos;
            lamp.ambient   = glm::vec3(0.12f, 0.09f, 0.04f);
            lamp.diffuse   = glm::vec3(2.60f, 2.20f, 1.20f);
            lamp.specular  = glm::vec3(1.80f, 1.50f, 0.80f);
            lamp.constant  = 1.0f;
            lamp.linear    = 0.055f;
            lamp.quadratic = 0.014f;
            lamp.enabled   = isNight;
            lightManager->addPointLight(lamp);
        }
    }

    lightingController->setDayNight(isNight);
}

void GameContext::applyDayNightState(bool isNight) {
    if (!lightManager || !lightingController || !trackRenderer) return;

    world::TrackTheme theme = world::TrackTheme::DAY_GRASSLAND;
    std::string trackName = "Pista";
    if (!tracks.empty() && selectedTrackIndex >= 0 && static_cast<size_t>(selectedTrackIndex) < tracks.size()) {
        theme = tracks[selectedTrackIndex].theme;
        trackName = tracks[selectedTrackIndex].name;
    }

    auto& sun = lightManager->getDirectionalLight();
    auto& spot = lightManager->getSpotLight();
    auto& points = lightManager->getPointLights();

    switch (theme) {
        case world::TrackTheme::VOLCANO_CALDERA: {
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
                lightingController->setHeadlights(true);

                trackRenderer->setGroundColor(glm::vec4(0.06f, 0.05f, 0.06f, 1.0f));
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
                lightingController->setHeadlights(false);

                trackRenderer->setGroundColor(glm::vec4(0.09f, 0.08f, 0.09f, 1.0f));
            }
            sun.enabled = true;
            break;
        }

        case world::TrackTheme::AQUATIC_REEF: {
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
                lightingController->setHeadlights(true);

                trackRenderer->setGroundColor(glm::vec4(0.02f, 0.09f, 0.14f, 1.0f));
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
                lightingController->setHeadlights(false);

                trackRenderer->setGroundColor(glm::vec4(0.04f, 0.16f, 0.22f, 1.0f));
            }
            sun.enabled = true;
            break;
        }

        case world::TrackTheme::COSMIC_SPACE: {
            if (isNight) {
                sun.direction = glm::normalize(glm::vec3(-0.45f, -0.60f, -0.65f));
                sun.ambient   = glm::vec3(0.05f, 0.03f, 0.10f);
                sun.diffuse   = glm::vec3(0.28f, 0.22f, 0.45f);
                sun.specular  = glm::vec3(0.35f, 0.30f, 0.50f);

                const auto& stars = propSpawner->getStarLights();
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
                lightingController->setHeadlights(true);
            } else {
                sun.direction = glm::normalize(glm::vec3(-0.45f, -0.60f, -0.65f));
                sun.ambient   = glm::vec3(0.16f, 0.12f, 0.26f);
                sun.diffuse   = glm::vec3(1.05f, 0.98f, 1.25f);
                sun.specular  = glm::vec3(1.20f, 1.10f, 1.40f);

                const auto& stars = propSpawner->getStarLights();
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
                lightingController->setHeadlights(false);
            }
            sun.enabled = true;
            trackRenderer->setGroundColor(glm::vec4(0.012f, 0.010f, 0.024f, 1.0f));
            break;
        }

        case world::TrackTheme::DESERT_DUNES: {
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
                lightingController->setHeadlights(true);

                trackRenderer->setGroundColor(glm::vec4(0.20f, 0.19f, 0.23f, 1.0f));
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
                lightingController->setHeadlights(false);

                trackRenderer->setGroundColor(glm::vec4(0.89f, 0.74f, 0.48f, 1.0f));
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
                lightingController->setHeadlights(true);

                trackRenderer->setGroundColor(glm::vec4(0.08f, 0.12f, 0.10f, 1.0f));
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
                lightingController->setHeadlights(false);

                trackRenderer->setGroundColor(glm::vec4(0.28f, 0.62f, 0.28f, 1.0f));
            }
            sun.enabled = true;
            break;
        }
    }
}

void GameContext::setupBoosterPads() {
    if (!boosterPads || !trackGrid || !zoneManager) return;
    boosterPads->clear();

    auto placements = trackGrid->computeOptimalBoosterPlacements(kartInitialYaw, 8);
    for (const auto& item : placements) {
        world::BoosterPad pad(item.worldPos, item.trafficYaw, item.width, item.length);
        pad.setBoostMultiplier(1.6f);
        pad.setBoostDuration(2.0f);
        pad.setInstantImpulse(8.0f);
        pad.registerTrigger(*zoneManager);
        boosterPads->push_back(std::move(pad));
    }

    std::cout << "[OK] [Boosters] Desplegados " << boosterPads->size()
              << " pads de aceleracion alineados con el sentido de carrera." << std::endl;
}

void GameContext::printSelectionHUD() const {
    if (drivers.empty() || vehicles.empty() || tracks.empty()) return;

    const auto& driver = drivers[selectedDriverIndex];
    const auto& vehicle = vehicles[selectedVehicleIndex];
    const auto& track = tracks[selectedTrackIndex];

    std::cout << "\n========================================================" << std::endl;
    std::cout << "  SHOWROOM 3D - CONFIGURACION DE CARRERA MARIO KART     " << std::endl;
    std::cout << "  [VISTA FRONTAL FIJA ACTIVADA (SIN GIRO AUTOMATICO)]   " << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << (selectionCategory == SelectionCategory::DRIVER ? ">> " : "   ")
              << "[1] CORREDOR : " << driver.name << " (" << (selectedDriverIndex + 1)
              << "/" << drivers.size() << ") - " << driver.description << std::endl;
    std::cout << (selectionCategory == SelectionCategory::VEHICLE ? ">> " : "   ")
              << "[2] VEHICULO : " << vehicle.name << " (" << (selectedVehicleIndex + 1)
              << "/" << vehicles.size() << ") [" << vehicle.category << "]" << std::endl;
    std::cout << "      Stats: Vel: " << (vehicle.maxSpeed * 3.6f) << " km/h | Acel: " << vehicle.acceleration
              << " m/s2 | Giro: " << vehicle.turnSpeed << " deg/s | Cesped: "
              << static_cast<int>(vehicle.offRoadFriction * 100) << "% traccion" << std::endl;
    std::cout << (selectionCategory == SelectionCategory::TRACK ? ">> " : "   ")
              << "[3] PISTA    : " << track.name << " (" << (selectedTrackIndex + 1)
              << "/" << tracks.size() << ") [" << track.difficulty << "]" << std::endl;
    std::cout << "      Detalle: " << track.description << " (" << track.cols << "x" << track.rows << " celdas)" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "  [TAB / W / S o Flechas Arriba/Abajo]: Alternar Categoria" << std::endl;
    std::cout << "  [A / D o Flechas Izq/Der]           : Cambiar Opcion  " << std::endl;
    std::cout << "  [1]                                 : Elegir Corredor " << std::endl;
    std::cout << "  [2]                                 : Elegir Vehiculo " << std::endl;
    std::cout << "  [3]                                 : Elegir Pista    " << std::endl;
    std::cout << "  [ENTER / ESPACIO]                   : Iniciar Carrera " << std::endl;
    std::cout << "  [ESC]                               : Volver al Menu  " << std::endl;
    std::cout << "========================================================\n" << std::endl;
}

} // namespace core
