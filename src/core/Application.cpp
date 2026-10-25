#include "core/Application.hpp"
#include "core/Input.hpp"
#include "geometry/KartPreprocessor.hpp"
#include "scenes/MainMenuScene.hpp"
#include "scenes/ShowroomScene.hpp"
#include "scenes/RacingScene.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <stdexcept>
#include <cmath>

namespace core {

static Application* s_instance = nullptr;

Application& Application::getInstance() {
    if (!s_instance) {
        throw std::runtime_error("Application instance has not been initialized!");
    }
    return *s_instance;
}

Application::Application(const AppConfig& config)
    : m_config(config)
    , m_lightingController(&m_lightManager)
    , m_fpsCamera(glm::vec3(0.0f, 1.5f, 0.0f))
    , m_chaseCamera(3.0f, 1.3f, 1.2f)
{
    s_instance = this;
}

Application::~Application() {
    close();
    s_instance = nullptr;
}

void Application::initContext() {
    m_gameContext.sceneManager = &m_sceneManager;
    m_gameContext.window = m_window.get();
    m_gameContext.litShader = &m_litShader;
    m_gameContext.boosterShader = &m_boosterShader;
    m_gameContext.paletteTexture = &m_paletteTexture;
    m_gameContext.pedestalMesh = &m_pedestalMesh;

    m_gameContext.skyboxDay = &m_skyboxDay;
    m_gameContext.skyboxNight = &m_skyboxNight;
    m_gameContext.skyboxDesert = &m_skyboxDesert;
    m_gameContext.skyboxCosmic = &m_skyboxCosmic;
    m_gameContext.skyboxAquatic = &m_skyboxAquatic;
    m_gameContext.skyboxVolcano = &m_skyboxVolcano;

    m_gameContext.trackGrid = &m_trackGrid;
    m_gameContext.trackRenderer = &m_trackRenderer;
    m_gameContext.propSpawner = &m_propSpawner;
    m_gameContext.zoneManager = &m_zoneManager;
    m_gameContext.boosterPads = &m_boosterPads;

    m_gameContext.collisionSystem = &m_collisionSystem;
    m_gameContext.kartPhysics = &m_kartPhysics;
    m_gameContext.kartEntity = &m_kartEntity;
    m_gameContext.wheelAnimator = &m_wheelAnimator;
    m_gameContext.kartEmitter = &m_kartEmitter;
    m_gameContext.skidmarkManager = &m_skidmarkManager;

    m_gameContext.lightManager = &m_lightManager;
    m_gameContext.lightingController = &m_lightingController;

    m_gameContext.racingHud = &m_racingHud;
    m_gameContext.fpsCamera = &m_fpsCamera;
    m_gameContext.chaseCamera = &m_chaseCamera;
}

void Application::loadCatalogs() {
    m_gameContext.drivers = world::TrackEnvironmentRegistry::createDefaultDrivers();
    std::cout << "\n========================================================" << std::endl;
    std::cout << "  Cargando Catalogo de Pilotos y Corredores...          " << std::endl;
    std::cout << "========================================================" << std::endl;
    for (size_t i = 0; i < m_gameContext.drivers.size(); ++i) {
        auto& driver = m_gameContext.drivers[i];
        geometry::KartAssembly tempAssembly;
        if (geometry::KartPreprocessor::processKart(driver.sourcePath, tempAssembly)) {
            driver.driverMesh = tempAssembly.driver;
            driver.loaded = !driver.driverMesh.vertices.empty();
            std::cout << "  [" << (i + 1) << "/5] Piloto " << driver.name << " -> [CARGADO]" << std::endl;
        } else {
            std::cerr << "  [ERROR] Fallo al cargar piloto: " << driver.sourcePath << std::endl;
        }
    }

    m_gameContext.vehicles = world::TrackEnvironmentRegistry::createDefaultVehicles();
    std::cout << "\n========================================================" << std::endl;
    std::cout << "  Cargando Catalogo de Vehiculos para el Showroom...    " << std::endl;
    std::cout << "========================================================" << std::endl;
    for (size_t i = 0; i < m_gameContext.vehicles.size(); ++i) {
        auto& v = m_gameContext.vehicles[i];
        if (geometry::KartPreprocessor::processKart(v.modelPath, v.assembly)) {
            v.loaded = true;
            std::cout << "  [" << (i + 1) << "/" << m_gameContext.vehicles.size() << "] "
                      << v.name << " (" << v.category << ") -> [CARGADO]" << std::endl;
        } else {
            std::cerr << "  [ERROR] Fallo al cargar vehiculo: " << v.modelPath << std::endl;
        }
    }
    std::cout << "========================================================\n" << std::endl;

    m_gameContext.tracks = world::TrackEnvironmentRegistry::createDefaultTracks();
}

void Application::createPedestalMesh() {
    geometry::SubMesh pedestal;
    pedestal.name = "Showroom_Pedestal";

    const int segments = 36;
    const float radius = 1.75f;
    const float height = 0.12f;
    const float topY = 0.0f;
    const float botY = -height;

    // Vertice central superior
    pedestal.vertices.push_back({ {0.0f, topY, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f} });
    // Vertice central inferior
    pedestal.vertices.push_back({ {0.0f, botY, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f} });

    // Anillo de vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = glm::two_pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;

        // Borde superior
        pedestal.vertices.push_back({ {x, topY, z}, {0.0f, 1.0f, 0.0f}, {0.5f + 0.5f * std::cos(angle), 0.5f + 0.5f * std::sin(angle)} });
        // Borde lateral
        pedestal.vertices.push_back({ {x, botY, z}, {std::cos(angle), 0.0f, std::sin(angle)}, {0.5f, 0.5f} });
    }

