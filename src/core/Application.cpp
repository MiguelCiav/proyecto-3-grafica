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
    const float radius = 1.35f;
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

void Application::loadKartProfiles() {
    m_kartProfiles = {
        {
            "kart-oobi",
            "Oobi (Verde Clasico)",
            "Equilibrado | Ideal para todo tipo de pistas",
            "assets/models/karts/kart-oobi.obj",
            22.0f, 18.0f, 100.0f, 0.45f,
            glm::vec4(0.2f, 0.85f, 0.3f, 1.0f)
        },
        {
            "kart-oodi",
            "Oodi (Azul Turbo)",
            "Alta Velocidad Punta | Bólido aerodinámico",
            "assets/models/karts/kart-oodi.obj",
            25.5f, 16.0f, 85.0f, 0.38f,
            glm::vec4(0.2f, 0.5f, 0.95f, 1.0f)
        },
        {
            "kart-ooli",
            "Ooli (Naranja Rally)",
            "Tracción Total | Máximo agarre en césped y curvas",
            "assets/models/karts/kart-ooli.obj",
            21.0f, 20.0f, 105.0f, 0.65f,
            glm::vec4(0.95f, 0.55f, 0.15f, 1.0f)
        },
        {
            "kart-oopi",
            "Oopi (Rojo Furia)",
            "Aceleración Explosiva | Salida rápida y turbo potente",
            "assets/models/karts/kart-oopi.obj",
            23.5f, 22.0f, 95.0f, 0.40f,
            glm::vec4(0.9f, 0.2f, 0.2f, 1.0f)
        },
        {
            "kart-oozi",
            "Oozi (Negro Nocturno)",
            "Derrape Ultra Ágil | Máxima maniobrabilidad técnica",
            "assets/models/karts/kart-oozi.obj",
            22.5f, 19.0f, 120.0f, 0.44f,
            glm::vec4(0.25f, 0.25f, 0.3f, 1.0f)
        }
    };

    // Preprocesar y cargar las 5 geometrias con pivotes en (0,0,0) a la memoria
    std::cout << "\n========================================================" << std::endl;
    std::cout << "  Cargando Catálogo de Vehículos para el Showroom...    " << std::endl;
    std::cout << "========================================================" << std::endl;

    for (size_t i = 0; i < m_kartProfiles.size(); ++i) {
        auto& profile = m_kartProfiles[i];
        if (geometry::KartPreprocessor::processKart(profile.filePath, profile.assembly)) {
            profile.loaded = true;
            std::cout << "  [" << (i + 1) << "/5] " << profile.name << " -> [CARGADO]" << std::endl;
        } else {
            std::cerr << "  [ERROR] Fallo al cargar: " << profile.filePath << std::endl;
        }
    }
    std::cout << "========================================================\n" << std::endl;
}

