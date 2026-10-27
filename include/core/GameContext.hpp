#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "core/SessionState.hpp"
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
#include "audio/IAudioService.hpp"

namespace core {

class SceneManager;
class Window;

/**
 * @brief Contexto compartido de recursos, estado de pista y vehiculo entre escenas.
 *        Hereda el estado puro de sesion (SessionState) y delega la configuracion
 *        ambiental del circuito en world::TrackEnvironmentConfigurator.
 */
struct GameContext : public SessionState {
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

    // Interfaz, Audio y Cámaras
    ui::RacingHud* racingHud{nullptr};
    audio::IAudioService* audioService{nullptr};
    camera::Camera* fpsCamera{nullptr};
    camera::ChaseCamera* chaseCamera{nullptr};

    [[nodiscard]] audio::IAudioService& getAudio() noexcept {
        if (audioService) {
            return *audioService;
        }
        static audio::NullAudioService s_nullAudio;
        return s_nullAudio;
    }

    [[nodiscard]] const camera::ICamera& getActiveCamera() const {
        return (cameraMode == CameraMode::CHASE)
            ? static_cast<const camera::ICamera&>(*chaseCamera)
            : static_cast<const camera::ICamera&>(*fpsCamera);
    }

    [[nodiscard]] SessionState& getSessionState() noexcept { return *this; }
    [[nodiscard]] const SessionState& getSessionState() const noexcept { return *this; }

    // Catálogo de Perfiles
    std::vector<world::DriverProfile> drivers;
    std::vector<world::VehicleProfile> vehicles;
    std::vector<world::TrackProfile> tracks;

    [[nodiscard]] const world::TrackProfile* getActiveTrack() const noexcept {
        if (tracks.empty() || selectedTrackIndex < 0 ||
            static_cast<size_t>(selectedTrackIndex) >= tracks.size()) {
            return nullptr;
        }
        return &tracks[selectedTrackIndex];
    }

    // Operaciones de Coordinación de Dominio
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
