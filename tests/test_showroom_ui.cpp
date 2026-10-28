#include <iostream>
#include <cassert>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "ui/ShowroomUI.hpp"
#include "ui/UIRenderer.hpp"
#include "core/GameContext.hpp"
#include "core/Input.hpp"
#include "core/SceneManager.hpp"
#include "lighting/LightManager.hpp"
#include "lighting/LightingController.hpp"
#include "world/TrackEnvironmentRegistry.hpp"

namespace {
class DummyScene : public core::IScene {
public:
    void onEnter() override {}
    void onExit() override {}
    void handleInput(float) override {}
    void update(float) override {}
    void render(int, int) override {}
};
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Validacion de Interfaz Grafica ShowroomUI (2D/3D)  " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // 1. Inicialización de GLFW invisible y contexto OpenGL 3.3 Core
    // -------------------------------------------------------------------------
    if (!glfwInit()) {
        std::cerr << "Fallo al inicializar GLFW" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Test ShowroomUI", nullptr, nullptr);
    if (!window) {
        std::cerr << "Fallo al crear ventana GLFW" << std::endl;
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Fallo al cargar extensiones GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    core::Input::init(window);

    // Bloque delimitado para destruccion RAII de todos los recursos OpenGL antes de glfwTerminate
    {
        // -------------------------------------------------------------------------
        // 2. Inicialización de UIRenderer y Contexto de Juego con Catálogo Kenney
        // -------------------------------------------------------------------------
        ui::UIRenderer renderer;
        bool rendOk = renderer.init("shaders/ui.vert", "shaders/ui.frag");
        assert(rendOk && "UIRenderer::init() debe retornar true");
        (void)rendOk;

        lighting::LightManager lightManager;
        lightManager.init();
        lighting::LightingController lightingController(&lightManager);

        core::SceneManager sceneManager;
        sceneManager.registerScene(core::GameState::RACING, std::make_shared<DummyScene>());
        sceneManager.registerScene(core::GameState::MAIN_MENU, std::make_shared<DummyScene>());

        core::GameContext ctx;
        ctx.lightManager = &lightManager;
        ctx.lightingController = &lightingController;
        ctx.sceneManager = &sceneManager;
        ctx.cameraMode = core::CameraMode::CHASE;

        // Poblar catálogo de corredores, vehículos y circuitos oficiales
        ctx.drivers = world::TrackEnvironmentRegistry::createDefaultDrivers();
        ctx.vehicles = world::TrackEnvironmentRegistry::createDefaultVehicles();
        ctx.tracks = world::TrackEnvironmentRegistry::createDefaultTracks();

        assert(!ctx.drivers.empty() && "El catálogo de corredores debe tener elementos");
        assert(!ctx.vehicles.empty() && "El catálogo de vehículos debe tener elementos");
        assert(!ctx.tracks.empty() && "El catálogo de pistas debe tener elementos");

        ui::ShowroomUI ui;
        bool uiOk = ui.init(&ctx, &renderer);
        assert(uiOk && "ShowroomUI::init() debe retornar true");
        (void)uiOk;
        assert(ui.isInitialized() && "ShowroomUI debe marcarse como inicializado");
        assert(ui.areTexturesLoaded() && "Todas las texturas de Kenney UI Pack deben cargarse");
        assert(glGetError() == GL_NO_ERROR && "Sin errores de GL tras inicializar ShowroomUI");

        std::cout << "  [CHECK 1] Inicializacion de ShowroomUI con texturas Kenney OK." << std::endl;

        // -------------------------------------------------------------------------
        // 3. Navegación de Categorías (Pestañas Superiores: 1. Piloto, 2. Auto, 3. Pista)
        // -------------------------------------------------------------------------
        // Categoría inicial: DRIVER
        assert(ui.getActiveCategory() == core::SelectionCategory::DRIVER);

        // Tecla 2 -> VEHICLE
        core::Input::simulateKey(GLFW_KEY_2, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(ui.getActiveCategory() == core::SelectionCategory::VEHICLE);

        // Tecla 3 -> TRACK
        core::Input::simulateKey(GLFW_KEY_3, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(ui.getActiveCategory() == core::SelectionCategory::TRACK);

        // Tecla 1 -> DRIVER
        core::Input::simulateKey(GLFW_KEY_1, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(ui.getActiveCategory() == core::SelectionCategory::DRIVER);

        // TAB -> VEHICLE -> TRACK -> DRIVER (Ciclo circular)
        core::Input::simulateKey(GLFW_KEY_TAB, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(ui.getActiveCategory() == core::SelectionCategory::VEHICLE);

        core::Input::simulateKey(GLFW_KEY_TAB, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(ui.getActiveCategory() == core::SelectionCategory::TRACK);

        core::Input::simulateKey(GLFW_KEY_TAB, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(ui.getActiveCategory() == core::SelectionCategory::DRIVER);

        // Flecha ARRIBA / W -> Ciclo inverso (DRIVER -> TRACK)
        core::Input::simulateKey(GLFW_KEY_UP, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(ui.getActiveCategory() == core::SelectionCategory::TRACK);

        ui.setActiveCategory(core::SelectionCategory::DRIVER);
        assert(ui.getActiveCategory() == core::SelectionCategory::DRIVER);
        std::cout << "  [CHECK 2] Navegacion de categorias y pestanas superiores (1, 2, 3, TAB, UP/DOWN) OK." << std::endl;

        // -------------------------------------------------------------------------
        // 4. Navegación del Carrusel (A/D, Flechas Izq/Der con Wrap-around)
        // -------------------------------------------------------------------------
        // En Categoría DRIVER
        ui.setActiveCategory(core::SelectionCategory::DRIVER);
        int initialDriverIdx = ui.getSelectedDriverIndex();
        assert(initialDriverIdx == 0);
        (void)initialDriverIdx;

        // Flecha Derecha (D) -> Siguiente piloto
        core::Input::simulateKey(GLFW_KEY_D, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(ui.getSelectedDriverIndex() == 1);

        // Flecha Izquierda (A) -> Piloto anterior
        core::Input::simulateKey(GLFW_KEY_A, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(ui.getSelectedDriverIndex() == 0);

        // Flecha Izquierda de nuevo -> Wrap-around al último piloto
        core::Input::simulateKey(GLFW_KEY_LEFT, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(ui.getSelectedDriverIndex() == static_cast<int>(ctx.drivers.size()) - 1);

        // En Categoría VEHICLE
        ui.setActiveCategory(core::SelectionCategory::VEHICLE);
        ui.setVehicleIndex(0);
        assert(ui.getSelectedVehicleIndex() == 0);

        core::Input::simulateKey(GLFW_KEY_RIGHT, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(ui.getSelectedVehicleIndex() == 1);

        ui.selectPrevious();
        assert(ui.getSelectedVehicleIndex() == 0);

        ui.selectPrevious();
        assert(ui.getSelectedVehicleIndex() == static_cast<int>(ctx.vehicles.size()) - 1);

        // En Categoría TRACK
        ui.setActiveCategory(core::SelectionCategory::TRACK);
        ui.setTrackIndex(0);
        assert(ui.getSelectedTrackIndex() == 0);

        ui.selectNext();
        assert(ui.getSelectedTrackIndex() == 1);

        ui.selectPrevious();
        assert(ui.getSelectedTrackIndex() == 0);

        ui.selectPrevious();
        assert(ui.getSelectedTrackIndex() == static_cast<int>(ctx.tracks.size()) - 1);

        std::cout << "  [CHECK 3] Selector de carrusel y navegacion A/D / Flechas con envoltura circular OK." << std::endl;

        // -------------------------------------------------------------------------
        // 5. Validación de Métricas y Especificaciones de Mario Kart
        // -------------------------------------------------------------------------
        // Verificar estadísticas de vehículos
        const auto& v0 = ctx.vehicles[0];
        assert(v0.maxSpeed > 0.0f);
        assert(v0.acceleration > 0.0f);
        assert(v0.turnSpeed > 0.0f);
        assert(v0.offRoadFriction > 0.0f && v0.offRoadFriction <= 1.0f);
        (void)v0;

        // Verificar datos de pistas
        const auto& t0 = ctx.tracks[0];
        assert(!t0.name.empty());
        assert(!t0.difficulty.empty());
        assert(t0.cols > 0 && t0.rows > 0);
        (void)t0;

        // Verificar color de acento de corredores
        const auto& d0 = ctx.drivers[0];
        assert(d0.accentColor.a > 0.0f);
        (void)d0;
        std::cout << "  [CHECK 4] Consistencia de metricas tecnicas, biomas y especificaciones OK." << std::endl;

        // -------------------------------------------------------------------------
        // 6. Pase de Renderizado 2D completo (Pestañas, Tarjetas de 3 Categorías, Carrusel)
        // -------------------------------------------------------------------------
        // Render en categoría DRIVER
        ui.setActiveCategory(core::SelectionCategory::DRIVER);
        ui.update(0.016f);
        ui.render(1280.0f, 720.0f);
        assert(glGetError() == GL_NO_ERROR && "Renderizado de ShowroomUI (Piloto) sin errores GL");

        // Render en categoría VEHICLE (incluye las 4 barras progresivas segmentadas)
        ui.setActiveCategory(core::SelectionCategory::VEHICLE);
        ui.update(0.016f);
        ui.render(1280.0f, 720.0f);
        assert(glGetError() == GL_NO_ERROR && "Renderizado de ShowroomUI (Vehículo y 4 barras) sin errores GL");

        // Render en categoría TRACK (incluye estrellas de dificultad y ficha de bioma)
        ui.setActiveCategory(core::SelectionCategory::TRACK);
        ui.update(0.016f);
        ui.render(1280.0f, 720.0f);
        assert(glGetError() == GL_NO_ERROR && "Renderizado de ShowroomUI (Pista y Estrellas) sin errores GL");

        std::cout << "  [CHECK 5] Pases de dibujo 2D (Tabs, Barras 3-slice, Estrellas, Carrusel) sin errores OpenGL OK." << std::endl;

        // -------------------------------------------------------------------------
        // 7. Acciones de Transición de Escena (ENTER -> RACING, ESC -> MAIN_MENU)
        // -------------------------------------------------------------------------
        // ENTER confirma e inicia carrera
        core::Input::simulateKey(GLFW_KEY_ENTER, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(sceneManager.getCurrentState() == core::GameState::RACING);

        // ESC vuelve al menú principal
        core::Input::simulateKey(GLFW_KEY_ESCAPE, GLFW_PRESS);
        ui.handleInput(0.016f);
        core::Input::update();
        assert(sceneManager.getCurrentState() == core::GameState::MAIN_MENU);

        std::cout << "  [CHECK 6] Disparadores de transicion [ENTER]->RACING y [ESC]->MAIN_MENU OK." << std::endl;

        // -------------------------------------------------------------------------
        // 8. Liberación y Limpieza RAII
        // -------------------------------------------------------------------------
        ui.destroy();
        assert(!ui.isInitialized() && "ShowroomUI::destroy() debe invalidar el estado");
        renderer.destroy();
        std::cout << "  [CHECK 7] Destruccion y limpieza RAII de recursos GPU OK." << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Todos los tests de ShowroomUI completados 100%!  " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
