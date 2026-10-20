#pragma once

#include "core/Window.hpp"
#include "core/Time.hpp"
#include "camera/Camera.hpp"
#include "camera/ChaseCamera.hpp"
#include "physics/CollisionSystem.hpp"
#include "physics/KartPhysics.hpp"
#include "animation/WheelAnimator.hpp"
#include "vehicle/KartEntity.hpp"
#include "renderer/Shader.hpp"
#include "renderer/Texture.hpp"
#include "renderer/TrackRenderer.hpp"
#include "world/TrackGrid.hpp"
#include "world/PropSpawner.hpp"
#include "world/ZoneTrigger.hpp"
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
    Engine::Physics::KartPhysics& getKartPhysics() { return m_kartPhysics; }
    vehicle::KartEntity& getKartEntity() { return m_kartEntity; }
    animation::WheelAnimator& getWheelAnimator() { return m_wheelAnimator; }

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

    // Mundo y Circuito Modular (12x12)
    world::TrackGrid m_trackGrid;
    renderer::TrackRenderer m_trackRenderer;
    world::PropSpawner m_propSpawner;
    world::ZoneTriggerManager m_zoneManager;

    // Fisicas y Colisiones
    Engine::Physics::CollisionSystem m_collisionSystem;
    Engine::Physics::KartPhysics m_kartPhysics;

    // Entidad jerarquica del Kart articulado (Fase 6 - Issues #21, #22, #23, #24)
    vehicle::KartEntity m_kartEntity;
    animation::WheelAnimator m_wheelAnimator;

    glm::vec3 m_kartInitialPos{0.0f};
    float m_kartInitialYaw{90.0f};

    // Telemetria y cooldowns
    float m_finishLineCooldown{8.0f};
    float m_titleUpdateTimer{0.0f};
    int m_lapsCompleted{0};
};

} // namespace core
