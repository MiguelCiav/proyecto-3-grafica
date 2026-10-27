#pragma once

#include "core/Window.hpp"
#include "core/Time.hpp"
#include "core/SceneManager.hpp"
#include "core/GameContext.hpp"
#include "audio/AudioManager.hpp"
#include <memory>
#include <string>

namespace core {

struct AppConfig {
    std::string title{"Prototipo Mario Kart - CGI (UCV)"};
    int width{1280};
    int height{720};
    bool vsync{true};
};

/**
 * @brief Orquestador principal de la aplicacion (Ciclo de vida, ventana, contexto y escenas).
 */
class Application {
public:
    explicit Application(const AppConfig& config = AppConfig());
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    bool init();
    void run();
    void close();

    Window& getWindow() { return *m_window; }
    const Time& getTime() const { return m_time; }
    GameState getGameState() const { return m_sceneManager.getCurrentState(); }
    void setGameState(GameState state);

    const renderer::Skybox& getSkybox() const { return m_gameContext.getCurrentSkybox(); }
    const renderer::Shader& getBoosterShader() const { return m_boosterShader; }
    const std::vector<world::BoosterPad>& getBoosterPads() const { return m_boosterPads; }
    vfx::KartParticleEmitter& getKartEmitter() { return m_kartEmitter; }
    const vfx::KartParticleEmitter& getKartEmitter() const { return m_kartEmitter; }

private:
    bool initWindowAndInput();
    bool initGraphicsResources();
    void initSubsystemsAndContext();
    void initContext();
    void loadCatalogs();
    void createPedestalMesh();

    AppConfig m_config;
    std::unique_ptr<Window> m_window;
    Time m_time;
    bool m_running{false};

    // Gestor de Escenas
    SceneManager m_sceneManager;

    // Recursos GPU y Shaders
    renderer::Shader m_basicShader;
    renderer::Shader m_litShader;
    renderer::Shader m_boosterShader;
    renderer::Texture m_paletteTexture;
    renderer::Skybox m_skyboxDay;
    renderer::Skybox m_skyboxNight;
    renderer::Skybox m_skyboxDesert;
    renderer::Skybox m_skyboxCosmic;
    renderer::Skybox m_skyboxAquatic;
    renderer::Skybox m_skyboxVolcano;

    // Mundo y Renderizado
    world::TrackGrid m_trackGrid;
    renderer::TrackRenderer m_trackRenderer;
    world::PropSpawner m_propSpawner;
    world::ZoneTriggerManager m_zoneManager;
    std::vector<world::BoosterPad> m_boosterPads;

    // Física y Dinámica
    Engine::Physics::CollisionSystem m_collisionSystem;
    Engine::Physics::KartPhysics m_kartPhysics;
    std::unique_ptr<vehicle::KartEntity> m_kartEntity;
    animation::WheelAnimator m_wheelAnimator;
    vfx::KartParticleEmitter m_kartEmitter;
    vfx::SkidmarkManager m_skidmarkManager;

    // Iluminación y Cámaras
    lighting::LightManager m_lightManager;
    lighting::LightingController m_lightingController;
    camera::Camera m_fpsCamera;
    camera::ChaseCamera m_chaseCamera;

    // HUD, Audio y Pedestal
    ui::RacingHud m_racingHud;
    std::unique_ptr<audio::AudioManager> m_audioManager;
    renderer::MeshGL m_pedestalMesh;

    // Contexto de Juego Compartido
    GameContext m_gameContext;
};

} // namespace core
