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
#include "renderer/MeshGL.hpp"
#include "world/TrackGrid.hpp"
#include "world/PropSpawner.hpp"
#include "world/ZoneTrigger.hpp"
#include "geometry/KartPreprocessor.hpp"
#include "lighting/LightManager.hpp"
#include "lighting/LightingController.hpp"
#include "renderer/Skybox.hpp"
#include "world/BoosterPad.hpp"
#include "vfx/KartParticleEmitter.hpp"
#include <memory>
#include <string>
#include <vector>

namespace core {

enum class GameState {
    MAIN_MENU,       // Pantalla de inicio con camara orbital cinematografica
    VEHICLE_SELECT,  // Showroom 3D con seleccion de corredor, vehiculo y pista de frente
    RACING           // Modo de juego de carreras activo
};

enum class SelectionCategory {
    DRIVER,   // [1] Seleccionando piloto / corredor
    VEHICLE,  // [2] Seleccionando vehiculo / carroceria
    TRACK     // [3] Seleccionando pista / circuito
};

enum class CameraMode {
    CHASE, // Conduccion en tercera persona siguiendo al kart
    FPS    // Inspeccion libre en primera persona restringida por colision
};

struct DriverProfile {
    std::string id;
    std::string name;
    std::string description;
    std::string sourcePath;
    geometry::SubMesh driverMesh;
    glm::vec4 accentColor{1.0f};
    bool loaded{false};
};

struct VehicleProfile {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    std::string modelPath;
    glm::vec3 driverOffset{0.0f};
    float maxSpeed{22.0f};
    float acceleration{18.0f};
    float turnSpeed{100.0f};
    float offRoadFriction{0.45f};
    geometry::KartAssembly assembly;
    bool loaded{false};
};

struct TrackProfile {
    std::string id;
    std::string name;
    std::string difficulty;
    std::string description;
    std::string filePath;
    int cols{12};
    int rows{12};
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
    GameState getGameState() const { return m_gameState; }
    void setGameState(GameState state);

    const renderer::Skybox& getSkybox() const { return m_skybox; }
    const renderer::Shader& getBoosterShader() const { return m_boosterShader; }
    const std::vector<world::BoosterPad>& getBoosterPads() const { return m_boosterPads; }
    vfx::KartParticleEmitter& getKartEmitter() { return m_kartEmitter; }
    const vfx::KartParticleEmitter& getKartEmitter() const { return m_kartEmitter; }

private:
    void processInput();
    void update(float deltaTime);
    void render();
    void setupColliders();
    void setupTrackLighting();
    void setupBoosterPads();
    void loadCatalog();
    void selectDriver(int index);
    void selectVehicle(int index);
    void selectTrack(int index);
    void applySelection();
    void printSelectionHUD();
    void createPedestalMesh();

    AppConfig m_config;
    std::unique_ptr<Window> m_window;
    Time m_time;

    // Estado del juego
    GameState m_gameState{GameState::MAIN_MENU};

    // Camaras
    CameraMode m_cameraMode{CameraMode::CHASE};
    camera::Camera m_camera;              // FPS Cam
    camera::ChaseCamera m_chaseCamera;    // 3rd person chase cam

    bool m_running{false};
    bool m_cursorLocked{false};

    // Shaders y Texturas
    renderer::Shader m_basicShader;
    renderer::Shader m_litShader;
    renderer::Shader m_boosterShader;
    renderer::Texture m_paletteTexture;
    renderer::Skybox m_skybox;

    // Mundo y Circuito Modular
    world::TrackGrid m_trackGrid;
    renderer::TrackRenderer m_trackRenderer;
    world::PropSpawner m_propSpawner;
    world::ZoneTriggerManager m_zoneManager;
    std::vector<world::BoosterPad> m_boosterPads;

    // Fisicas y Colisiones
    Engine::Physics::CollisionSystem m_collisionSystem;
    Engine::Physics::KartPhysics m_kartPhysics;

    // Entidad jerarquica del Kart articulado
    vehicle::KartEntity m_kartEntity;
    animation::WheelAnimator m_wheelAnimator;
    vfx::KartParticleEmitter m_kartEmitter;

    // Sistema y Controlador de Iluminacion
    lighting::LightManager m_lightManager;
    lighting::LightingController m_lightingController;

    // Catalogo de Corredores, Vehiculos y Pistas
    SelectionCategory m_selectionCategory{SelectionCategory::DRIVER};
    std::vector<DriverProfile> m_drivers;
    std::vector<VehicleProfile> m_vehicles;
    std::vector<TrackProfile> m_tracks;
    int m_selectedDriverIndex{0};
    int m_selectedVehicleIndex{0};
    int m_selectedTrackIndex{0};

    float m_showcaseAngle{0.0f}; // Fijo en 0 grados (de frente a la camara)
    float m_menuOrbitAngle{0.0f};
    renderer::MeshGL m_pedestalMesh;

    glm::vec3 m_kartInitialPos{0.0f};
    float m_kartInitialYaw{90.0f};

    // Telemetria y cooldowns
    float m_finishLineCooldown{8.0f};
    float m_titleUpdateTimer{0.0f};
    int m_lapsCompleted{0};
};

} // namespace core
