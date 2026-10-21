#include "core/Application.hpp"
#include "core/Input.hpp"
#include "world/LevelLoader.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace core {

static Application* s_instance = nullptr;

Application& Application::getInstance() {
    return *s_instance;
}

Application::Application(const AppConfig& config)
    : m_config(config),
      m_camera(glm::vec3(0.0f, 4.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -15.0f),
      m_chaseCamera(4.2f, 1.8f, 1.5f) {
    s_instance = this;
}

Application::~Application() {
    s_instance = nullptr;
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

    // Indices tapa superior
    for (int i = 0; i < segments; ++i) {
        unsigned int curr = 2 + i * 2;
        unsigned int next = 2 + (i + 1) * 2;
        pedestal.indices.push_back(0);
        pedestal.indices.push_back(curr);
        pedestal.indices.push_back(next);

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

void Application::loadCatalog() {
    // -------------------------------------------------------------
    // 1. Catalogo de Corredores / Pilotos
    // -------------------------------------------------------------
    m_drivers = {
        {
            "driver-oobi",
            "Oobi (Verde)",
            "Personaje clasico, amistoso y equilibrado",
            "assets/models/karts/kart-oobi.obj",
            {}, glm::vec4(0.2f, 0.85f, 0.3f, 1.0f), false
        },
        {
            "driver-oodi",
            "Oodi (Azul)",
            "Piloto veloz con instinto aerodinamico",
            "assets/models/karts/kart-oodi.obj",
            {}, glm::vec4(0.2f, 0.5f, 0.95f, 1.0f), false
        },
        {
            "driver-ooli",
            "Ooli (Naranja)",
            "Especialista en terrenos rusticos y rally",
            "assets/models/karts/kart-ooli.obj",
            {}, glm::vec4(0.95f, 0.55f, 0.15f, 1.0f), false
        },
        {
            "driver-oopi",
            "Oopi (Rojo)",
            "Espiritu competitivo y aceleracion feroz",
            "assets/models/karts/kart-oopi.obj",
            {}, glm::vec4(0.9f, 0.2f, 0.2f, 1.0f), false
        },
        {
            "driver-oozi",
            "Oozi (Negro)",
            "Piloto sigiloso y maestro de curvas cerradas",
            "assets/models/karts/kart-oozi.obj",
            {}, glm::vec4(0.25f, 0.25f, 0.3f, 1.0f), false
        }
    };

    std::cout << "\n========================================================" << std::endl;
    std::cout << "  Cargando Catalogo de Pilotos y Corredores...          " << std::endl;
    std::cout << "========================================================" << std::endl;

    for (size_t i = 0; i < m_drivers.size(); ++i) {
        auto& driver = m_drivers[i];
        geometry::KartAssembly tempAssembly;
        if (geometry::KartPreprocessor::processKart(driver.sourcePath, tempAssembly)) {
            driver.driverMesh = tempAssembly.driver;
            driver.loaded = !driver.driverMesh.vertices.empty();
            std::cout << "  [" << (i + 1) << "/5] Piloto " << driver.name << " -> [CARGADO]" << std::endl;
        } else {
            std::cerr << "  [ERROR] Fallo al cargar piloto: " << driver.sourcePath << std::endl;
        }
    }

    // -------------------------------------------------------------
    // 2. Catalogo de Vehiculos / Carrocerias
    // -------------------------------------------------------------
    m_vehicles = {
        // --- Karts Oficiales Kenney ---
        {
            "kart-oobi",
            "Kart Clasico",
            "Kart",
            "Chasis ligero estandar, agil y facil de maniobrar",
            "assets/models/karts/kart-oobi.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            22.0f, 18.0f, 100.0f, 0.45f, {}, false
        },
        {
            "kart-oodi",
            "Kart Turbo",
            "Kart",
            "Aleron aerodinamico de alta velocidad punta",
            "assets/models/karts/kart-oodi.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            25.5f, 16.0f, 85.0f, 0.38f, {}, false
        },
        {
            "kart-ooli",
            "Kart Rally",
            "Kart",
            "Traccion total y suspension reforzada para curvas",
            "assets/models/karts/kart-ooli.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            21.0f, 20.0f, 105.0f, 0.65f, {}, false
        },
        {
            "kart-oopi",
            "Kart Furia",
            "Kart",
            "Aceleracion explosiva y turbo de salida rapida",
            "assets/models/karts/kart-oopi.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            23.5f, 22.0f, 95.0f, 0.40f, {}, false
        },
        {
            "kart-oozi",
            "Kart Nocturno",
            "Kart",
            "Maximo angulo de giro y derrape ultratecnico",
            "assets/models/karts/kart-oozi.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            22.5f, 19.0f, 120.0f, 0.44f, {}, false
        },
        // --- Vehiculos Adicionales Kenney Car-Kit ---
        {
            "race",
            "Formula Race F1",
            "Carreras",
            "Monoplaza de competicion ultraligero y aerodinamico",
            "assets/models/vehicles/race.obj",
            glm::vec3(0.0f, -0.04f, -0.12f),
            27.0f, 21.0f, 110.0f, 0.35f, {}, false
        },
        {
            "race-future",
            "Formula Future",
            "Carreras",
            "Prototipo futurista de vanguardia y maxima velocidad",
            "assets/models/vehicles/race-future.obj",
            glm::vec3(0.0f, -0.04f, -0.18f),
            28.5f, 23.0f, 105.0f, 0.32f, {}, false
        },
        {
            "police",
            "Patrulla Policia",
            "Especial",
            "Vehiculo de persecucion con gran estabilidad y sirena",
            "assets/models/vehicles/police.obj",
            glm::vec3(0.0f, -0.15f, 0.02f),
            23.0f, 19.0f, 90.0f, 0.45f, {}, false
        },
        {
            "taxi",
            "Taxi Deportivo",
            "Ciudad",
            "Iconico taxi urbano preparado para carreras callejeras",
            "assets/models/vehicles/taxi.obj",
            glm::vec3(0.0f, -0.15f, 0.02f),
            23.5f, 17.5f, 95.0f, 0.42f, {}, false
        },
        {
            "sedan-sports",
            "Sedan Sports",
            "Ciudad",
            "Sedan con aleron deportivo y agarre equilibrado",
            "assets/models/vehicles/sedan-sports.obj",
            glm::vec3(0.0f, -0.15f, 0.02f),
            24.5f, 18.5f, 92.0f, 0.40f, {}, false
        },
        {
            "suv",
            "Camioneta SUV 4x4",
            "Especial",
            "Traccion integral 4x4 con rueda de repuesto",
            "assets/models/vehicles/suv.obj",
            glm::vec3(0.0f, -0.15f, 0.02f),
            21.5f, 19.5f, 88.0f, 0.70f, {}, false
        }
    };

    std::cout << "\n========================================================" << std::endl;
    std::cout << "  Cargando Catalogo de Vehiculos para el Showroom...    " << std::endl;
    std::cout << "========================================================" << std::endl;

    for (size_t i = 0; i < m_vehicles.size(); ++i) {
        auto& v = m_vehicles[i];
        if (geometry::KartPreprocessor::processKart(v.modelPath, v.assembly)) {
            v.loaded = true;
            std::cout << "  [" << (i + 1) << "/" << m_vehicles.size() << "] "
                      << v.name << " (" << v.category << ") -> [CARGADO]" << std::endl;
        } else {
            std::cerr << "  [ERROR] Fallo al cargar vehiculo: " << v.modelPath << std::endl;
        }
    }
    std::cout << "========================================================\n" << std::endl;

    // -------------------------------------------------------------
    // 3. Catalogo de Circuitos y Pistas
    // -------------------------------------------------------------
    m_tracks = {
        {
            "track-oval",
            "Ovalo Clasico UCV",
            "Facil ★☆☆",
            "Circuito tradicional de alta velocidad con rectas largas y 4 curvas.",
            "assets/levels/track_oval.txt",
            12, 12
        },
        {
            "track-gp",
            "Chicana Grand Prix",
            "Media ★★☆",
            "Circuito tecnico F1 con chicanas en 'S' y curvas que exigen frenado.",
            "assets/levels/track_grand_prix.txt",
            14, 14
        },
        {
            "track-karto",
            "Kartodromo Compacto",
            "Tecnico ★★☆",
            "Trazado corto y frenetico de karts con curvas cerradas continuas.",
            "assets/levels/track_kartodromo.txt",
            10, 10
        },
        {
            "track-spd",
            "Super Speedway UCV",
            "Extremo ★★★",
            "Mega circuito gigante de 54 tramos disenado para maxima aceleracion.",
            "assets/levels/track_speedway.txt",
            17, 16
        },
        {
            "track-tech",
            "Desafio Doble Chicana",
            "Experto ★★★",
            "Trazado complejo con multiples curvas consecutivas y zonas estrechas.",
            "assets/levels/track_technical.txt",
            14, 14
        }
    };
}

void Application::applySelection() {
    if (m_vehicles.empty() || m_drivers.empty()) return;

    auto& vehicle = m_vehicles[m_selectedVehicleIndex];
    auto& driver = m_drivers[m_selectedDriverIndex];

    // Combinar la carroceria y ruedas del vehiculo con el piloto seleccionado
    geometry::KartAssembly combinedAssembly = vehicle.assembly;
    combinedAssembly.driver = driver.driverMesh;
    combinedAssembly.driver.pivotOffset = vehicle.driverOffset;

    // Configurar el grafo de escena articulado
    m_kartEntity.setup(combinedAssembly);

    // Actualizar fisicas segun el vehiculo elegido
    m_kartPhysics.m_maxSpeed = vehicle.maxSpeed;
    m_kartPhysics.m_acceleration = vehicle.acceleration;
    m_kartPhysics.m_turnSpeed = vehicle.turnSpeed;
    m_kartPhysics.m_drag = 7.0f;
}

void Application::selectDriver(int index) {
    if (m_drivers.empty()) return;
    int count = static_cast<int>(m_drivers.size());
    m_selectedDriverIndex = (index % count + count) % count;

    applySelection();
    printSelectionHUD();
}

void Application::selectVehicle(int index) {
    if (m_vehicles.empty()) return;
    int count = static_cast<int>(m_vehicles.size());
    m_selectedVehicleIndex = (index % count + count) % count;

    applySelection();
    printSelectionHUD();
}

void Application::selectTrack(int index) {
    if (m_tracks.empty()) return;
    int count = static_cast<int>(m_tracks.size());
    m_selectedTrackIndex = (index % count + count) % count;
    const auto& track = m_tracks[m_selectedTrackIndex];

    // Cargar nuevo archivo de pista modular
    world::LevelLoader::loadFromFile(track.filePath, m_trackGrid);

    // Regenerar obstaculos y triggers para la nueva pista
    m_propSpawner.generateForGrid(m_trackGrid);
    m_zoneManager.autoGenerateFromGrid(m_trackGrid);

    // Actualizar colisionadores
    setupColliders();

    // Actualizar posicion y rumbo de salida
    m_kartInitialPos = glm::vec3(0.0f, 0.0f, 0.0f);
    m_kartInitialYaw = 90.0f;

    for (const auto& tile : m_trackGrid.getAllTiles()) {
        if (tile.type == world::TileType::START_FINISH) {
            m_kartInitialPos = tile.worldPos;
            m_kartInitialYaw = tile.rotationDegrees;
            break;
        }
    }

    // Reposicionar kart y camara
    m_kartPhysics.setPosition(m_kartInitialPos);
    m_kartPhysics.setYaw(m_kartInitialYaw);
    m_kartPhysics.resetSpeed();
    m_wheelAnimator.reset();
    m_chaseCamera.reset(m_kartInitialPos, m_kartPhysics.getForwardVector());
    m_lapsCompleted = 0;

    printSelectionHUD();
}

void Application::printSelectionHUD() {
    const auto& driver = m_drivers[m_selectedDriverIndex];
    const auto& vehicle = m_vehicles[m_selectedVehicleIndex];
    const auto& track = m_tracks[m_selectedTrackIndex];

    std::cout << "\n========================================================" << std::endl;
    std::cout << "  SHOWROOM 3D - CONFIGURACION DE CARRERA MARIO KART     " << std::endl;
    std::cout << "  [VISTA FRONTAL FIJA ACTIVADA (SIN GIRO AUTOMATICO)]   " << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << (m_selectionCategory == SelectionCategory::DRIVER ? ">> " : "   ")
              << "[1] CORREDOR : " << driver.name << " (" << (m_selectedDriverIndex + 1)
              << "/" << m_drivers.size() << ") - " << driver.description << std::endl;
    std::cout << (m_selectionCategory == SelectionCategory::VEHICLE ? ">> " : "   ")
              << "[2] VEHICULO : " << vehicle.name << " (" << (m_selectedVehicleIndex + 1)
              << "/" << m_vehicles.size() << ") [" << vehicle.category << "]" << std::endl;
    std::cout << "      Stats: Vel: " << (vehicle.maxSpeed * 3.6f) << " km/h | Acel: " << vehicle.acceleration
              << " m/s2 | Giro: " << vehicle.turnSpeed << " deg/s | Cesped: "
              << static_cast<int>(vehicle.offRoadFriction * 100) << "% traccion" << std::endl;
    std::cout << (m_selectionCategory == SelectionCategory::TRACK ? ">> " : "   ")
              << "[3] PISTA    : " << track.name << " (" << (m_selectedTrackIndex + 1)
              << "/" << m_tracks.size() << ") [" << track.difficulty << "]" << std::endl;
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

void Application::setGameState(GameState state) {
    m_gameState = state;

    if (m_gameState == GameState::RACING) {
        m_cursorLocked = false;
        Input::setCursorVisible(true);
        m_cameraMode = CameraMode::CHASE;
        m_kartPhysics.setPosition(m_kartInitialPos);
        m_kartPhysics.setYaw(m_kartInitialYaw);
        m_wheelAnimator.reset();
        m_chaseCamera.reset(m_kartInitialPos, m_kartPhysics.getForwardVector());
        std::cout << "\n>>> [CARRERA INICIADA] Pista: " << m_tracks[m_selectedTrackIndex].name
                  << " | Conduce con W/S/A/D | [M: Cambiar Auto/Pista] [ESC: Menu] <<<\n" << std::endl;
    } else if (m_gameState == GameState::VEHICLE_SELECT) {
        m_cursorLocked = false;
        Input::setCursorVisible(true);
        m_showcaseAngle = 0.0f; // Vista frontal fija obligatoria
        applySelection();
        printSelectionHUD();
    } else if (m_gameState == GameState::MAIN_MENU) {
        m_cursorLocked = false;
        Input::setCursorVisible(true);
    }
}

void Application::setupColliders() {
    m_collisionSystem.clear();

    // 1. Registrar obstaculos generados (barreras y conos) con sus AABB exactos
    for (const auto& prop : m_propSpawner.getProps()) {
        m_collisionSystem.addStaticBox(Engine::Physics::BoundingBox(prop.aabbMin, prop.aabbMax));
    }

    // 2. Registrar limites perimetrales exteriores adaptados al tamano de la pista actual
    float halfWidth = (m_trackGrid.getCols() * world::TrackGrid::TILE_SIZE) * 0.5f + 4.0f;
    float halfDepth = (m_trackGrid.getRows() * world::TrackGrid::TILE_SIZE) * 0.5f + 4.0f;
    float wallHeight = 6.0f;

    m_collisionSystem.addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(-halfWidth, 0.0f, halfDepth),
        glm::vec3(halfWidth, wallHeight, halfWidth + 2.0f)
    ));
    m_collisionSystem.addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(-halfWidth, 0.0f, -halfDepth - 2.0f),
        glm::vec3(halfWidth, wallHeight, -halfDepth)
    ));
    m_collisionSystem.addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(halfWidth, 0.0f, -halfDepth),
        glm::vec3(halfWidth + 2.0f, wallHeight, halfDepth)
    ));
    m_collisionSystem.addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(-halfWidth - 2.0f, 0.0f, -halfDepth),
        glm::vec3(-halfWidth, wallHeight, halfDepth)
    ));

    std::cout << "[OK] [CollisionSystem] Registrados " << m_collisionSystem.getStaticBoxes().size()
              << " colisionadores estaticos adaptados al circuito (" << m_trackGrid.getCols()
              << "x" << m_trackGrid.getRows() << ")." << std::endl;
}