void Application::selectKart(int index) {
    if (m_kartProfiles.empty()) return;

    if (index < 0) index = static_cast<int>(m_kartProfiles.size()) - 1;
    if (index >= static_cast<int>(m_kartProfiles.size())) index = 0;

    m_selectedKartIndex = index;
    const auto& profile = m_kartProfiles[m_selectedKartIndex];

    if (profile.loaded) {
        m_kartEntity.setup(profile.assembly);

        // Aplicar personalidad física del vehículo
        m_kartPhysics.m_maxSpeed = profile.maxSpeed;
        m_kartPhysics.m_acceleration = profile.acceleration;
        m_kartPhysics.m_turnSpeed = profile.turnSpeed;

        std::cout << "\n>>> [SHOWROOM] Vehículo seleccionado: " << profile.name << " <<<" << std::endl;
        std::cout << "    Características: " << profile.subtitle << std::endl;
        std::cout << "    Velocidad Punta: " << (profile.maxSpeed * 3.6f) << " km/h"
                  << " | Aceleración: " << profile.acceleration
                  << " | Maniobrabilidad: " << profile.turnSpeed << "\n" << std::endl;
    }
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
        std::cout << "\n>>> [CARRERA INICIADA] Conduce con W/S/A/D | [M: Cambiar Kart] [ESC: Menú] <<<\n" << std::endl;
    } else if (m_gameState == GameState::VEHICLE_SELECT) {
        m_cursorLocked = false;
        Input::setCursorVisible(true);
        selectKart(m_selectedKartIndex);
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

    // 2. Registrar limites perimetrales exteriores del circuito para evitar caidas al vacio
    float halfSize = 25.0f;
    float wallHeight = 6.0f;
    m_collisionSystem.addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(-halfSize, 0.0f, halfSize),
        glm::vec3(halfSize, wallHeight, halfSize + 2.0f)
    ));
    m_collisionSystem.addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(-halfSize, 0.0f, -halfSize - 2.0f),
        glm::vec3(halfSize, wallHeight, -halfSize)
    ));
    m_collisionSystem.addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(halfSize, 0.0f, -halfSize),
        glm::vec3(halfSize + 2.0f, wallHeight, halfSize)
    ));
    m_collisionSystem.addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(-halfSize - 2.0f, 0.0f, -halfSize),
        glm::vec3(-halfSize, wallHeight, halfSize)
    ));

    std::cout << "[OK] [CollisionSystem] Registrados " << m_collisionSystem.getStaticBoxes().size()
              << " colisionadores estaticos en el escenario." << std::endl;
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

    // 2. Cargar circuito modular (12x12)
    world::LevelLoader::loadFromFile("assets/levels/main_track.txt", m_trackGrid);

    if (!m_trackRenderer.init("assets/models/track")) {
        std::cerr << "[ERROR] [Application] Fallo al inicializar TrackRenderer." << std::endl;
        return false;
    }

    if (!m_propSpawner.init("assets/models/obstacles")) {
        std::cerr << "[AVISO] [Application] Fallo al inicializar PropSpawner." << std::endl;
    }
    m_propSpawner.generateForGrid(m_trackGrid);
    m_zoneManager.autoGenerateFromGrid(m_trackGrid);

    // 3. Configurar el motor de colisiones
    setupColliders();

    // 4. Crear pedestal para el showroom 3D y cargar catálogo de karts
    createPedestalMesh();
    loadKartProfiles();

    // 5. Posicionar kart en casilla START_FINISH
    m_kartInitialPos = glm::vec3(0.0f, 0.0f, 0.0f);
    m_kartInitialYaw = 90.0f;

    for (const auto& tile : m_trackGrid.getAllTiles()) {
        if (tile.type == world::TileType::START_FINISH) {
            m_kartInitialPos = tile.worldPos;
            m_kartInitialYaw = tile.rotationDegrees;
            break;
        }
    }

    // Seleccionar el primer kart por defecto
    selectKart(0);

    // Iniciar en el Menu Principal con opciones
    setGameState(GameState::MAIN_MENU);

    std::cout << "\n========================================================" << std::endl;
    std::cout << "  PROTOTIPO MARIO KART - CGI PROYECTO #3 (UCV)          " << std::endl;
    std::cout << "  [MENU DEL JUEGO Y SHOWROOM ACTIVOS]:                  " << std::endl;
    std::cout << "    [1] o ENTER : Seleccionar Vehículo y Piloto         " << std::endl;
    std::cout << "    [2] o ESPACIO: Iniciar Carrera Inmediata             " << std::endl;
    std::cout << "    [ESC]        : Salir                                " << std::endl;
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
            close();
            return;
        }
        if (Input::isKeyJustPressed(GLFW_KEY_1) || Input::isKeyJustPressed(GLFW_KEY_ENTER)) {
            setGameState(GameState::VEHICLE_SELECT);
            return;
        }
        if (Input::isKeyJustPressed(GLFW_KEY_2) || Input::isKeyJustPressed(GLFW_KEY_SPACE)) {
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
            setGameState(GameState::MAIN_MENU);
            return;
        }
        // Navegacion entre karts: A / D o Flechas Izquierda / Derecha
        if (Input::isKeyJustPressed(GLFW_KEY_A) || Input::isKeyJustPressed(GLFW_KEY_LEFT)) {
            selectKart(m_selectedKartIndex - 1);
        }
        if (Input::isKeyJustPressed(GLFW_KEY_D) || Input::isKeyJustPressed(GLFW_KEY_RIGHT)) {
            selectKart(m_selectedKartIndex + 1);
        }
        // Confirmar seleccion y comenzar la carrera
        if (Input::isKeyJustPressed(GLFW_KEY_ENTER) || Input::isKeyJustPressed(GLFW_KEY_SPACE)) {
            setGameState(GameState::RACING);
        }
        return;
    }

    // -------------------------------------------------------------
    // Control en RACING (Carrera activa)
    // -------------------------------------------------------------
    if (m_gameState == GameState::RACING) {
        // Tecla M: Volver al Showroom a cambiar de auto
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
            std::cout << "[Kart] Posicion reiniciada a la linea de salida." << std::endl;
        }

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
            ss << m_config.title << " | [MENU PRINCIPAL] [1 o ENTER: Seleccionar Vehiculo] [2 o ESPACIO: Correr] [ESC: Salir]";
            m_window->setTitle(ss.str());
        }
        return;
    }

    // -------------------------------------------------------------
    // Actualizacion en VEHICLE_SELECT: Rotacion en Showroom
    // -------------------------------------------------------------
    if (m_gameState == GameState::VEHICLE_SELECT) {
        m_showcaseAngle += deltaTime * 35.0f;
        if (m_showcaseAngle > 360.0f) m_showcaseAngle -= 360.0f;

        // Rotar el kart en el pedestal del showroom
        m_wheelAnimator.update(deltaTime, 0.0f, 0.0f);
        m_kartEntity.update(glm::vec3(0.0f, 0.05f, 0.0f), m_showcaseAngle, m_wheelAnimator);

        m_titleUpdateTimer += deltaTime;
        if (m_titleUpdateTimer >= 0.20f) {
            m_titleUpdateTimer = 0.0f;
            const auto& p = m_kartProfiles[m_selectedKartIndex];
            std::stringstream ss;
            ss << m_config.title << " | SHOWROOM [" << (m_selectedKartIndex + 1) << "/5]: < " << p.name << " > "
               << "| Vel: " << static_cast<int>(p.maxSpeed * 3.6f) << " km/h "
               << "| [A/D: Cambiar] [ENTER: Confirmar y Correr] [ESC: Menu]";
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
        m_kartPhysics.setSurfaceFriction(m_kartProfiles[m_selectedKartIndex].offRoadFriction);
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
            std::cout << ">>> [META!] Vuelta " << m_lapsCompleted << " completada con "
                      << m_kartProfiles[m_selectedKartIndex].name << "! <<<" << std::endl;
        }
    }

    // 6. Actualizar cinemática de ruedas y Grafo de Escena
    m_wheelAnimator.update(deltaTime, m_kartPhysics.getSpeed(), steer);
    m_kartEntity.update(m_kartPhysics.getPosition(), m_kartPhysics.getYaw(), m_wheelAnimator);

    // 7. Actualizar camaras
    m_chaseCamera.update(deltaTime, m_kartPhysics.getPosition(), m_kartPhysics.getForwardVector());
    if (m_cameraMode == CameraMode::FPS) {
        m_camera.resolveCollision(m_collisionSystem);
    }

    // 8. Titulo con telemetria en tiempo real
    m_titleUpdateTimer += deltaTime;
    if (m_titleUpdateTimer >= 0.20f) {
        m_titleUpdateTimer = 0.0f;
        std::stringstream ss;
        ss << m_config.title << " | " << static_cast<int>(m_time.getFPS()) << " FPS | ";
        if (m_cameraMode == CameraMode::CHASE) {
            ss << "[" << m_kartProfiles[m_selectedKartIndex].name << "] ";
            ss << "| Vel: " << std::fixed << std::setprecision(1) << (m_kartPhysics.getSpeed() * 3.6f) << " km/h";
            if (m_kartPhysics.isBoosting()) ss << " [TURBO BOOST!]";
            if (m_kartPhysics.getSurfaceFriction() < 0.9f) ss << " [OFF-ROAD]";
            ss << " | Vueltas: " << m_lapsCompleted;
        } else {
            ss << "[Modo Inspeccion FPS (Colisiones ON)] | Pos: ("
               << std::setprecision(1) << m_camera.getPosition().x << ", "
               << m_camera.getPosition().y << ", "
               << m_camera.getPosition().z << ")";
        }
        ss << " | [M: Cambiar Kart] [C: Camara] [R: Reset] [ESC: Menu]";
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
        float orbitDist = 28.0f;
        glm::vec3 camPos(std::sin(rad) * orbitDist, 14.0f, std::cos(rad) * orbitDist);
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
    // RENDERIZADO: VEHICLE_SELECT (Showroom 3D con pedestal)
    // -------------------------------------------------------------
    if (m_gameState == GameState::VEHICLE_SELECT) {
        // Fondo showroom azul marino degradado suave
        m_window->clear(0.14f, 0.18f, 0.28f, 1.0f);
        if (!m_basicShader.isValid()) return;

        m_basicShader.use();

        // Camara enfocada al vehiculo en exhibicion
        glm::vec3 camPos(0.0f, 1.35f, 3.4f);
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f, 0.40f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        m_basicShader.setMat4("u_View", view);
        m_basicShader.setMat4("u_Projection", proj);
        m_basicShader.setInt("u_Texture", 0);

        if (m_paletteTexture.isLoaded()) m_paletteTexture.bind(0);

        // 1. Dibujar el Pedestal giratorio bajo el kart
        m_basicShader.setBool("u_UseTexture", false);
        glm::mat4 pedMat = glm::rotate(glm::mat4(1.0f), glm::radians(m_showcaseAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        m_basicShader.setMat4("u_Model", pedMat);
        m_basicShader.setVec4("u_Color", glm::vec4(0.20f, 0.22f, 0.28f, 1.0f)); // Plataforma metalica
        m_pedestalMesh.draw();

        // 2. Dibujar el Kart girando con sus texturas
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

    // 3. Renderizar el Kart articulado jerarquicamente via Grafo de Escena
    m_basicShader.setBool("u_UseTexture", true);
    m_basicShader.setVec4("u_Color", glm::vec4(1.0f));
    m_kartEntity.draw(m_basicShader);
}

void Application::run() {
    while (m_running && !m_window->shouldClose()) {
        m_time.update();
        float dt = m_time.getDeltaTime();

        m_window->pollEvents();
        processInput();
        update(dt);
        render();
        m_window->swapBuffers();
        Input::update();
    }
}

void Application::close() {
    m_running = false;
    if (m_window) {
        m_window->setShouldClose(true);
    }
}

} // namespace core
