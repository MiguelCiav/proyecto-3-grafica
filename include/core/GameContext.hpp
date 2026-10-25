#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "camera/Camera.hpp"
#include "camera/ChaseCamera.hpp"
#include "physics/CollisionSystem.hpp"
#include "physics/KartPhysics.hpp"
#include "animation/WheelAnimator.hpp"
#include "vehicle/KartEntity.hpp"
#include "renderer/Shader.hpp"
#include "renderer/Texture.hpp"
#include "renderer/TrackRenderer.hpp"
#include "renderer/MeshGL.hpp"
#include "renderer/Skybox.hpp"
#include "world/TrackGrid.hpp"
#include "world/PropSpawner.hpp"
#include "world/ZoneTrigger.hpp"
#include "world/BoosterPad.hpp"
#include "world/TrackEnvironmentRegistry.hpp"
#include "lighting/LightManager.hpp"
#include "lighting/LightingController.hpp"
#include "vfx/KartParticleEmitter.hpp"
#include "vfx/SkidmarkManager.hpp"
#include "ui/RacingHud.hpp"

namespace core {

class SceneManager;
class Window;

enum class CameraMode {
    CHASE,
    FPS
};

enum class SelectionCategory {
    DRIVER,
    VEHICLE,
    TRACK
};

/**
 * @brief Contexto compartido de recursos, estado de pista y vehículo entre escenas.
 */
struct GameContext {
    SceneManager* sceneManager{nullptr};
    Window* window{nullptr};

    // Shaders y Recursos GPU
    renderer::Shader* litShader{nullptr};
    renderer::Shader* boosterShader{nullptr};
    renderer::Texture* paletteTexture{nullptr};
    renderer::MeshGL* pedestalMesh{nullptr};

    // Skyboxes
    renderer::Skybox* skyboxDay{nullptr};
    renderer::Skybox* skyboxNight{nullptr};
    renderer::Skybox* skyboxDesert{nullptr};
    renderer::Skybox* skyboxCosmic{nullptr};
    renderer::Skybox* skyboxAquatic{nullptr};
    renderer::Skybox* skyboxVolcano{nullptr};

    [[nodiscard]] const renderer::Skybox& getCurrentSkybox() const;

    // Mundo y Pista
    world::TrackGrid* trackGrid{nullptr};
    renderer::TrackRenderer* trackRenderer{nullptr};
    world::PropSpawner* propSpawner{nullptr};
    world::ZoneTriggerManager* zoneManager{nullptr};
    std::vector<world::BoosterPad>* boosterPads{nullptr};

    // Físicas y Vehículo
    Engine::Physics::CollisionSystem* collisionSystem{nullptr};
    Engine::Physics::KartPhysics* kartPhysics{nullptr};
    vehicle::KartEntity* kartEntity{nullptr};
    animation::WheelAnimator* wheelAnimator{nullptr};
    vfx::KartParticleEmitter* kartEmitter{nullptr};
    vfx::SkidmarkManager* skidmarkManager{nullptr};

    // Iluminación
    lighting::LightManager* lightManager{nullptr};
    lighting::LightingController* lightingController{nullptr};

    // Interfaz y Cámaras
    ui::RacingHud* racingHud{nullptr};
    camera::Camera* fpsCamera{nullptr};
    camera::ChaseCamera* chaseCamera{nullptr};
    CameraMode cameraMode{CameraMode::CHASE};

    [[nodiscard]] const camera::ICamera& getActiveCamera() const {
        return (cameraMode == CameraMode::CHASE)
            ? static_cast<const camera::ICamera&>(*chaseCamera)
            : static_cast<const camera::ICamera&>(*fpsCamera);
    }

    // Catálogo y Selección
    std::vector<world::DriverProfile> drivers;
    std::vector<world::VehicleProfile> vehicles;
    std::vector<world::TrackProfile> tracks;
    int selectedDriverIndex{0};
    int selectedVehicleIndex{0};
    int selectedTrackIndex{0};
    SelectionCategory selectionCategory{SelectionCategory::DRIVER};

    // Salida inicial
    glm::vec3 kartInitialPos{0.0f};
    float kartInitialYaw{90.0f};

    // Operaciones de Dominio
    void applySelection();
    void selectDriver(int index);
    void selectVehicle(int index);
    void selectTrack(int index);
    void setupColliders();
    void setupTrackLighting();
    void applyDayNightState(bool isNight);
    void setupBoosterPads();
    void printSelectionHUD() const;
};

} // namespace core
