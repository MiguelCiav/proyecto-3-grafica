#pragma once

#include "core/Window.hpp"
#include "core/Time.hpp"
#include "camera/Camera.hpp"
#include "camera/ChaseCamera.hpp"
#include "physics/CollisionSystem.hpp"
#include "physics/KartPhysics.hpp"
#include "renderer/Shader.hpp"
#include "renderer/Texture.hpp"
#include "renderer/MeshGL.hpp"
#include "renderer/TrackRenderer.hpp"
#include "world/TrackGrid.hpp"
#include "world/PropSpawner.hpp"
#include "world/ZoneTrigger.hpp"
#include "geometry/KartPreprocessor.hpp"
#include <memory>
#include <string>
#include <vector>

namespace core {

enum class CameraMode {
    CHASE, // Conduccion en tercera persona siguiendo al kart
    FPS    // Inspeccion libre en primera persona restringida por colision
};

struct AppConfig {
    std::string title{"Prototipo Mario Kart - CGI (UCV)"};
    int width{1280};
    int height{720};
    bool vsync{true};
};

class Application {
public:
    static Application& getInstance();

    explicit Application(const AppConfig& config = AppConfig());
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool init();
    void run();
    void close();

    Window& getWindow() { return *m_window; }
    const Time& getTime() const { return m_time; }
    camera::Camera& getFPSCamera() { return m_camera; }
    camera::ChaseCamera& getChaseCamera() { return m_chaseCamera; }
    Engine::Physics::KartPhysics& getKart() { return m_kartPhysics; }

private:
    void processInput();
    void update(float deltaTime);
    void render();
    void setupColliders();

    AppConfig m_config;
    std::unique_ptr<Window> m_window;
    Time m_time;

    // Camaras
    CameraMode m_cameraMode{CameraMode::CHASE};
    camera::Camera m_camera;              // FPS Cam
    camera::ChaseCamera m_chaseCamera;    // 3rd person chase cam

    bool m_running{false};
    bool m_cursorLocked{false};

    // Shaders y Texturas
    renderer::Shader m_basicShader;
    renderer::Texture m_paletteTexture;

    // Mundo y Circuito Modular
    world::TrackGrid m_trackGrid;
    renderer::TrackRenderer m_trackRenderer;
    world::PropSpawner m_propSpawner;
    world::ZoneTriggerManager m_zoneManager;

    // Fisicas y Colisiones
    Engine::Physics::CollisionSystem m_collisionSystem;
    Engine::Physics::KartPhysics m_kartPhysics;

    // Mallas y Geometria del Kart articulado
    renderer::MeshGL m_chassisMesh;
    renderer::MeshGL m_driverMesh;
    renderer::MeshGL m_wheelFLMesh;
    renderer::MeshGL m_wheelFRMesh;
    renderer::MeshGL m_wheelBLMesh;
    renderer::MeshGL m_wheelBRMesh;

    // Offsets de pivote de ruedas (calculados en C++ en Fase 2)
    glm::vec3 m_wheelFLOffset{0.0f};
    glm::vec3 m_wheelFROffset{0.0f};
    glm::vec3 m_wheelBLOffset{0.0f};
    glm::vec3 m_wheelBROffset{0.0f};

    float m_wheelRollAngle{0.0f};
    glm::vec3 m_kartInitialPos{0.0f};
    float m_kartInitialYaw{90.0f};

    // Telemetria y cooldowns
    float m_finishLineCooldown{0.0f};
    float m_titleUpdateTimer{0.0f};
    int m_lapsCompleted{0};
};

} // namespace core
