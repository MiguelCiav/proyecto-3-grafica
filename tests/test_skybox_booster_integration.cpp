#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderer/Skybox.hpp"
#include "renderer/Shader.hpp"
#include "world/BoosterPad.hpp"
#include "world/TrackGrid.hpp"
#include "world/LevelLoader.hpp"
#include "physics/KartPhysics.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cassert>

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Integracion de Skybox y Boosters Procedimentales  " << std::endl;
    std::cout << "==========================================================" << std::endl;

    if (!glfwInit()) {
        std::cerr << "[FAIL] No se pudo inicializar GLFW." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Test Skybox & Booster Integration", nullptr, nullptr);
    if (!window) {
        std::cerr << "[FAIL] No se pudo crear ventana GLFW." << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "[FAIL] No se pudo inicializar GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    std::cout << "  [OK] Contexto OpenGL " << glGetString(GL_VERSION) << " activo." << std::endl;

    {
        // 1. Cargar pista y desplegar pads en rectas
        world::TrackGrid grid(12, 12);
        bool levelLoaded = world::LevelLoader::loadFromFile("assets/levels/track_oval.txt", grid);
        assert(levelLoaded);

        world::ZoneTriggerManager zoneManager;
        std::vector<world::BoosterPad> pads;

        std::vector<const world::TileData*> straightTiles;
        for (const auto& tile : grid.getAllTiles()) {
            if (tile.type == world::TileType::ROAD_STRAIGHT) {
                straightTiles.push_back(&tile);
            }
        }
        assert(!straightTiles.empty());

        size_t count = std::min<size_t>(3, straightTiles.size());
        size_t step = straightTiles.size() / count;
        for (size_t i = 0; i < count; ++i) {
            size_t idx = (i * step + step / 2) % straightTiles.size();
            const auto* tile = straightTiles[idx];
            world::BoosterPad pad(tile->worldPos, tile->rotationDegrees, 2.8f, 4.0f);
            pad.setBoostMultiplier(1.6f);
            pad.setBoostDuration(2.0f);
            pad.registerTrigger(zoneManager);
            pads.push_back(std::move(pad));
        }

        assert(pads.size() >= 2);
        std::cout << "  [CHECK 1] Pista cargada y " << pads.size() << " BoosterPads desplegados en rectas." << std::endl;

        // 2. Cargar shaders de boosters y skybox
        renderer::Shader boosterShader;
        bool bShaderOk = boosterShader.loadFromFiles("shaders/booster_procedural.vert", "shaders/booster_procedural.frag");
        assert(bShaderOk);

        renderer::Skybox skybox;
        bool sShaderOk = skybox.loadShader("shaders/skybox.vert", "shaders/skybox.frag");
        assert(sShaderOk);

        bool sTexOk = skybox.loadFromDirectory("assets/textures/skybox", ".png");
        assert(sTexOk);
        assert(skybox.isLoaded());
        std::cout << "  [CHECK 2] Shaders procedurales y Skybox 3D inicializados con exito." << std::endl;

        // 3. Probar física de contacto del kart con el booster pad
        Engine::Physics::KartPhysics kart;
        const auto& testPad = pads[0];
        kart.setPosition(testPad.getPosition());
        kart.resetSpeed();

        bool contacted = testPad.onKartContact(kart);
        assert(contacted);
        assert(kart.isBoosting());
        assert(kart.getSpeed() > 0.0f);
        assert(kart.getBoostMultiplier() == 1.6f);
        std::cout << "  [CHECK 3] Conduccion reactiva sobre el booster: impulso instantaneo y turbo 1.6x activos." << std::endl;

        // 4. Pase completo de renderizado con optimizacion GL_LEQUAL
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glm::mat4 proj = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 10.0f, 25.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        boosterShader.use();
        boosterShader.setMat4("u_View", view);
        boosterShader.setMat4("u_Projection", proj);

        float currentTime = 3.5f;
        for (const auto& pad : pads) {
            pad.draw(boosterShader, currentTime);
        }

        skybox.draw(view, proj);

        GLenum err = glGetError();
        assert(err == GL_NO_ERROR);
        std::cout << "  [CHECK 4] Pase de renderizado integrado (Boosters + Skybox) ejecutado sin errores." << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Integracion de Skybox y Boosters completada      " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
