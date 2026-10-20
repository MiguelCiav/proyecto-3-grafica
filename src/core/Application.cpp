#include "core/Application.hpp"
#include "core/Input.hpp"
#include "world/LevelLoader.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace core {

static Application* s_instance = nullptr;

Application& Application::getInstance() {
    return *s_instance;
}

Application::Application(const AppConfig& config)
    : m_config(config)
    , m_camera(glm::vec3(0.0f, 4.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -15.0f)
{
    s_instance = this;
}

Application::~Application() {
    s_instance = nullptr;
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

    // Inicializar subsistema de entrada
    Input::init(m_window->getNativeWindow());

    // 1. Cargar Shaders Base
    if (!m_basicShader.loadFromFiles("shaders/basic.vert", "shaders/basic.frag")) {
        std::cerr << "[ERROR] [Application] Fallo al cargar shaders/basic" << std::endl;
        return false;
    }
    std::cout << "[OK] [Application] Shaders compilados exitosamente." << std::endl;

    // 2. Cargar Textura colormap.png
    if (!m_paletteTexture.loadFromFile("assets/textures/colormap.png")) {
        std::cerr << "[AVISO] [Application] No se pudo cargar colormap.png" << std::endl;
    }

    // 3. Inicializar y Cargar el Circuito Modular Completo (Fase 4 - Issue #16)
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

    // 4. Buscar posición de salida (START_FINISH) para posicionar el kart
    m_kartPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    m_kartRotationY = 90.0f; // Mirando hacia adelante en la recta de salida

    for (const auto& tile : m_trackGrid.getAllTiles()) {
        if (tile.type == world::TileType::START_FINISH) {
            m_kartPosition = tile.worldPos;
            m_kartRotationY = tile.rotationDegrees;
            break;
        }
    }

    // 5. Cargar y ensamblar el Kart (kart-oobi.obj) con pivotes en (0,0,0)
    geometry::KartAssembly kartOobi;
    if (geometry::KartPreprocessor::processKart("assets/models/karts/kart-oobi.obj", kartOobi)) {
        geometry::KartPreprocessor::validateAndReport(kartOobi, "kart-oobi (Verde)");

        glm::mat4 kartRoot = glm::translate(glm::mat4(1.0f), m_kartPosition);
        kartRoot = glm::rotate(kartRoot, glm::radians(m_kartRotationY), glm::vec3(0.0f, 1.0f, 0.0f));

        // Chasis
        RenderItem chasisItem;
        chasisItem.name = "Kart_Chassis";
        chasisItem.mesh.setup(kartOobi.chassis);
        chasisItem.transform = kartRoot;
        m_kartRenderItems.push_back(std::move(chasisItem));

        // Ruedas en sus pivotes relativos
        RenderItem flWheel;
        flWheel.name = "Kart_Wheel_FL";
        flWheel.mesh.setup(kartOobi.wheelFrontLeft);
        flWheel.transform = kartRoot * glm::translate(glm::mat4(1.0f), kartOobi.wheelFrontLeft.pivotOffset);
        m_kartRenderItems.push_back(std::move(flWheel));

        RenderItem frWheel;
        frWheel.name = "Kart_Wheel_FR";
        frWheel.mesh.setup(kartOobi.wheelFrontRight);
        frWheel.transform = kartRoot * glm::translate(glm::mat4(1.0f), kartOobi.wheelFrontRight.pivotOffset);
        m_kartRenderItems.push_back(std::move(frWheel));

        RenderItem blWheel;
        blWheel.name = "Kart_Wheel_BL";
        blWheel.mesh.setup(kartOobi.wheelBackLeft);
        blWheel.transform = kartRoot * glm::translate(glm::mat4(1.0f), kartOobi.wheelBackLeft.pivotOffset);
        m_kartRenderItems.push_back(std::move(blWheel));

        RenderItem brWheel;
        brWheel.name = "Kart_Wheel_BR";
        brWheel.mesh.setup(kartOobi.wheelBackRight);
        brWheel.transform = kartRoot * glm::translate(glm::mat4(1.0f), kartOobi.wheelBackRight.pivotOffset);
        m_kartRenderItems.push_back(std::move(brWheel));

        // Piloto
        if (!kartOobi.driver.vertices.empty()) {
            RenderItem driverItem;
            driverItem.name = "Kart_Driver";
            driverItem.mesh.setup(kartOobi.driver);
            driverItem.transform = kartRoot;
            m_kartRenderItems.push_back(std::move(driverItem));
        }
    }

    // 6. Configurar cámara FPS ubicada detrás del kart con vista panorámica del circuito
    glm::vec3 camPos = m_kartPosition + glm::vec3(-6.0f, 4.0f, 0.0f);
    m_camera.setPosition(camPos);
    m_camera.lookAtTarget(m_kartPosition + glm::vec3(10.0f, 1.0f, 0.0f));

    std::cout << "\n=====================================================" << std::endl;
    std::cout << "  Prototipo Mario Kart - CGI Proyecto #3 (UCV)      " << std::endl;
    std::cout << "  Fase 4 - Circuito Cerrado y Obstáculos Activos    " << std::endl;
    std::cout << "  [Resumen del Mundo 3D]:                           " << std::endl;
    std::cout << "    - Celdas de Pista: " << m_trackGrid.getCols() << "x" << m_trackGrid.getRows() << std::endl;
    std::cout << "    - Piezas de Asfalto: " << m_trackRenderer.getRenderedTileCount() << std::endl;
    std::cout << "    - Obstáculos / Barreras: " << m_propSpawner.getPropCount() << std::endl;
    std::cout << "    - Zonas Especiales (Meta/Turbo): " << m_zoneManager.getZones().size() << std::endl;
    std::cout << "  [Instrucciones de Control]:                       " << std::endl;
    std::cout << "    - Clic o TAB: Bloquear/Liberar cursor del mouse " << std::endl;
    std::cout << "    - WASD: Desplazar cámara libre FPS              " << std::endl;
    std::cout << "    - Espacio / Shift: Subir y Bajar                " << std::endl;
    std::cout << "    - Rueda del mouse: Zoom (FOV)                   " << std::endl;
    std::cout << "    - ESC: Liberar cursor / Salir                   " << std::endl;
    std::cout << "=====================================================\n" << std::endl;

    m_time.reset();
    m_running = true;
    return true;
}

void Application::run() {
    float titleTimer = 0.0f;

    while (m_running && !m_window->shouldClose()) {
        m_time.update();
        float dt = m_time.getDeltaTime();

        // 1. Manejo de eventos
        m_window->pollEvents();

        // 2. Procesar entradas y cámara
        processInput();

        // 3. Actualización de lógica
        update(dt);

        // 4. Renderizado 3D
        render();

        // 5. Intercambio de buffers
        m_window->swapBuffers();

        // 6. Limpieza de eventos de un solo frame
        Input::update();

        // Actualizar métricas en el título de la ventana
        titleTimer += dt;
        if (titleTimer >= 0.25f) {
            std::stringstream ss;
            ss << m_config.title << " | FPS: " << std::fixed << std::setprecision(1) << m_time.getFPS()
               << " (" << std::setprecision(2) << m_time.getFrameTimeMs() << " ms)"
               << (m_cursorLocked ? " [CAMARA FPS ACTIVA]" : " [CURSOR LIBRE - Clic para capturar]");
            m_window->setTitle(ss.str());
            titleTimer = 0.0f;
        }
    }

    std::cout << "[OK] [Application] Bucle principal finalizado limpiamente." << std::endl;
}

void Application::close() {
    m_running = false;
    if (m_window) {
        m_window->setShouldClose(true);
    }
}

void Application::processInput() {
    // ESC: Liberar cursor si está capturado, o cerrar si ya está libre
    if (Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
        if (m_cursorLocked) {
            m_cursorLocked = false;
            Input::setCursorVisible(true);
        } else {
            close();
            return;
        }
    }

    // TAB o Clic Izquierdo en ventana: alternar captura de cursor para control FPS
    if (Input::isKeyJustPressed(GLFW_KEY_TAB) || (!m_cursorLocked && Input::isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT))) {
        m_cursorLocked = !m_cursorLocked;
        Input::setCursorVisible(!m_cursorLocked);
    }

    // Controles de Cámara Libre FPS
    if (m_cursorLocked) {
        float dt = m_time.getDeltaTime();

        if (Input::isKeyPressed(GLFW_KEY_W)) m_camera.processKeyboard(camera::CameraMovement::FORWARD, dt);
        if (Input::isKeyPressed(GLFW_KEY_S)) m_camera.processKeyboard(camera::CameraMovement::BACKWARD, dt);
        if (Input::isKeyPressed(GLFW_KEY_A)) m_camera.processKeyboard(camera::CameraMovement::LEFT, dt);
        if (Input::isKeyPressed(GLFW_KEY_D)) m_camera.processKeyboard(camera::CameraMovement::RIGHT, dt);
        if (Input::isKeyPressed(GLFW_KEY_SPACE) || Input::isKeyPressed(GLFW_KEY_E))
            m_camera.processKeyboard(camera::CameraMovement::UP, dt);
        if (Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT) || Input::isKeyPressed(GLFW_KEY_Q))
            m_camera.processKeyboard(camera::CameraMovement::DOWN, dt);

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

void Application::update(float /*deltaTime*/) {
    // Espacio para futuras físicas y animaciones de ruedas
}

void Application::render() {
    // Habilitar test de profundidad
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Limpiar color y profundidad (Cielo azul crepúsculo)
    m_window->clear(0.12f, 0.14f, 0.20f, 1.0f);

    if (!m_basicShader.isValid()) return;

    m_basicShader.use();

    float aspect = m_window->getAspectRatio();
    m_basicShader.setMat4("u_Projection", m_camera.getProjectionMatrix(aspect));
    m_basicShader.setMat4("u_View", m_camera.getViewMatrix());

    m_basicShader.setInt("u_Texture", 0);
    m_basicShader.setBool("u_UseTexture", m_paletteTexture.isLoaded());

    if (m_paletteTexture.isLoaded()) {
        m_paletteTexture.bind(0);
    }

    // 1. Dibujar el circuito modular completo y terreno verde
    m_trackRenderer.render(m_trackGrid, m_basicShader);

    // 2. Dibujar las barreras y conos perimetrales
    m_propSpawner.render(m_basicShader);

    // 3. Dibujar el Kart articulado con sus 4 ruedas en pivotes
    m_basicShader.setVec4("u_Color", glm::vec4(1.0f));
    for (const auto& item : m_kartRenderItems) {
        m_basicShader.setMat4("u_Model", item.transform);
        item.mesh.draw();
    }
}

} // namespace core
