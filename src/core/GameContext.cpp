#include "core/GameContext.hpp"
#include "core/SceneManager.hpp"
#include "world/LevelLoader.hpp"
#include "world/TrackEnvironmentConfigurator.hpp"
#include <iostream>

namespace core {

const renderer::Skybox& GameContext::getCurrentSkybox() const {
    if (const auto* track = getActiveTrack()) {
        if (track->theme == world::TrackTheme::COSMIC_SPACE && skyboxCosmic && skyboxCosmic->isLoaded()) return *skyboxCosmic;
        if (track->theme == world::TrackTheme::AQUATIC_REEF && skyboxAquatic && skyboxAquatic->isLoaded()) return *skyboxAquatic;
        if (track->theme == world::TrackTheme::VOLCANO_CALDERA && skyboxVolcano && skyboxVolcano->isLoaded()) return *skyboxVolcano;
        if (track->theme == world::TrackTheme::DESERT_DUNES) {
            if (lightingController && lightingController->isNight() && skyboxNight && skyboxNight->isLoaded()) return *skyboxNight;
            if (skyboxDesert && skyboxDesert->isLoaded()) return *skyboxDesert;
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

    physics::VehicleDynamicsConfig cfg = kartPhysics->getDynamicsConfig();
    cfg.maxSpeed = vehicle.maxSpeed;
    cfg.acceleration = vehicle.acceleration;
    cfg.turnSpeed = vehicle.turnSpeed;
    cfg.drag = 5.0f;
    cfg.brakingRate = 22.0f;
    cfg.maxReverseSpeed = 4.5f;
    kartPhysics->setDynamicsConfig(cfg);
}

void GameContext::selectDriver(int index) {
    if (drivers.empty()) return;
    setDriverIndex(index, static_cast<int>(drivers.size()));
    applySelection();
    printSelectionHUD();
}

void GameContext::selectVehicle(int index) {
    if (vehicles.empty()) return;
    setVehicleIndex(index, static_cast<int>(vehicles.size()));
    applySelection();
    printSelectionHUD();
}

void GameContext::selectTrack(int index) {
    if (tracks.empty() || !trackGrid || !propSpawner || !zoneManager || !trackRenderer) return;
    setTrackIndex(index, static_cast<int>(tracks.size()));
    const auto& track = tracks[selectedTrackIndex];

    world::LevelLoader::loadFromFile(track.filePath, *trackGrid);
    propSpawner->generateForGrid(*trackGrid, world::TrackEnvironmentRegistry::getBiomeForTrack(track));
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
    if (wheelAnimator) wheelAnimator->reset();
    if (chaseCamera && kartPhysics) chaseCamera->reset(kartInitialPos, kartPhysics->getForwardVector());
    if (skidmarkManager) skidmarkManager->clear();

    printSelectionHUD();
}

void GameContext::setupColliders() {
    if (!collisionSystem || !propSpawner || !trackGrid) return;
    world::TrackEnvironmentConfigurator::setupColliders(*trackGrid, *propSpawner, *collisionSystem);
}

void GameContext::setupTrackLighting() {
    if (!lightManager || !lightingController || !propSpawner) return;
    world::TrackEnvironmentConfigurator::setupTrackLighting(
        getActiveTrack(), *propSpawner, *lightManager, *lightingController, trackRenderer
    );
}

void GameContext::applyDayNightState(bool isNight) {
    if (!lightManager || !lightingController || !trackRenderer || !propSpawner) return;
    world::TrackEnvironmentConfigurator::applyDayNightState(
        isNight, getActiveTrack(), *propSpawner, *lightManager, trackRenderer, lightingController
    );
}

void GameContext::setupBoosterPads() {
    if (!boosterPads || !trackGrid || !zoneManager) return;
    world::TrackEnvironmentConfigurator::setupBoosterPads(
        *trackGrid, *zoneManager, *boosterPads, kartInitialYaw
    );
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
