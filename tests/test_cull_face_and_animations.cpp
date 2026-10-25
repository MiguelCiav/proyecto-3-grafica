#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "geometry/ProceduralGeometry.hpp"
#include "world/BoosterPad.hpp"
#include "world/PropSpawner.hpp"
#include "world/TrackGrid.hpp"
#include "vfx/KartParticleEmitter.hpp"
#include "vfx/ParticleSystem.hpp"
#include "vfx/SkidmarkManager.hpp"
#include "renderer/Skybox.hpp"
#include "ui/RacingHud.hpp"
#include "physics/KartPhysics.hpp"

static void checkGlError(const std::string& step) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "[ERROR OpenGL] en " << step << ": codigo 0x" << std::hex << err << std::endl;
        assert(false);
    }
}

// Verifica si un triangulo visto desde la direccion de la camara tiene devanado CCW
static float calculate2DAreaOnScreen(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                                     const glm::mat4& viewProj) {
    glm::vec4 p0 = viewProj * glm::vec4(v0, 1.0f);
    glm::vec4 p1 = viewProj * glm::vec4(v1, 1.0f);
    glm::vec4 p2 = viewProj * glm::vec4(v2, 1.0f);

    glm::vec2 ndc0(p0.x / p0.w, p0.y / p0.w);
    glm::vec2 ndc1(p1.x / p1.w, p1.y / p1.w);
    glm::vec2 ndc2(p2.x / p2.w, p2.y / p2.w);

    // Area con signo en 2D (positivo = CCW, negativo = CW)
    return 0.5f * ((ndc1.x - ndc0.x) * (ndc2.y - ndc0.y) - (ndc2.x - ndc0.x) * (ndc1.y - ndc0.y));
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Back-Face Culling, Particulas y Animaciones Props " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // TEST 1: Validacion Matematica de Devanado CCW en Geometrias Procedurales
    // -------------------------------------------------------------------------
    {
        // Camara cenital mirando hacia abajo (+Y -> origen, up = (0, 0, -1))
        glm::mat4 viewTop = glm::lookAt(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 100.0f);
        glm::mat4 vpTop = proj * viewTop;

        // 1.1 Plano de Suelo: indices {0, 3, 2, 2, 1, 0}
        float size = 50.0f;
        float y = 0.0f;
        std::vector<glm::vec3> groundVerts = {
            {-size, y, -size}, // 0
            { size, y, -size}, // 1
            { size, y,  size}, // 2
            {-size, y,  size}  // 3
        };
        float gTri1 = calculate2DAreaOnScreen(groundVerts[0], groundVerts[3], groundVerts[2], vpTop);
        float gTri2 = calculate2DAreaOnScreen(groundVerts[2], groundVerts[1], groundVerts[0], vpTop);
        (void)gTri1;
        (void)gTri2;
        assert(gTri1 > 0.0f && "Ground tri 1 debe ser CCW visto desde arriba");
        assert(gTri2 > 0.0f && "Ground tri 2 debe ser CCW visto desde arriba");

        // 1.2 BoosterPad
        // Tri 1: (-0.5, 0, -0.5) -> (-0.5, 0, 0.5) -> (0.5, 0, 0.5)
        // Tri 2: (-0.5, 0, -0.5) -> (0.5, 0, 0.5) -> (0.5, 0, -0.5)
        float bTri1 = calculate2DAreaOnScreen({-0.5f, 0.0f, -0.5f}, {-0.5f, 0.0f, 0.5f}, {0.5f, 0.0f, 0.5f}, vpTop);
        float bTri2 = calculate2DAreaOnScreen({-0.5f, 0.0f, -0.5f}, {0.5f, 0.0f, 0.5f}, {0.5f, 0.0f, -0.5f}, vpTop);
        (void)bTri1;
        (void)bTri2;
        assert(bTri1 > 0.0f && "Booster tri 1 debe ser CCW visto desde arriba");
        assert(bTri2 > 0.0f && "Booster tri 2 debe ser CCW visto desde arriba");

        // 1.3 Procedural Box: Todas las 6 caras deben ser outward CCW
        auto box = geometry::ProceduralGeometry::createBox(1.0f, 1.0f, 1.0f);
        assert(box.indices.size() == 36);
        for (size_t i = 0; i < box.indices.size(); i += 3) {
            glm::vec3 v0 = box.vertices[box.indices[i + 0]].position;
            glm::vec3 v1 = box.vertices[box.indices[i + 1]].position;
            glm::vec3 v2 = box.vertices[box.indices[i + 2]].position;
            glm::vec3 normal = glm::cross(v1 - v0, v2 - v0);
            glm::vec3 center = (v0 + v1 + v2) / 3.0f;
            (void)normal;
            (void)center;
            // El vector normal calculado por la regla de la mano derecha debe apuntar hacia afuera del cubo
            assert(glm::dot(normal, center) > 0.0f && "Caras del cubo deben apuntar hacia afuera en CCW");
        }

        // 1.4 Procedural Sphere: Los triangulos deben tener normales hacia afuera
        auto sphere = geometry::ProceduralGeometry::createSphere(1.0f, 8, 12);
        for (size_t i = 0; i < sphere.indices.size(); i += 3) {
            glm::vec3 v0 = sphere.vertices[sphere.indices[i + 0]].position;
            glm::vec3 v1 = sphere.vertices[sphere.indices[i + 1]].position;
            glm::vec3 v2 = sphere.vertices[sphere.indices[i + 2]].position;
            glm::vec3 normal = glm::cross(v1 - v0, v2 - v0);
            glm::vec3 center = (v0 + v1 + v2) / 3.0f;
            (void)normal;
            (void)center;
            assert(glm::dot(normal, center) > 0.0f && "Triangulos de la esfera deben ser outward CCW");
        }

        std::cout << "  [CHECK 1] Devanado CCW validado matematicamente en todas las mallas procedurales." << std::endl;
    }

    // -------------------------------------------------------------------------
    // TEST 2: Inicializacion de OpenGL y Guardas de Estado para GL_CULL_FACE
    // -------------------------------------------------------------------------
    if (!glfwInit()) {
        std::cerr << "[ERROR] Fallo inicializacion GLFW" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Test Cull Face & Animations", nullptr, nullptr);
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

    // Habilitar Back-Face Culling como en Application::init()
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    assert(glIsEnabled(GL_CULL_FACE) == GL_TRUE);

    {
        // 2.1 Verificar que Skybox guarde y restaure GL_CULL_FACE
        renderer::Skybox skybox;
        skybox.loadShader("shaders/skybox.vert", "shaders/skybox.frag");
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 proj = glm::mat4(1.0f);
        skybox.draw(view, proj);
        assert(glIsEnabled(GL_CULL_FACE) == GL_TRUE && "Skybox::draw debe restaurar GL_CULL_FACE a activo");

        // 2.2 Verificar que RacingHud guarde y restaure GL_CULL_FACE
        ui::RacingHud hud;
        hud.init();
        ui::TelemetryData telem;
        ui::LapTimingData timing;
        hud.render(800.0f, 600.0f, telem, timing);
        assert(glIsEnabled(GL_CULL_FACE) == GL_TRUE && "RacingHud::render debe restaurar GL_CULL_FACE a activo");

        // 2.3 Verificar que ParticleSystem guarde y restaure GL_CULL_FACE
        vfx::ParticleSystem ps(100);
        ps.init();
        ps.loadShader("shaders/particle.vert", "shaders/particle.frag");
        ps.emit(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f), 0.2f, 1.0f);
        ps.render(view, proj);
        assert(glIsEnabled(GL_CULL_FACE) == GL_TRUE && "ParticleSystem::render debe restaurar GL_CULL_FACE a activo");

        // 2.4 Verificar que SkidmarkManager guarde y restaure GL_CULL_FACE
        vfx::SkidmarkManager sm;
        sm.init();
        sm.updateWithWheelPositions(0.016f, glm::vec3(-0.35f, 0.012f, 0.0f), glm::vec3(0.35f, 0.012f, 0.0f), 12.0f, 0.8f, true, false);
        sm.updateWithWheelPositions(0.016f, glm::vec3(-0.35f, 0.012f, 1.0f), glm::vec3(0.35f, 0.012f, 1.0f), 12.0f, 0.8f, true, false);
        sm.render(view, proj);
        assert(glIsEnabled(GL_CULL_FACE) == GL_TRUE && "SkidmarkManager::render debe restaurar GL_CULL_FACE a activo");

        checkGlError("Pases de renderizado con guardas de estado");
        std::cout << "  [CHECK 2] Back-Face Culling activo globalmente y guardas de estado verificadas en GPU." << std::endl;
    }

    // -------------------------------------------------------------------------
    // TEST 3: Inicializacion Automatica de Shaders en KartParticleEmitter y Burbujas
    // -------------------------------------------------------------------------
    {
        vfx::KartParticleEmitter emitter;
        assert(emitter.init() && "init() de KartParticleEmitter debe cargar buffers y shaders");
        assert(emitter.getSmokeSystem().isShaderLoaded() && "El shader de humo debe estar cargado");
        assert(emitter.getBoostSystem().isShaderLoaded() && "El shader de boost debe estar cargado");

        // Simular aceleración en tierra (humo)
        Engine::Physics::KartPhysics physics;
        emitter.update(0.1f, physics, true);
        assert(emitter.getSmokeSystem().getActiveParticleCount() > 0 && "Debe generar particulas de humo");

        // Activar modo subacuático (burbujas)
        emitter.setUnderwater(true);
        assert(emitter.isUnderwater() == true);
        emitter.getSmokeSystem().clear();

        emitter.update(0.1f, physics, true);
        size_t bubbleCount = emitter.getSmokeSystem().getActiveParticleCount();
        (void)bubbleCount;
        assert(bubbleCount > 0 && "Debe generar particulas de burbujas");

        // Renderizado sin fallos con culling activo
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f);
        emitter.render(view, proj);
        checkGlError("Renderizado de particulas (humo y burbujas)");

        std::cout << "  [CHECK 3] Shaders de particulas cargados y emision de humo/burbujas validada." << std::endl;
    }

    // -------------------------------------------------------------------------
    // TEST 4: Animaciones Procedurales de Props del Mundo (Burbujas Arrecife, Fuego, Ostras)
    // -------------------------------------------------------------------------
    {
        world::PropSpawner spawner;
        spawner.init();
        world::TrackGrid grid(10, 10);

        // 4.1 Generar props para Bioma Acuático
        spawner.generateForGrid(grid, world::BiomeType::AQUATIC);
        const auto& props = spawner.getProps();

        bool hasBubbleColumn = false;
        bool hasSeaPearl = false;
        bool hasClamShell = false;
        glm::mat4 initialBubbleMat(1.0f);
        glm::mat4 initialPearlMat(1.0f);
        glm::mat4 initialClamMat(1.0f);

        for (const auto& p : props) {
            if (p.type == "bubbleColumn" && !hasBubbleColumn) {
                hasBubbleColumn = true;
                initialBubbleMat = p.modelMatrix;
            } else if (p.type == "seaPearl" && !hasSeaPearl) {
                hasSeaPearl = true;
                initialPearlMat = p.modelMatrix;
            } else if (p.type == "clamShell" && !hasClamShell) {
                hasClamShell = true;
                initialClamMat = p.modelMatrix;
            }
        }

        assert(hasBubbleColumn && "Debe existir al menos una columna de burbujas en bioma acuatico");
        assert(hasSeaPearl && "Debe existir al menos una perla marina en bioma acuatico");
        assert(hasClamShell && "Debe existir al menos una ostra en bioma acuatico");

        // Ejecutar update con avance de tiempo
        spawner.update(0.5f);

        bool bubbleMoved = false;
        bool pearlPulsed = false;
        bool clamAnimated = false;
        for (const auto& p : spawner.getProps()) {
            if (p.type == "bubbleColumn" && p.modelMatrix != initialBubbleMat) bubbleMoved = true;
            if (p.type == "seaPearl" && p.modelMatrix != initialPearlMat) pearlPulsed = true;
            if (p.type == "clamShell" && p.modelMatrix != initialClamMat) clamAnimated = true;
        }
        (void)bubbleMoved;
        (void)pearlPulsed;
        (void)clamAnimated;
        assert(bubbleMoved && "La columna de burbujas debe animar su matriz de modelo en update()");
        assert(pearlPulsed && "La perla marina debe animar su matriz de modelo en update()");
        assert(clamAnimated && "La ostra gigante debe animar su balanceo/respiracion en update()");

        // 4.2 Generar props para Bioma Volcánico (Fuego)
        spawner.generateForGrid(grid, world::BiomeType::VOLCANO);
        bool hasVolcanoFlame = false;
        glm::mat4 initialFlameMat(1.0f);
        for (const auto& p : spawner.getProps()) {
            if (p.type == "volcanoFlame") {
                hasVolcanoFlame = true;
                initialFlameMat = p.modelMatrix;
                break;
            }
        }
        (void)hasVolcanoFlame;
        assert(hasVolcanoFlame && "Debe existir fuego volcanico en bioma volcan");

        spawner.update(0.5f);
        bool flameAnimated = false;
        for (const auto& p : spawner.getProps()) {
            if (p.type == "volcanoFlame" && p.modelMatrix != initialFlameMat) {
                flameAnimated = true;
                break;
            }
        }
        (void)flameAnimated;
        assert(flameAnimated && "El fuego volcanico debe oscilar y vibrar dinamicamente en update()");

        std::cout << "  [CHECK 4] Animaciones procedurales de burbujas, fuego, perlas y ostras verificadas." << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Todos los tests de Cull Face, VFX y Animaciones  " << std::endl;
    std::cout << "         completados con exito (100% de criterios UCV).   " << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
