#include "core/Application.hpp"
#include "core/Input.hpp"
#include "world/LevelLoader.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace core {

static Application* s_instance = nullptr;

Application& Application::getInstance() {
    return *s_instance;
}

Application::Application(const AppConfig& config)
    : m_config(config),
      m_camera(glm::vec3(0.0f, 4.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -15.0f),
      m_chaseCamera(5.2f, 2.3f, 1.2f) {
    s_instance = this;
}

Application::~Application() {
    s_instance = nullptr;
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
    // Norte (+Z)
    m_collisionSystem.addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(-halfSize, 0.0f, halfSize),
        glm::vec3(halfSize, wallHeight, halfSize + 2.0f)
    ));
    // Sur (-Z)
    m_collisionSystem.addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(-halfSize, 0.0f, -halfSize - 2.0f),
        glm::vec3(halfSize, wallHeight, -halfSize)
    ));
    // Este (+X)
    m_collisionSystem.addStaticBox(Engine::Physics::BoundingBox(
        glm::vec3(halfSize, 0.0f, -halfSize),
        glm::vec3(halfSize + 2.0f, wallHeight, halfSize)
    ));
    // Oeste (-X)
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

    // 4. Cargar y ensamblar el Kart (kart-oobi.obj) con pivotes recentrados
    geometry::KartAssembly kartOobi;
    if (geometry::KartPreprocessor::processKart("assets/models/kart-oobi.obj", kartOobi)) {
        geometry::KartPreprocessor::validateAndReport(kartOobi, "kart-oobi (Verde)");

        m_chassisMesh.setup(kartOobi.chassis);
        m_wheelFLMesh.setup(kartOobi.wheelFrontLeft);
        m_wheelFRMesh.setup(kartOobi.wheelFrontRight);
        m_wheelBLMesh.setup(kartOobi.wheelBackLeft);
        m_wheelBRMesh.setup(kartOobi.wheelBackRight);

        m_wheelFLOffset = kartOobi.wheelFrontLeft.pivotOffset;
        m_wheelFROffset = kartOobi.wheelFrontRight.pivotOffset;
        m_wheelBLOffset = kartOobi.wheelBackLeft.pivotOffset;
        m_wheelBROffset = kartOobi.wheelBackRight.pivotOffset;

        if (!kartOobi.driver.vertices.empty()) {
            m_driverMesh.setup(kartOobi.driver);
        }
    }

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

    m_kartPhysics.setPosition(m_kartInitialPos);
    m_kartPhysics.setYaw(m_kartInitialYaw);

    // Inicializar camara Chase
    m_chaseCamera.reset(m_kartPhysics.getPosition(), m_kartPhysics.getForwardVector());

    // Inicializar camara FPS panoramica detras del kart
    glm::vec3 camPos = m_kartInitialPos + glm::vec3(-6.0f, 4.0f, 0.0f);
    m_camera.setPosition(camPos);
    m_camera.lookAtTarget(m_kartInitialPos + glm::vec3(6.0f, 1.0f, 0.0f));

    std::cout << "\n=====================================================" << std::endl;
    std::cout << "  Prototipo Mario Kart - CGI Proyecto #3 (UCV)      " << std::endl;
    std::cout << "  Fase 5 - Fisicas Arcade y Colisiones Integradas    " << std::endl;
    std::cout << "  [Controles]:                                      " << std::endl;
    std::cout << "    W / S             : Acelerar / Frenar y Reversa " << std::endl;
    std::cout << "    A / D             : Girar volante               " << std::endl;
    std::cout << "    C                 : Conmutar Camara (Chase/FPS) " << std::endl;
    std::cout << "    R                 : Reiniciar Kart a la Salida  " << std::endl;
    std::cout << "    ESC               : Liberar cursor / Salir      " << std::endl;
    std::cout << "=====================================================\n" << std::endl;

    m_time.reset();
    m_running = true;
    return true;
}

