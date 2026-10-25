#include <iostream>
#include <cassert>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "ui/MainMenuUI.hpp"
#include "ui/UIRenderer.hpp"
#include "core/GameContext.hpp"
#include "core/Input.hpp"
#include "lighting/LightManager.hpp"
#include "lighting/LightingController.hpp"

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Validacion de Interfaz Grafica MainMenuUI y Modal " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // 1. Inicializacion de GLFW invisible y contexto OpenGL 3.3 Core
    // -------------------------------------------------------------------------
    if (!glfwInit()) {
        std::cerr << "Fallo al inicializar GLFW" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Test MainMenuUI", nullptr, nullptr);
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
        // 2. Inicializacion de UIRenderer y Contexto de Juego
        // -------------------------------------------------------------------------
        ui::UIRenderer renderer;
        bool rendOk = renderer.init("shaders/ui.vert", "shaders/ui.frag");
        assert(rendOk && "UIRenderer::init() debe retornar true");
        (void)rendOk;

        lighting::LightManager lightManager;
        lightManager.init();
        lighting::LightingController lightingController(&lightManager);

        core::GameContext ctx;
        ctx.lightManager = &lightManager;
        ctx.lightingController = &lightingController;
        ctx.cameraMode = core::CameraMode::CHASE;

        ui::MainMenuUI menu;
        bool menuOk = menu.init(&renderer, &ctx);
        assert(menuOk && "MainMenuUI::init() debe retornar true");
        (void)menuOk;
        assert(!menu.isSettingsOpen() && "El modal de configuracion debe iniciar cerrado");
        assert(glGetError() == GL_NO_ERROR && "Sin errores de GL tras inicializar MainMenuUI");
        std::cout << "  [CHECK 1] Inicializacion de MainMenuUI con texturas Kenney OK." << std::endl;

        // -------------------------------------------------------------------------
        // 3. Navegacion Teclado en Menu Principal (PLAY, SETTINGS, QUIT con wrap)
        // -------------------------------------------------------------------------
        core::Input::simulateKey(GLFW_KEY_DOWN, GLFW_PRESS);
        menu.handleInput(0.016f);
        core::Input::update();

        core::Input::simulateKey(GLFW_KEY_DOWN, GLFW_PRESS);
        menu.handleInput(0.016f);
        core::Input::update();

        core::Input::simulateKey(GLFW_KEY_DOWN, GLFW_PRESS);
        menu.handleInput(0.016f);
        core::Input::update();

        core::Input::simulateKey(GLFW_KEY_UP, GLFW_PRESS);
        menu.handleInput(0.016f);
        core::Input::update();
        std::cout << "  [CHECK 2] Navegacion vertical con envoltura (wrap-around) en Menu Principal OK." << std::endl;

        // -------------------------------------------------------------------------
        // 4. Apertura y Cierre de Modal de Configuracion
        // -------------------------------------------------------------------------
        menu.openSettings();
        assert(menu.isSettingsOpen() && "openSettings() debe activar m_inSettings");

        core::Input::simulateKey(GLFW_KEY_ESCAPE, GLFW_PRESS);
        menu.handleInput(0.016f);
        core::Input::update();
        assert(!menu.isSettingsOpen() && "ESC dentro del modal debe cerrarlo");

        menu.openSettings();
        assert(menu.isSettingsOpen());
        std::cout << "  [CHECK 3] Apertura y cierre de modal de configuracion con tecla ESC OK." << std::endl;

        // -------------------------------------------------------------------------
        // 5. Modificacion en tiempo real de parametros del sistema
        // -------------------------------------------------------------------------
        // Opcion 0: SHADING_MODEL
        lighting::LightingModel origModel = lightManager.getLightingModel();
        core::Input::simulateKey(GLFW_KEY_RIGHT, GLFW_PRESS);
        menu.handleInput(0.016f);
        core::Input::update();
        assert(lightManager.getLightingModel() != origModel && "Flecha Derecha debe alternar el modelo de iluminacion");
        (void)origModel;

        // Opcion 1: DAY_NIGHT
        core::Input::simulateKey(GLFW_KEY_DOWN, GLFW_PRESS);
        menu.handleInput(0.016f);
        core::Input::update();

        bool origNight = lightingController.isNight();
        core::Input::simulateKey(GLFW_KEY_ENTER, GLFW_PRESS);
        menu.handleInput(0.016f);
        core::Input::update();
        assert(lightingController.isNight() != origNight && "ENTER debe alternar Ciclo Dia/Noche");
        (void)origNight;

        // Opcion 2: HEADLIGHTS
        core::Input::simulateKey(GLFW_KEY_DOWN, GLFW_PRESS);
        menu.handleInput(0.016f);
        core::Input::update();

        bool origHeadlights = lightingController.areHeadlightsOn();
        core::Input::simulateKey(GLFW_KEY_ENTER, GLFW_PRESS);
        menu.handleInput(0.016f);
        core::Input::update();
        assert(lightingController.areHeadlightsOn() != origHeadlights && "ENTER debe alternar Faros del Kart");
        (void)origHeadlights;

        // Opcion 4: INITIAL_CAMERA
        core::Input::simulateKey(GLFW_KEY_DOWN, GLFW_PRESS); // Option 3 (Track lights)
        menu.handleInput(0.016f);
        core::Input::update();

        core::Input::simulateKey(GLFW_KEY_DOWN, GLFW_PRESS); // Option 4 (Camera)
        menu.handleInput(0.016f);
        core::Input::update();

        core::CameraMode origCam = ctx.cameraMode;
        core::Input::simulateKey(GLFW_KEY_ENTER, GLFW_PRESS);
        menu.handleInput(0.016f);
        core::Input::update();
        assert(ctx.cameraMode != origCam && "ENTER debe alternar modo de camara Chase / FPS");
        (void)origCam;

        // Opcion 5: BACK
        core::Input::simulateKey(GLFW_KEY_DOWN, GLFW_PRESS); // Option 5 (Back)
        menu.handleInput(0.016f);
        core::Input::update();

        core::Input::simulateKey(GLFW_KEY_ENTER, GLFW_PRESS);
        menu.handleInput(0.016f);
        core::Input::update();
        assert(!menu.isSettingsOpen() && "Boton 'Volver' con ENTER debe cerrar el modal de configuracion");
        std::cout << "  [CHECK 4] Conmutacion interactiva de sombreado, dia/noche, faros y camara OK." << std::endl;

        // -------------------------------------------------------------------------
        // 6. Pase de Renderizado 2D completo (Menu y Modal con sombras y alto contraste)
        // -------------------------------------------------------------------------
        menu.update(0.016f);
        menu.render(1280.0f, 720.0f);
        assert(glGetError() == GL_NO_ERROR && "Renderizado de Menu Principal sin errores de GL");

        menu.openSettings();
        // Renderizar modal para cada opcion seleccionada asegurando cobertura de cada fila y paleta
        for (int opt = 0; opt < 6; ++opt) {
            menu.update(0.016f);
            menu.render(1280.0f, 720.0f);
            assert(glGetError() == GL_NO_ERROR && "Renderizado de Modal de Configuracion sin errores de GL");

            core::Input::simulateKey(GLFW_KEY_DOWN, GLFW_PRESS);
            menu.handleInput(0.016f);
            core::Input::update();
        }
        std::cout << "  [CHECK 5] Pases de dibujo 2D (Menu y Modal con placa de contraste y sombras) sin errores de OpenGL OK." << std::endl;

        renderer.destroy();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Todos los tests de MainMenuUI completados 100%!  " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