bool Application::init() {
    WindowProps props;
    props.title = m_config.title;
    props.width = m_config.width;
    props.height = m_config.height;
    props.vsync = m_config.vsync;

    m_window = std::make_unique<Window>(props);
    if (!m_window->init()) {
        std::cerr << "[ERROR] [Application] Fallo al inicializar la ventana." << std::endl;
        return false;
    }

    Input::init(m_window->getNativeWindow());

    // 1. Shaders y Texturas
    if (!m_basicShader.loadFromFiles("shaders/basic.vert", "shaders/basic.frag")) {
        std::cerr << "[ERROR] [Application] Fallo al cargar shaders basicos." << std::endl;
        return false;
    }

    if (!m_paletteTexture.loadFromFile("assets/textures/colormap.png")) {
        std::cerr << "[AVISO] [Application] No se pudo cargar colormap.png" << std::endl;
    }

    // 2. Inicializar renderizadores
    if (!m_trackRenderer.init("assets/models/track")) {
        std::cerr << "[ERROR] [Application] Fallo al inicializar TrackRenderer." << std::endl;
        return false;
    }

    if (!m_propSpawner.init("assets/models/obstacles")) {
        std::cerr << "[AVISO] [Application] Fallo al inicializar PropSpawner." << std::endl;
    }

    // 3. Crear pedestal para el showroom 3D y cargar catalogo completo
    createPedestalMesh();
    loadCatalog();

    // 4. Inicializar sistema y controlador de iluminacion (Fase 7 - Issues #25, #27)
    m_lightManager.init();
    m_lightingController.init(&m_lightManager);

    // 4. Cargar la primera pista por defecto (Ovalo Clasico)
    selectTrack(0);

    // 5. Seleccionar el primer corredor y vehiculo por defecto
    selectDriver(0);
    selectVehicle(0);

    // 6. Iniciar en el Menu Principal con opciones
    setGameState(GameState::MAIN_MENU);

    std::cout << "\n========================================================" << std::endl;
    std::cout << "  PROTOTIPO MARIO KART - CGI PROYECTO #3 (UCV)          " << std::endl;
    std::cout << "  [MENU DEL JUEGO Y MULTIPLES PISTAS ACTIVOS]:          " << std::endl;
    std::cout << "    [1] o ENTER : Seleccionar Corredor, Auto y Pista    " << std::endl;
    std::cout << "    [2]         : Seleccion Directa de Vehiculo         " << std::endl;
    std::cout << "    [3]         : Seleccion Directa de Pista            " << std::endl;
    std::cout << "    [ESPACIO]   : Iniciar Carrera Inmediata             " << std::endl;
    std::cout << "    [ESC]       : Salir                                 " << std::endl;
    std::cout << "========================================================\n" << std::endl;

    m_time.reset();
    m_running = true;
    return true;
}

