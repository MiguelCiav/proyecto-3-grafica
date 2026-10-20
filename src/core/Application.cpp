#include "geometry/KartPreprocessor.hpp"
#include "core/Application.hpp"
#include "core/Input.hpp"
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

    m_time.reset();
    m_running = true;

    std::cout << "=====================================================" << std::endl;
    std::cout << "  Prototipo Mario Kart - CGI Proyecto #3 (UCV)      " << std::endl;
    std::cout << "  Fase 1 - Game Loop y Pipeline Inicial Activo      " << std::endl;
    // Validar Pipeline de Preprocesamiento de Karts (Fase 2 - Issue #8)
    geometry::KartAssembly kartOobi;
    if (geometry::KartPreprocessor::processKart("assets/models/karts/kart-oobi.obj", kartOobi)) {
        geometry::KartPreprocessor::validateAndReport(kartOobi, "kart-oobi (Verde)");
    }

    geometry::KartAssembly kartOodi;
    if (geometry::KartPreprocessor::processKart("assets/models/karts/kart-oodi.obj", kartOodi)) {
        geometry::KartPreprocessor::validateAndReport(kartOodi, "kart-oodi (Azul)");
    }

    std::cout << "=====================================================" << std::endl;

    return true;
}

void Application::run() {
    float titleTimer = 0.0f;

    while (m_running && !m_window->shouldClose()) {
        m_time.update();
        float dt = m_time.getDeltaTime();

        // 1. Manejo de eventos
        m_window->pollEvents();

        // 2. Procesar entradas
        processInput();

        // 3. Actualización de lógica
        update(dt);

        // 4. Renderizado
        render();

        // 5. Intercambio de buffers
        m_window->swapBuffers();

        // 6. Limpieza de entradas de un solo frame
        Input::update();

        // Actualizar métricas en el título de la ventana cada 0.25s
        titleTimer += dt;
        if (titleTimer >= 0.25f) {
            std::stringstream ss;
            ss << m_config.title << " | FPS: " << std::fixed << std::setprecision(1) << m_time.getFPS()
               << " (" << std::setprecision(2) << m_time.getFrameTimeMs() << " ms)";
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
    if (Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
        close();
    }
}

void Application::update(float /*deltaTime*/) {
    // Lógica futura de escena, kart y física
}

void Application::render() {
    // Color de asfalto / noche inicial: #1A1A24
    m_window->clear(0.10f, 0.10f, 0.14f, 1.0f);
}

} // namespace core