void Application::processInput() {
    // ESC: Liberar cursor si esta capturado en modo FPS, o salir si ya esta libre
    if (Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
        if (m_cameraMode == CameraMode::FPS && m_cursorLocked) {
            m_cursorLocked = false;
            Input::setCursorVisible(true);
        } else {
            close();
            return;
        }
    }

    // Conmutador de modo de camara (Tecla C)
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

void Application::update(float deltaTime) {
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
        m_kartPhysics.setSurfaceFriction(0.42f); // Desaceleracion fuera de pista
    } else {
        m_kartPhysics.setSurfaceFriction(1.0f);  // Adherencia normal
    }

    // 3. Actualizar modelo de dinamica de vehiculo
    m_kartPhysics.update(deltaTime, accel, brake, steer);

    // 4. Resolucion de colisiones del kart contra barreras/conos
    glm::vec3 mtv(0.0f);
    if (m_collisionSystem.checkCollision(m_kartPhysics.getBoundingBox(), mtv)) {
        m_kartPhysics.onCollision(mtv, 0.3f);
    }

    // 5. Evaluacion de zonas de boost y meta
    const world::SpecialZone* hitZone = m_zoneManager.checkTrigger(m_kartPhysics.getPosition());
    if (hitZone != nullptr) {
        if (hitZone->type == world::ZoneType::BOOST) {
            m_kartPhysics.applyBoost(hitZone->durationSeconds);
        } else if (hitZone->type == world::ZoneType::FINISH_LINE && m_finishLineCooldown <= 0.0f) {
            m_lapsCompleted++;
            m_finishLineCooldown = 5.0f;
            std::cout << ">>> [META!] Vuelta " << m_lapsCompleted << " completada! <<<" << std::endl;
        }
    }

    // 6. Animacion de rotacion de ruedas (roll en eje X segun distancia recorrida)
    float wheelRadius = 0.22f;
    float deltaRoll = glm::degrees((m_kartPhysics.getSpeed() * deltaTime) / wheelRadius);
    m_wheelRollAngle += deltaRoll;
    if (m_wheelRollAngle > 360.0f) m_wheelRollAngle -= 360.0f;
    if (m_wheelRollAngle < -360.0f) m_wheelRollAngle += 360.0f;

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
            ss << "[Modo Conduccion 3ra Persona] ";
            ss << "| Vel: " << std::fixed << std::setprecision(1) << (m_kartPhysics.getSpeed() * 3.6f) << " km/h";
            if (m_kartPhysics.isBoosting()) ss << " [TURBO BOOST!]";
            if (m_kartPhysics.getSurfaceFriction() < 0.9f) ss << " [CESPED/OFF-ROAD]";
            ss << " | Vueltas: " << m_lapsCompleted;
        } else {
            ss << "[Modo Inspeccion FPS (Colisiones ON)] | Pos: ("
               << std::setprecision(1) << m_camera.getPosition().x << ", "
               << m_camera.getPosition().y << ", "
               << m_camera.getPosition().z << ")";
        }
        ss << " | [C: Camara] [R: Reset]";
        m_window->setTitle(ss.str());
    }
}

void Application::render() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    m_window->clear(0.12f, 0.14f, 0.20f, 1.0f);

    if (!m_basicShader.isValid()) return;

    m_basicShader.use();

    float aspect = m_window->getAspectRatio();
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

    // 3. Renderizar el Kart articulado jerarquicamente
    glm::mat4 kartRoot = glm::translate(glm::mat4(1.0f), m_kartPhysics.getPosition());
    kartRoot = glm::rotate(kartRoot, glm::radians(m_kartPhysics.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f));

    m_basicShader.setVec4("u_Color", glm::vec4(1.0f));

    // Chasis
    m_basicShader.setMat4("u_Model", kartRoot);
    m_chassisMesh.draw();

    // Piloto
    m_basicShader.setMat4("u_Model", kartRoot);
    m_driverMesh.draw();

    // Ruedas Delanteras (Rotacion en Y por direccion del volante + Roll en X por rodamiento)
    float steerAngle = m_kartPhysics.getSteeringAngle();

    // Rueda Delantera Izquierda (FL)
    glm::mat4 flTransform = kartRoot * glm::translate(glm::mat4(1.0f), m_wheelFLOffset);
    flTransform = glm::rotate(flTransform, glm::radians(steerAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    flTransform = glm::rotate(flTransform, glm::radians(m_wheelRollAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    m_basicShader.setMat4("u_Model", flTransform);
    m_wheelFLMesh.draw();

    // Rueda Delantera Derecha (FR)
    glm::mat4 frTransform = kartRoot * glm::translate(glm::mat4(1.0f), m_wheelFROffset);
    frTransform = glm::rotate(frTransform, glm::radians(steerAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    frTransform = glm::rotate(frTransform, glm::radians(m_wheelRollAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    m_basicShader.setMat4("u_Model", frTransform);
    m_wheelFRMesh.draw();

    // Rueda Trasera Izquierda (BL)
    glm::mat4 blTransform = kartRoot * glm::translate(glm::mat4(1.0f), m_wheelBLOffset);
    blTransform = glm::rotate(blTransform, glm::radians(m_wheelRollAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    m_basicShader.setMat4("u_Model", blTransform);
    m_wheelBLMesh.draw();

    // Rueda Trasera Derecha (BR)
    glm::mat4 brTransform = kartRoot * glm::translate(glm::mat4(1.0f), m_wheelBROffset);
    brTransform = glm::rotate(brTransform, glm::radians(m_wheelRollAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    m_basicShader.setMat4("u_Model", brTransform);
    m_wheelBRMesh.draw();
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