void Application::processInput() {
    // -------------------------------------------------------------
    // Control en MAIN_MENU
    // -------------------------------------------------------------
    if (m_gameState == GameState::MAIN_MENU) {
        if (Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            std::cout << "[MENU] Saliendo del juego..." << std::endl;
            close();
            return;
        }
        if (Input::isKeyJustPressed(GLFW_KEY_1) || Input::isKeyJustPressed(GLFW_KEY_KP_1) ||
            Input::isKeyJustPressed(GLFW_KEY_ENTER) || Input::isKeyJustPressed(GLFW_KEY_KP_ENTER)) {
            std::cout << "[MENU] Opcion [1 / ENTER]: Abriendo Configuracion (Corredor)..." << std::endl;
            m_selectionCategory = SelectionCategory::DRIVER;
            setGameState(GameState::VEHICLE_SELECT);
            return;
        }
        if (Input::isKeyJustPressed(GLFW_KEY_2) || Input::isKeyJustPressed(GLFW_KEY_KP_2)) {
            std::cout << "[MENU] Opcion [2]: Abriendo Configuracion (Vehiculo)..." << std::endl;
            m_selectionCategory = SelectionCategory::VEHICLE;
            setGameState(GameState::VEHICLE_SELECT);
            return;
        }
        if (Input::isKeyJustPressed(GLFW_KEY_3) || Input::isKeyJustPressed(GLFW_KEY_KP_3)) {
            std::cout << "[MENU] Opcion [3]: Abriendo Configuracion (Pista)..." << std::endl;
            m_selectionCategory = SelectionCategory::TRACK;
            setGameState(GameState::VEHICLE_SELECT);
            return;
        }
        if (Input::isKeyJustPressed(GLFW_KEY_SPACE)) {
            std::cout << "[MENU] Opcion [ESPACIO]: Iniciando Carrera Inmediata..." << std::endl;
            setGameState(GameState::RACING);
            return;
        }
        return;
    }

    // -------------------------------------------------------------
    // Control en VEHICLE_SELECT (Showroom)
    // -------------------------------------------------------------
    if (m_gameState == GameState::VEHICLE_SELECT) {
        if (Input::isKeyJustPressed(GLFW_KEY_ESCAPE) || Input::isKeyJustPressed(GLFW_KEY_BACKSPACE)) {
            std::cout << "[SHOWROOM] Regresando al Menu Principal..." << std::endl;
            setGameState(GameState::MAIN_MENU);
            return;
        }

        // Alternar circularmente categoria con TAB, W, S, Arriba, Abajo
        if (Input::isKeyJustPressed(GLFW_KEY_TAB) || Input::isKeyJustPressed(GLFW_KEY_S) || Input::isKeyJustPressed(GLFW_KEY_DOWN)) {
            if (m_selectionCategory == SelectionCategory::DRIVER) m_selectionCategory = SelectionCategory::VEHICLE;
            else if (m_selectionCategory == SelectionCategory::VEHICLE) m_selectionCategory = SelectionCategory::TRACK;
            else m_selectionCategory = SelectionCategory::DRIVER;
            printSelectionHUD();
        } else if (Input::isKeyJustPressed(GLFW_KEY_W) || Input::isKeyJustPressed(GLFW_KEY_UP)) {
            if (m_selectionCategory == SelectionCategory::DRIVER) m_selectionCategory = SelectionCategory::TRACK;
            else if (m_selectionCategory == SelectionCategory::TRACK) m_selectionCategory = SelectionCategory::VEHICLE;
            else m_selectionCategory = SelectionCategory::DRIVER;
            printSelectionHUD();
        }

        // Acceso directo por tecla numerica (estandar y keypad)
        if (Input::isKeyJustPressed(GLFW_KEY_1) || Input::isKeyJustPressed(GLFW_KEY_KP_1)) {
            m_selectionCategory = SelectionCategory::DRIVER;
            printSelectionHUD();
        }
        if (Input::isKeyJustPressed(GLFW_KEY_2) || Input::isKeyJustPressed(GLFW_KEY_KP_2)) {
            m_selectionCategory = SelectionCategory::VEHICLE;
            printSelectionHUD();
        }
        if (Input::isKeyJustPressed(GLFW_KEY_3) || Input::isKeyJustPressed(GLFW_KEY_KP_3)) {
            m_selectionCategory = SelectionCategory::TRACK;
            printSelectionHUD();
        }

        // Navegacion entre opciones de la categoria activa (A/D o Flechas Izq/Der)
        if (Input::isKeyJustPressed(GLFW_KEY_A) || Input::isKeyJustPressed(GLFW_KEY_LEFT)) {
            if (m_selectionCategory == SelectionCategory::DRIVER) {
                selectDriver(m_selectedDriverIndex - 1);
            } else if (m_selectionCategory == SelectionCategory::VEHICLE) {
                selectVehicle(m_selectedVehicleIndex - 1);
            } else {
                selectTrack(m_selectedTrackIndex - 1);
            }
        }
        if (Input::isKeyJustPressed(GLFW_KEY_D) || Input::isKeyJustPressed(GLFW_KEY_RIGHT)) {
            if (m_selectionCategory == SelectionCategory::DRIVER) {
                selectDriver(m_selectedDriverIndex + 1);
            } else if (m_selectionCategory == SelectionCategory::VEHICLE) {
                selectVehicle(m_selectedVehicleIndex + 1);
            } else {
                selectTrack(m_selectedTrackIndex + 1);
            }
        }

        // Confirmar seleccion y comenzar la carrera
        if (Input::isKeyJustPressed(GLFW_KEY_ENTER) || Input::isKeyJustPressed(GLFW_KEY_KP_ENTER) ||
            Input::isKeyJustPressed(GLFW_KEY_SPACE)) {
            std::cout << "[SHOWROOM] Seleccion confirmada! Iniciando carrera en "
                      << m_tracks[m_selectedTrackIndex].name << "..." << std::endl;
            setGameState(GameState::RACING);
        }
        return;
    }

    // -------------------------------------------------------------
    // Control en RACING (Carrera activa)
    // -------------------------------------------------------------
    if (m_gameState == GameState::RACING) {
        // Tecla M: Volver al Showroom a cambiar de auto, piloto o pista
        if (Input::isKeyJustPressed(GLFW_KEY_M)) {
            setGameState(GameState::VEHICLE_SELECT);
            return;
        }

        // ESC: Liberar cursor en modo FPS o volver al menu principal
        if (Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            if (m_cameraMode == CameraMode::FPS && m_cursorLocked) {
                m_cursorLocked = false;
                Input::setCursorVisible(true);
            } else {
                setGameState(GameState::MAIN_MENU);
                return;
            }
        }

        // Conmutador de camara (Tecla C)
        if (Input::isKeyJustPressed(GLFW_KEY_C)) {
            if (m_cameraMode == CameraMode::CHASE) {
                m_cameraMode = CameraMode::FPS;
                m_cursorLocked = true;
                Input::setCursorVisible(false);
                m_camera.setPosition(m_chaseCamera.getPosition());
                m_camera.lookAtTarget(m_kartPhysics.getPosition() + glm::vec3(0.0f, 1.0f, 0.0f));
                std::cout << "[Camara] Modo: INSPECCION LIBRE FPS (Restriccion de Colision ON)" << std::endl;
            } else {
                m_cameraMode = CameraMode::CHASE;
                m_cursorLocked = false;
                Input::setCursorVisible(true);
                m_chaseCamera.reset(m_kartPhysics.getPosition(), m_kartPhysics.getForwardVector());
                std::cout << "[Camara] Modo: CONDUCCION TERCERA PERSONA (Chase Cam)" << std::endl;
            }
        }

        // Reinicio de kart (Tecla R)
        if (Input::isKeyJustPressed(GLFW_KEY_R)) {
            m_kartPhysics.setPosition(m_kartInitialPos);
            m_kartPhysics.setYaw(m_kartInitialYaw);
            m_wheelAnimator.reset();
            m_chaseCamera.reset(m_kartInitialPos, m_kartPhysics.getForwardVector());
            std::cout << "[Kart] Posicion reiniciada a la linea de salida de "
                      << m_tracks[m_selectedTrackIndex].name << "." << std::endl;
        }

        // Controlador de Iluminacion y Faros (Fase 7 - Issue #27)
        // [1]: Flat Shading | [2]: Lambert | [3]: Blinn-Phong | [L]: Toggle Faros Kart
        m_lightingController.handleInput();

        // En modo FPS: alternar captura de mouse con clic o TAB
        if (m_cameraMode == CameraMode::FPS) {
            if (Input::isKeyJustPressed(GLFW_KEY_TAB) || (!m_cursorLocked && Input::isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT))) {
                m_cursorLocked = !m_cursorLocked;
                Input::setCursorVisible(!m_cursorLocked);
            }

            if (m_cursorLocked) {
                float dt = m_time.getDeltaTime();
                if (Input::isKeyPressed(GLFW_KEY_W)) m_camera.processKeyboardWithCollision(camera::CameraMovement::FORWARD, dt, m_collisionSystem);
                if (Input::isKeyPressed(GLFW_KEY_S)) m_camera.processKeyboardWithCollision(camera::CameraMovement::BACKWARD, dt, m_collisionSystem);
                if (Input::isKeyPressed(GLFW_KEY_A)) m_camera.processKeyboardWithCollision(camera::CameraMovement::LEFT, dt, m_collisionSystem);
                if (Input::isKeyPressed(GLFW_KEY_D)) m_camera.processKeyboardWithCollision(camera::CameraMovement::RIGHT, dt, m_collisionSystem);
                if (Input::isKeyPressed(GLFW_KEY_SPACE) || Input::isKeyPressed(GLFW_KEY_E))
                    m_camera.processKeyboardWithCollision(camera::CameraMovement::UP, dt, m_collisionSystem);
                if (Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT) || Input::isKeyPressed(GLFW_KEY_Q))
                    m_camera.processKeyboardWithCollision(camera::CameraMovement::DOWN, dt, m_collisionSystem);

                float dx = Input::getMouseDeltaX();
                float dy = Input::getMouseDeltaY();
                if (dx != 0.0f || dy != 0.0f) {
                    m_camera.processMouseMovement(dx, dy);
                }

                float scroll = Input::getScrollDeltaY();
                if (scroll != 0.0f) {
                    m_camera.processMouseScroll(scroll);
                }
            }
        }
    }
}

