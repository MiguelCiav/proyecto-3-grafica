#include <iostream>
#include <cassert>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "core/Input.hpp"
#include "core/SceneManager.hpp"
#include "core/GameContext.hpp"
#include "scenes/MainMenuScene.hpp"
#include "scenes/ShowroomScene.hpp"
#include "scenes/RacingScene.hpp"

// Funciones para invocar la simulación de teclado en tests
namespace core {
    void simulateKeyPress(int key) {
        Input::simulateKey(key, GLFW_PRESS);
    }
    void simulateKeyRelease(int key) {
        Input::simulateKey(key, GLFW_RELEASE);
    }
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Validacion de Entrada (Input) y Escenas (Scenes)   " << std::endl;
    std::cout << "==========================================================" << std::endl;

    if (!glfwInit()) {
        std::cerr << "[ERROR] Fallo inicializacion GLFW" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Test Scene and Input", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    core::Input::init(window);

    // 1. Validar ciclo de vida de Input (update antes de eventos)
    {
        core::Input::update(); // Limpia flags del fotograma previo
        assert(!core::Input::isKeyJustPressed(GLFW_KEY_ENTER));
        assert(!core::Input::isKeyPressed(GLFW_KEY_ENTER));

        // Simular pulsación de ENTER
        core::simulateKeyPress(GLFW_KEY_ENTER);
        assert(core::Input::isKeyJustPressed(GLFW_KEY_ENTER));
        assert(core::Input::isKeyPressed(GLFW_KEY_ENTER));

        // Siguiente fotograma: update() limpia isKeyJustPressed pero mantiene isKeyPressed
        core::Input::update();
        assert(!core::Input::isKeyJustPressed(GLFW_KEY_ENTER));
        assert(core::Input::isKeyPressed(GLFW_KEY_ENTER));

        // Soltar tecla
        core::simulateKeyRelease(GLFW_KEY_ENTER);
        assert(!core::Input::isKeyPressed(GLFW_KEY_ENTER));

        std::cout << "  [CHECK 1] Ciclo de vida de Input (isKeyPressed / isKeyJustPressed) verificado." << std::endl;
    }

    // 2. Validar SceneManager y transiciones por input
    {
        core::SceneManager sceneManager;
        core::GameContext ctx;
        ctx.sceneManager = &sceneManager;

        // Subsystems mínimos para evitar crash en callbacks
        world::TrackGrid grid;
        renderer::TrackRenderer renderer;
        world::PropSpawner spawner;
        world::ZoneTriggerManager zoneMgr;
        std::vector<world::BoosterPad> pads;
        Engine::Physics::CollisionSystem col;
        Engine::Physics::KartPhysics physics;
        vehicle::KartEntity entity;
        animation::WheelAnimator anim;
        lighting::LightManager lightMgr;
        lighting::LightingController lightCtrl(&lightMgr);
        camera::Camera fpsCam;
        camera::ChaseCamera chaseCam;

        ctx.trackGrid = &grid;
        ctx.trackRenderer = &renderer;
        ctx.propSpawner = &spawner;
        ctx.zoneManager = &zoneMgr;
        ctx.boosterPads = &pads;
        ctx.collisionSystem = &col;
        ctx.kartPhysics = &physics;
        ctx.kartEntity = &entity;
        ctx.wheelAnimator = &anim;
        ctx.lightManager = &lightMgr;
        ctx.lightingController = &lightCtrl;
        ctx.fpsCamera = &fpsCam;
        ctx.chaseCamera = &chaseCam;

        ctx.drivers = world::TrackEnvironmentRegistry::createDefaultDrivers();
        ctx.vehicles = world::TrackEnvironmentRegistry::createDefaultVehicles();
        ctx.tracks = world::TrackEnvironmentRegistry::createDefaultTracks();

        auto menuScene = std::make_shared<scenes::MainMenuScene>(ctx);
        auto showroomScene = std::make_shared<scenes::ShowroomScene>(ctx);
        auto racingScene = std::make_shared<scenes::RacingScene>(ctx);

        sceneManager.registerScene(core::GameState::MAIN_MENU, menuScene);
        sceneManager.registerScene(core::GameState::VEHICLE_SELECT, showroomScene);
        sceneManager.registerScene(core::GameState::RACING, racingScene);

        sceneManager.changeScene(core::GameState::MAIN_MENU);
        assert(sceneManager.getCurrentState() == core::GameState::MAIN_MENU);

        // Frame 1 en MainMenu: presionar ENTER para ir a Showroom
        core::Input::update();
        core::simulateKeyPress(GLFW_KEY_ENTER);
        sceneManager.handleInput(0.016f);
        core::simulateKeyRelease(GLFW_KEY_ENTER);

        assert(sceneManager.getCurrentState() == core::GameState::VEHICLE_SELECT);
        std::cout << "  [CHECK 2] Transicion de MAIN_MENU a VEHICLE_SELECT por ENTER verificada." << std::endl;

        // Frame 2 en Showroom: presionar TAB para alternar categoria
        assert(ctx.selectionCategory == core::SelectionCategory::DRIVER);
        core::Input::update();
        core::simulateKeyPress(GLFW_KEY_TAB);
        sceneManager.handleInput(0.016f);
        core::simulateKeyRelease(GLFW_KEY_TAB);

        assert(ctx.selectionCategory == core::SelectionCategory::VEHICLE);
        std::cout << "  [CHECK 3] Navegacion por categorias en VEHICLE_SELECT con TAB verificada." << std::endl;

        // Frame 3 en Showroom: presionar ESPACIO para iniciar carrera
        core::Input::update();
        core::simulateKeyPress(GLFW_KEY_SPACE);
        sceneManager.handleInput(0.016f);
        core::simulateKeyRelease(GLFW_KEY_SPACE);

        assert(sceneManager.getCurrentState() == core::GameState::RACING);
        std::cout << "  [CHECK 4] Transicion de VEHICLE_SELECT a RACING por ESPACIO verificada." << std::endl;

        // Frame 4 en Racing: presionar C para alternar camara CHASE -> FPS
        assert(ctx.cameraMode == core::CameraMode::CHASE);
        core::Input::update();
        core::simulateKeyPress(GLFW_KEY_C);
        sceneManager.handleInput(0.016f);
        core::simulateKeyRelease(GLFW_KEY_C);

        assert(ctx.cameraMode == core::CameraMode::FPS);
        std::cout << "  [CHECK 5] Alternancia de camara en carrera con tecla C verificada." << std::endl;

        // Frame 5 en Racing: presionar M para volver al Showroom
        core::Input::update();
        core::simulateKeyPress(GLFW_KEY_M);
        sceneManager.handleInput(0.016f);
        core::simulateKeyRelease(GLFW_KEY_M);

        assert(sceneManager.getCurrentState() == core::GameState::VEHICLE_SELECT);
        std::cout << "  [CHECK 6] Regreso a VEHICLE_SELECT desde carrera con tecla M verificado." << std::endl;

        // Frame 6 en Showroom: presionar ESC para volver al Menu Principal
        core::Input::update();
        core::simulateKeyPress(GLFW_KEY_ESCAPE);
        sceneManager.handleInput(0.016f);
        core::simulateKeyRelease(GLFW_KEY_ESCAPE);

        assert(sceneManager.getCurrentState() == core::GameState::MAIN_MENU);
        std::cout << "  [CHECK 7] Regreso a MAIN_MENU con tecla ESC verificado." << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Todos los tests de Input y Escenas 100% exitosos! " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