    // Indices tapa superior (devanado CCW visto desde arriba)
    for (int i = 0; i < segments; ++i) {
        unsigned int curr = 2 + i * 2;
        unsigned int next = 2 + (i + 1) * 2;
        pedestal.indices.push_back(0);
        pedestal.indices.push_back(next);
        pedestal.indices.push_back(curr);

        // Lateral
        unsigned int currBot = curr + 1;
        unsigned int nextBot = next + 1;
        pedestal.indices.push_back(curr);
        pedestal.indices.push_back(currBot);
        pedestal.indices.push_back(next);

        pedestal.indices.push_back(next);
        pedestal.indices.push_back(currBot);
        pedestal.indices.push_back(nextBot);
    }

    m_pedestalMesh.setup(pedestal);
}

bool Application::init() {
    WindowProps props;
    props.title = m_config.title;
    props.width = m_config.width;
    props.height = m_config.height;

    m_window = std::make_unique<Window>(props);
    if (!m_window->init()) {
        std::cerr << "[Application] Error: Fallo al inicializar Window" << std::endl;
        return false;
    }

    m_window->setVSync(m_config.vsync);
    Input::init(m_window->getNativeWindow());

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    if (!m_litShader.loadFromFiles("shaders/lit.vert", "shaders/lit.frag")) {
        std::cerr << "[Application] Error: Fallo al compilar litShader" << std::endl;
        return false;
    }
    if (!m_boosterShader.loadFromFiles("shaders/booster_procedural.vert", "shaders/booster_procedural.frag")) {
        std::cerr << "[Application] Error: Fallo al compilar boosterShader" << std::endl;
        return false;
    }

    if (!m_paletteTexture.loadFromFile("assets/textures/colormap.png")) {
        std::cerr << "[Application] Advertencia: colormap.png no encontrado" << std::endl;
    }

    // Skyboxes (el diurno es .png; los tematicos se distribuyen como .tga)
    if (!m_skyboxDay.loadShader("shaders/skybox.vert", "shaders/skybox.frag") ||
        !m_skyboxDay.loadFromDirectory("assets/textures/skybox", ".png")) {
        std::cerr << "[AVISO] [Application] No se pudo cargar el Skybox diurno." << std::endl;
    }
    if (!m_skyboxNight.loadShader("shaders/skybox.vert", "shaders/skybox.frag") ||
        !m_skyboxNight.loadFromDirectory("assets/textures/skybox_night", ".tga")) {
        std::cerr << "[AVISO] [Application] No se pudo cargar el Skybox nocturno." << std::endl;
    }
    if (!m_skyboxDesert.loadShader("shaders/skybox.vert", "shaders/skybox.frag") ||
        !m_skyboxDesert.loadFromDirectory("assets/textures/skybox_desert", ".tga")) {
        std::cerr << "[AVISO] [Application] No se pudo cargar el Skybox del desierto." << std::endl;
    }
    if (!m_skyboxCosmic.loadShader("shaders/skybox.vert", "shaders/skybox.frag") ||
        !m_skyboxCosmic.loadFromDirectory("assets/textures/skybox_cosmic", ".tga")) {
        std::cerr << "[AVISO] [Application] No se pudo cargar el Skybox cosmico." << std::endl;
    }
    if (!m_skyboxAquatic.loadShader("shaders/skybox.vert", "shaders/skybox.frag") ||
        !m_skyboxAquatic.loadFromDirectory("assets/textures/skybox_aquatic", ".tga")) {
        std::cerr << "[AVISO] [Application] No se pudo cargar el Skybox acuatico." << std::endl;
    }
    if (!m_skyboxVolcano.loadShader("shaders/skybox.vert", "shaders/skybox.frag") ||
        !m_skyboxVolcano.loadFromDirectory("assets/textures/skybox_volcanic", ".tga")) {
        std::cerr << "[AVISO] [Application] No se pudo cargar el Skybox volcanico." << std::endl;
    }

    m_lightManager.init();
    m_lightingController.init(&m_lightManager);
    m_lightingController.setOnDayNightChanged([this](bool isNight) {
        m_gameContext.applyDayNightState(isNight);
    });
    m_lightingController.setOnPointLightsToggled([this]() {
        m_lightManager.togglePointLights();
    });

    m_trackRenderer.init();
    m_propSpawner.init();
    m_kartEmitter.init();
    if (!m_kartEmitter.loadShaders("shaders/particle.vert", "shaders/particle.frag")) {
        std::cerr << "[Application] Error: Fallo al cargar shaders de particulas" << std::endl;
    }
    m_skidmarkManager.init();
    m_racingHud.init();

    createPedestalMesh();
    initContext();
    loadCatalogs();

    // Seleccionar opciones iniciales (piloto 0, vehiculo 0, pista 0)
    m_gameContext.selectDriver(0);
    m_gameContext.selectVehicle(0);
    m_gameContext.selectTrack(0);

    // Registrar Escenas
    m_sceneManager.registerScene(GameState::MAIN_MENU, std::make_shared<scenes::MainMenuScene>(m_gameContext));
    m_sceneManager.registerScene(GameState::VEHICLE_SELECT, std::make_shared<scenes::ShowroomScene>(m_gameContext));
    m_sceneManager.registerScene(GameState::RACING, std::make_shared<scenes::RacingScene>(m_gameContext));

    // Estado inicial: MAIN_MENU
    m_sceneManager.changeScene(GameState::MAIN_MENU);

    std::cout << "\n========================================================" << std::endl;
    std::cout << "  PROTOTIPO MARIO KART - CGI PROYECTO #3 (UCV)          " << std::endl;
    std::cout << "  [MENU DEL JUEGO Y MULTIPLES PISTAS ACTIVOS]:          " << std::endl;
    std::cout << "    [1] o ENTER : Seleccionar Corredor, Auto y Pista    " << std::endl;
    std::cout << "    [2]         : Seleccion Directa de Vehiculo         " << std::endl;
    std::cout << "    [3]         : Seleccion Directa de Pista            " << std::endl;
    std::cout << "    [ESPACIO]   : Iniciar Carrera Inmediata             " << std::endl;
    std::cout << "    [ESC]       : Salir                                 " << std::endl;
    std::cout << "========================================================\n" << std::endl;

    m_running = true;
    m_time.reset();
    return true;
}

void Application::setGameState(GameState state) {
    m_sceneManager.changeScene(state);
}

void Application::run() {
    while (m_running && !m_window->shouldClose()) {
        m_time.update();
        float dt = m_time.getDeltaTime();

        // 1. Limpiar flags de teclas del fotograma anterior
        Input::update();

        // 2. Sondear eventos frescos del sistema operativo (GLFW dispara keyCallback aqui)
        m_window->pollEvents();

        // 3. Procesar las entradas capturadas en este fotograma
        m_sceneManager.handleInput(dt);

        // 4. Actualizar logica del juego
        m_sceneManager.update(dt);

        // 5. Renderizar escena activa
        m_sceneManager.render(m_window->getWidth(), m_window->getHeight());

        // 6. Presentar fotograma en pantalla
        m_window->swapBuffers();
    }
}

void Application::close() {
    m_running = false;
}

} // namespace core