void Application::update(float deltaTime) {
    // -------------------------------------------------------------
    // Actualizacion en MAIN_MENU: Camara orbital cinematografica
    // -------------------------------------------------------------
    if (m_gameState == GameState::MAIN_MENU) {
        m_menuOrbitAngle += deltaTime * 12.0f;
        if (m_menuOrbitAngle > 360.0f) m_menuOrbitAngle -= 360.0f;

        // Actualizar titulo con opciones de menu
        m_titleUpdateTimer += deltaTime;
        if (m_titleUpdateTimer >= 0.25f) {
            m_titleUpdateTimer = 0.0f;
            std::stringstream ss;
            ss << m_config.title << " | [MENU] [1: Corredor] [2: Auto] [3: Pista: "
               << m_tracks[m_selectedTrackIndex].name << "] [ESPACIO: Correr] [ESC: Salir]";
            m_window->setTitle(ss.str());
        }
        return;
    }

    // -------------------------------------------------------------
    // Actualizacion en VEHICLE_SELECT: Showroom Vista Frontal Fija
    // -------------------------------------------------------------
    if (m_gameState == GameState::VEHICLE_SELECT) {
        // En el showroom el kart se mantiene FIJO DE FRENTE (sin vueltas)
        m_showcaseAngle = 0.0f;

        // Kart en el pedestal del showroom mirando de frente
        m_wheelAnimator.update(deltaTime, 0.0f, 0.0f);
        m_kartEntity.update(glm::vec3(0.0f, 0.05f, 0.0f), m_showcaseAngle, m_wheelAnimator);

        m_titleUpdateTimer += deltaTime;
        if (m_titleUpdateTimer >= 0.20f) {
            m_titleUpdateTimer = 0.0f;
            const auto& d = m_drivers[m_selectedDriverIndex];
            const auto& v = m_vehicles[m_selectedVehicleIndex];
            const auto& t = m_tracks[m_selectedTrackIndex];
            std::stringstream ss;
            ss << m_config.title << " | SHOWROOM ["
               << (m_selectionCategory == SelectionCategory::DRIVER ? ">> 1.CORREDOR: " : "1.Piloto: ") << d.name << " | "
               << (m_selectionCategory == SelectionCategory::VEHICLE ? ">> 2.VEHICULO: " : "2.Auto: ") << v.name << " | "
               << (m_selectionCategory == SelectionCategory::TRACK ? ">> 3.PISTA: " : "3.Pista: ") << t.name
               << "] | [TAB] Categoria | [A/D] Cambiar | [ENTER] Correr";
            m_window->setTitle(ss.str());
        }
        return;
    }

    // -------------------------------------------------------------
    // Actualizacion en RACING
    // -------------------------------------------------------------
    if (m_finishLineCooldown > 0.0f) {
        m_finishLineCooldown -= deltaTime;
    }

    // 1. Entrada de conduccion del kart
    bool accel = false;
    bool brake = false;
    float steer = 0.0f;

    if (m_cameraMode == CameraMode::CHASE) {
        accel = Input::isKeyPressed(GLFW_KEY_W) || Input::isKeyPressed(GLFW_KEY_UP);
        brake = Input::isKeyPressed(GLFW_KEY_S) || Input::isKeyPressed(GLFW_KEY_DOWN);
        if (Input::isKeyPressed(GLFW_KEY_A) || Input::isKeyPressed(GLFW_KEY_LEFT))  steer -= 1.0f;
        if (Input::isKeyPressed(GLFW_KEY_D) || Input::isKeyPressed(GLFW_KEY_RIGHT)) steer += 1.0f;
    } else {
        // En modo inspeccion FPS, se usan las flechas para manejar el kart en segundo plano
        accel = Input::isKeyPressed(GLFW_KEY_UP);
        brake = Input::isKeyPressed(GLFW_KEY_DOWN);
        if (Input::isKeyPressed(GLFW_KEY_LEFT))  steer -= 1.0f;
        if (Input::isKeyPressed(GLFW_KEY_RIGHT)) steer += 1.0f;
    }

    // 2. Evaluacion de superficie (Cesped vs Asfalto)
    glm::vec3 kartPos = m_kartPhysics.getPosition();
    const auto* currentTile = m_trackGrid.getTileAtWorldPos(kartPos.x, kartPos.z);
    if (currentTile == nullptr || currentTile->type == world::TileType::EMPTY) {
        m_kartPhysics.setSurfaceFriction(m_vehicles[m_selectedVehicleIndex].offRoadFriction);
    } else {
        m_kartPhysics.setSurfaceFriction(1.0f);
    }

    // 3. Actualizar modelo de dinamica de vehiculo
    m_kartPhysics.update(deltaTime, accel, brake, steer);

    // 4. Resolucion de colisiones del kart contra barreras/conos con esfera suave
    glm::vec3 mtv(0.0f);
    if (m_collisionSystem.checkCollision(m_kartPhysics.getBoundingSphere(), mtv)) {
        m_kartPhysics.onCollision(mtv, 0.25f);
    }

    // 5. Evaluacion de zonas de boost y meta
    const world::SpecialZone* hitZone = m_zoneManager.checkTrigger(m_kartPhysics.getPosition());
    if (hitZone != nullptr) {
        if (hitZone->type == world::ZoneType::BOOST) {
            m_kartPhysics.applyBoost(hitZone->durationSeconds);
        } else if (hitZone->type == world::ZoneType::FINISH_LINE && m_finishLineCooldown <= 0.0f && m_kartPhysics.getSpeed() > 1.5f) {
            m_lapsCompleted++;
            m_finishLineCooldown = 5.0f;
            std::cout << ">>> [META!] Vuelta " << m_lapsCompleted << " completada en "
                      << m_tracks[m_selectedTrackIndex].name << " con "
                      << m_vehicles[m_selectedVehicleIndex].name << " ("
                      << m_drivers[m_selectedDriverIndex].name << ")! <<<" << std::endl;
        }
    }

    // 6. Actualizar cinematica de ruedas y Grafo de Escena
    m_wheelAnimator.update(deltaTime, m_kartPhysics.getSpeed(), steer);
    m_kartEntity.update(m_kartPhysics.getPosition(), m_kartPhysics.getYaw(), m_wheelAnimator);

    // 6.b Sincronizar faros moviles del kart (SpotLight) con la posicion y giro del morro (Fase 7 - Issue #27)
    m_lightingController.update(m_kartPhysics.getPosition(), m_kartPhysics.getForwardVector(), deltaTime);

    // 7. Actualizar camaras
    m_chaseCamera.update(deltaTime, m_kartPhysics.getPosition(), m_kartPhysics.getForwardVector());
    if (m_cameraMode == CameraMode::FPS) {
        m_camera.resolveCollision(m_collisionSystem);
    }

    // 8. Titulo con telemetria en tiempo real y estado de iluminacion (Issue #27)
    m_titleUpdateTimer += deltaTime;
    if (m_titleUpdateTimer >= 0.20f) {
        m_titleUpdateTimer = 0.0f;
        std::stringstream ss;
        ss << m_config.title << " | " << static_cast<int>(m_time.getFPS()) << " FPS | ";
        if (m_cameraMode == CameraMode::CHASE) {
            ss << "[" << m_vehicles[m_selectedVehicleIndex].name << " @ " << m_tracks[m_selectedTrackIndex].name << "] ";
            ss << "| Vel: " << std::fixed << std::setprecision(1) << (m_kartPhysics.getSpeed() * 3.6f) << " km/h";
            if (m_kartPhysics.isBoosting()) ss << " [TURBO!]";
            if (m_kartPhysics.getSurfaceFriction() < 0.9f) ss << " [OFF-ROAD]";
            ss << " | Vueltas: " << m_lapsCompleted;
        } else {
            ss << "[Modo Inspeccion FPS (Colisiones ON)] | Pos: ("
               << std::setprecision(1) << m_camera.getPosition().x << ", "
               << m_camera.getPosition().y << ", "
               << m_camera.getPosition().z << ")";
        }
        ss << " | [" << m_lightingController.getLightingModelName()
           << " | Faros: " << (m_lightingController.areHeadlightsOn() ? "ON" : "OFF") << "]";
        ss << " | [1/2/3: Luz] [L: Faros] [M: Menu] [C: Camara] [R: Reset] [ESC: Salir]";
        m_window->setTitle(ss.str());
    }
}

