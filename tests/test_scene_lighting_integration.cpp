#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderer/Shader.hpp"
#include "lighting/LightManager.hpp"
#include "lighting/LightingController.hpp"
#include "world/TrackGrid.hpp"
#include "world/LevelLoader.hpp"
#include <iostream>
#include <cassert>
#include <vector>

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Integracion de Iluminacion en Escena y Circuito   " << std::endl;
    std::cout << "  Validacion de las 3 fuentes de luz y conmutacion        " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // 1. Contexto OpenGL oculto para validación en GPU
    if (!glfwInit()) {
        std::cerr << "[FAIL] No se pudo inicializar GLFW." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Test Scene Lighting", nullptr, nullptr);
    if (!window) {
        std::cerr << "[FAIL] No se pudo crear contexto OpenGL." << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "[FAIL] No se pudo inicializar GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    std::cout << "  [OK] Contexto OpenGL " << glGetString(GL_VERSION) << " activo." << std::endl;

    // 2. Compilar shaders lit
    renderer::Shader litShader;
    bool loaded = litShader.loadFromFiles("shaders/lit.vert", "shaders/lit.frag");
    assert(loaded && litShader.isValid());
    std::cout << "  [CHECK 1] Shader lit cargado y vinculado exitosamente (ID: "
              << litShader.getID() << ")." << std::endl;

    // 3. Cargar pista y configurar distribucion de fuentes puntuales
    world::TrackGrid trackGrid;
    bool trackLoaded = world::LevelLoader::loadFromFile("assets/levels/track_classic_oval.txt", trackGrid);
    assert(trackLoaded);

    lighting::LightManager lightManager;
    lightManager.init();

    lighting::LightingController lightingController(&lightManager);

    // Configurar distribucion de farolas en base a la pista
    std::vector<glm::vec3> roadPositions;
    for (const auto& tile : trackGrid.getAllTiles()) {
        if (tile.type != world::TileType::EMPTY && tile.type != world::TileType::OFFROAD_GRASS) {
            roadPositions.push_back(tile.worldPos);
        }
    }
    assert(roadPositions.size() >= 4);

    lightManager.clearPointLights();
    size_t n = roadPositions.size();
    size_t count = (n >= 6) ? 6 : 4;
    for (size_t i = 0; i < count; ++i) {
        size_t idx = (i * n) / count;
        lighting::PointLight lamp;
        lamp.position = roadPositions[idx] + glm::vec3(0.0f, 4.2f, 0.0f);
        lamp.ambient  = glm::vec3(0.08f, 0.06f, 0.03f);
        lamp.diffuse  = glm::vec3(1.0f, 0.85f, 0.45f);
        lamp.specular = glm::vec3(1.0f, 0.90f, 0.60f);
        lamp.constant = 1.0f;
        lamp.linear   = 0.07f;
        lamp.quadratic = 0.017f;
        lamp.enabled  = true;
        lightManager.addPointLight(lamp);
    }

    assert(lightManager.getPointLightCount() >= 4);
    std::cout << "  [CHECK 2] Pista configurada con " << lightManager.getPointLightCount()
              << " luces puntuales calibradas a lo largo del asfalto." << std::endl;

    // 4. Cinemática de faros frontales y actualización de Spotlight
    glm::vec3 kartPos(0.0f, 0.0f, 0.0f);
    glm::vec3 kartForward(0.0f, 0.0f, 1.0f);
    lightingController.update(kartPos, kartForward, 0.016f);

    const auto& spot = lightManager.getSpotLight();
    assert(spot.enabled == true);
    assert(spot.position.y > 0.35f);
    assert(spot.direction == kartForward);
    std::cout << "  [CHECK 3] Luz focal (Spotlight) anclada al morro frontal del vehiculo." << std::endl;

    // 5. Transferencia de uniforms y conmutación en GPU de modelos de sombreado
    litShader.use();
    glm::vec3 cameraPos(0.0f, 5.0f, -10.0f);

    for (int model = 0; model < 3; ++model) {
        lighting::LightingModel m = static_cast<lighting::LightingModel>(model);
        lightingController.setLightingModel(m);
        lightManager.applyToShader(litShader, cameraPos);

        GLenum err = glGetError();
        assert(err == GL_NO_ERROR);
    }
    std::cout << "  [CHECK 4] Conmutacion de los 3 modelos (Flat, Lambert, Blinn-Phong) ejecutada en GPU sin errores." << std::endl;

    // 6. Alternancia de faros encendidos / apagados
    lightingController.toggleHeadlights();
    assert(!lightingController.areHeadlightsOn());
    lightManager.applyToShader(litShader, cameraPos);

    lightingController.toggleHeadlights();
    assert(lightingController.areHeadlightsOn());
    lightManager.applyToShader(litShader, cameraPos);
    std::cout << "  [CHECK 5] Alternancia de faros de vehiculo (Spotlight) verificada en GPU." << std::endl;

    litShader.unbind();
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "\n==========================================================" << std::endl;
    std::cout << "  [SUCCESS] Integracion de iluminacion global verificada! " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