void Application::render() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    float aspect = m_window->getAspectRatio();

    // -------------------------------------------------------------
    // RENDERIZADO: MAIN_MENU (Vista panoramica orbital)
    // -------------------------------------------------------------
    if (m_gameState == GameState::MAIN_MENU) {
        m_window->clear(0.40f, 0.68f, 0.95f, 1.0f);
        if (!m_basicShader.isValid()) return;

        m_basicShader.use();

        float rad = glm::radians(m_menuOrbitAngle);
        float orbitDist = (m_trackGrid.getCols() * world::TrackGrid::TILE_SIZE) * 0.65f;
        glm::vec3 camPos(std::sin(rad) * orbitDist, 15.0f, std::cos(rad) * orbitDist);
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(55.0f), aspect, 0.1f, 1000.0f);

        m_basicShader.setMat4("u_View", view);
        m_basicShader.setMat4("u_Projection", proj);
        m_basicShader.setInt("u_Texture", 0);

        if (m_paletteTexture.isLoaded()) m_paletteTexture.bind(0);

        // Circuito y props en el menu
        m_trackRenderer.render(m_trackGrid, m_basicShader);
        m_propSpawner.render(m_basicShader);

        // Kart inicial colocado en la pista
        m_basicShader.setBool("u_UseTexture", true);
        m_basicShader.setVec4("u_Color", glm::vec4(1.0f));
        m_kartEntity.draw(m_basicShader);
        return;
    }

    // -------------------------------------------------------------
    // RENDERIZADO: VEHICLE_SELECT (Showroom 3D con pedestal DE FRENTE)
    // -------------------------------------------------------------
    if (m_gameState == GameState::VEHICLE_SELECT) {
        // Fondo showroom azul marino suave
        m_window->clear(0.14f, 0.18f, 0.28f, 1.0f);
        if (!m_basicShader.isValid()) return;

        m_basicShader.use();

        // Camara enfocada al vehiculo de frente
        glm::vec3 camPos(0.0f, 1.35f, 3.4f);
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f, 0.40f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        m_basicShader.setMat4("u_View", view);
        m_basicShader.setMat4("u_Projection", proj);
        m_basicShader.setInt("u_Texture", 0);

        if (m_paletteTexture.isLoaded()) m_paletteTexture.bind(0);

        // 1. Dibujar el Pedestal bajo el kart
        m_basicShader.setBool("u_UseTexture", false);
        glm::mat4 pedMat = glm::rotate(glm::mat4(1.0f), glm::radians(m_showcaseAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        m_basicShader.setMat4("u_Model", pedMat);
        m_basicShader.setVec4("u_Color", glm::vec4(0.20f, 0.22f, 0.28f, 1.0f)); // Plataforma metalica
        m_pedestalMesh.draw();

        // 2. Dibujar el Kart y Piloto DE FRENTE con sus texturas
        m_basicShader.setBool("u_UseTexture", true);
        m_basicShader.setVec4("u_Color", glm::vec4(1.0f));
        m_kartEntity.draw(m_basicShader);
        return;
    }

    // -------------------------------------------------------------
    // RENDERIZADO: RACING (Modo Carrera activo)
    // -------------------------------------------------------------
    m_window->clear(0.42f, 0.70f, 0.95f, 1.0f);
    if (!m_basicShader.isValid()) return;

    m_basicShader.use();

    glm::mat4 view = (m_cameraMode == CameraMode::CHASE) ? m_chaseCamera.getViewMatrix() : m_camera.getViewMatrix();
    glm::mat4 proj = (m_cameraMode == CameraMode::CHASE) ? m_chaseCamera.getProjectionMatrix(aspect) : m_camera.getProjectionMatrix(aspect);

    m_basicShader.setMat4("u_View", view);
    m_basicShader.setMat4("u_Projection", proj);

    m_basicShader.setInt("u_Texture", 0);
    m_basicShader.setBool("u_UseTexture", m_paletteTexture.isLoaded());

    if (m_paletteTexture.isLoaded()) {
        m_paletteTexture.bind(0);
    }

    // 1. Renderizar pista modular
    m_trackRenderer.render(m_trackGrid, m_basicShader);

    // 2. Renderizar props y obstaculos
    m_propSpawner.render(m_basicShader);

    // 3. Renderizar entidad jerarquica del kart
    m_basicShader.setVec4("u_Color", glm::vec4(1.0f));
    m_kartEntity.draw(m_basicShader);
}

void Application::run() {
    while (m_running && !m_window->shouldClose()) {
        m_time.update();

        // 1. Limpiar flags de teclas del fotograma anterior
        Input::update();

        // 2. Sondear eventos frescos del sistema operativo (GLFW dispara keyCallback aqui)
        m_window->pollEvents();

        // 3. Procesar las entradas capturadas en este fotograma
        processInput();

        // 4. Actualizar logica del juego
        update(m_time.getDeltaTime());

        // 5. Renderizar escena
        render();

        // 6. Presentar fotograma en pantalla
        m_window->swapBuffers();
    }
}

void Application::close() {
    m_running = false;
}

} // namespace core
