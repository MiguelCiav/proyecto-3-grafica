#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "world/PropSpawner.hpp"
#include "world/PropArchetypeRegistry.hpp"
#include "world/BiomeSpawners.hpp"
#include "world/TrackGrid.hpp"
#include "geometry/ProceduralGeometry.hpp"
#include "renderer/Shader.hpp"

#include <iostream>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

static void validateMesh(const geometry::SubMesh& mesh, const std::string& name) {
    assert(!mesh.vertices.empty() && "Vertices must not be empty");
    assert(!mesh.indices.empty() && "Indices must not be empty");
    assert(mesh.indices.size() % 3 == 0 && "Indices must form complete triangles");

    for (uint32_t idx : mesh.indices) {
        assert(idx < mesh.vertices.size() && "Index out of vertex bounds");
        (void)idx;
    }

    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        const auto& v = mesh.vertices[i];
        float len = glm::length(v.normal);
        assert(!std::isnan(len) && "Normal vector length is NaN");
        assert(len > 0.0001f && "Normal vector must not be zero-length");
        (void)len;
    }

    std::cout << "    [ProceduralGeometry] " << name << ": "
              << mesh.vertices.size() << " vertices, "
              << (mesh.indices.size() / 3) << " triangles - OK" << std::endl;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Spawner de Props y Biomas (PropSpawner)           " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // 1. Validar ProceduralGeometry directamente (CPU, sin requerir OpenGL)
    // -------------------------------------------------------------------------
    std::cout << "\n[FASE 1] Verificacion de ProceduralGeometry (Primitivas CPU)..." << std::endl;
    auto brazier = geometry::ProceduralGeometry::createBrazier(0.24f, 0.38f, 1.40f);
    validateMesh(brazier, "createBrazier");

    auto facetedDome = geometry::ProceduralGeometry::createFacetedDome(0.52f, 0.45f, 3, 8);
    validateMesh(facetedDome, "createFacetedDome");

    auto clamShell = geometry::ProceduralGeometry::createClamShell(0.52f, 68.0f);
    validateMesh(clamShell, "createClamShell");

    auto star = geometry::ProceduralGeometry::createStar(0.55f, 0.22f, 0.18f);
    validateMesh(star, "createStar");

    auto sphere = geometry::ProceduralGeometry::createSphere(0.18f, 6, 12);
    validateMesh(sphere, "createSphere");

    auto bubbleColumn = geometry::ProceduralGeometry::createBubbleColumn();
    validateMesh(bubbleColumn, "createBubbleColumn");

    std::cout << "  [CHECK 1] ProceduralGeometry: Todas las primitivas procedurales generadas con vertices, indices y normales consistentes: OK" << std::endl;

    // -------------------------------------------------------------------------
    // 2. Validar PropArchetypeRegistry::calculateAABB para arquetipos requeridos
    // -------------------------------------------------------------------------
    std::cout << "\n[FASE 2] Verificacion de AABB en PropArchetypeRegistry..." << std::endl;
    std::vector<std::string> testArchetypes = { "barrierWall", "coralBrain", "clamShell", "fireBrazier" };
    glm::vec3 testPos(10.0f, 0.0f, -5.0f);
    glm::vec3 testScale(1.0f, 1.0f, 1.0f);

    for (const auto& archName : testArchetypes) {
        glm::vec3 minB(0.0f), maxB(0.0f);
        world::PropArchetypeRegistry::calculateAABB(archName, testPos, 0.0f, testScale, minB, maxB);

        assert(minB.x <= maxB.x && "AABB min.x must be <= max.x");
        assert(minB.y <= maxB.y && "AABB min.y must be <= max.y");
        assert(minB.z <= maxB.z && "AABB min.z must be <= max.z");

        glm::vec3 extents = maxB - minB;
        assert(extents.x > 0.001f && extents.y > 0.001f && extents.z > 0.001f && "AABB must have non-zero volume");
        (void)extents;

        std::cout << "    [AABB] " << archName << ": min("
                  << minB.x << ", " << minB.y << ", " << minB.z << ") -> max("
                  << maxB.x << ", " << maxB.y << ", " << maxB.z << ") - OK" << std::endl;
    }

    // Verificar rotación en barrierWall (debe intercambiar dimensiones en X/Z)
    {
        glm::vec3 minRot0(0.0f), maxRot0(0.0f);
        glm::vec3 minRot90(0.0f), maxRot90(0.0f);
        world::PropArchetypeRegistry::calculateAABB("barrierWall", testPos, 0.0f, testScale, minRot0, maxRot0);
        world::PropArchetypeRegistry::calculateAABB("barrierWall", testPos, 90.0f, testScale, minRot90, maxRot90);

        float dx0 = maxRot0.x - minRot0.x;
        float dz0 = maxRot0.z - minRot0.z;
        float dx90 = maxRot90.x - minRot90.x;
        float dz90 = maxRot90.z - minRot90.z;

        assert(std::abs(dx0 - dz90) < 0.001f && "barrierWall X extent at 0 deg must match Z extent at 90 deg");
        assert(std::abs(dz0 - dx90) < 0.001f && "barrierWall Z extent at 0 deg must match X extent at 90 deg");
        (void)dx0; (void)dz0; (void)dx90; (void)dz90;
    }

    std::cout << "  [CHECK 2] PropArchetypeRegistry::calculateAABB: barrierWall, coralBrain, clamShell, fireBrazier verificados con cajas envolventes validas y rotacion consistente: OK" << std::endl;

    // -------------------------------------------------------------------------
    // 3. Inicializar GLFW y Contexto OpenGL Headless
    // -------------------------------------------------------------------------
    std::cout << "\n[FASE 3] Inicializacion de Contexto OpenGL Headless..." << std::endl;
    if (!glfwInit()) {
        std::cerr << "[FAIL] No se pudo inicializar GLFW." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Test PropSpawner Headless", nullptr, nullptr);
    if (!window) {
        std::cerr << "[FAIL] No se pudo crear la ventana headless GLFW." << std::endl;
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

    std::cout << "  [CHECK 3] Contexto OpenGL 3.3 Core headless inicializado exitosamente: OK" << std::endl;

    // Bloque delimitado para destruccion RAII de objetos OpenGL antes del shutdown
    {
        // -------------------------------------------------------------------------
        // 4. Inicializar PropSpawner y Cargar Modelos + Geometrías Procedimentales
        // -------------------------------------------------------------------------
        std::cout << "\n[FASE 4] Inicializacion de PropSpawner..." << std::endl;
        std::string obstaclesDir = "assets/models/obstacles";
        if (!fs::exists(obstaclesDir) && fs::exists("../assets/models/obstacles")) {
            obstaclesDir = "../assets/models/obstacles";
        }

        world::PropSpawner spawner;
        bool initOk = spawner.init(obstaclesDir);
        assert(initOk && "PropSpawner::init() debe retornar true");
        (void)initOk;
        std::cout << "  [CHECK 4] PropSpawner::init: Modelos Kenney y primitivas procedurales instanciados en GPU: OK" << std::endl;

        // -------------------------------------------------------------------------
        // 5. Configurar TrackGrid de prueba (10x10 con rectas, curvas y meta)
        // -------------------------------------------------------------------------
        std::cout << "\n[FASE 5] Generacion de Props en los 5 Biomas sobre TrackGrid..." << std::endl;
        world::TrackGrid grid(10, 10);
        grid.setTile(1, 1, world::TileType::ROAD_CORNER_SMALL, 0.0f);
        grid.setTile(2, 1, world::TileType::START_FINISH, 90.0f);
        grid.setTile(3, 1, world::TileType::ROAD_STRAIGHT, 90.0f);
        grid.setTile(4, 1, world::TileType::ROAD_STRAIGHT, 90.0f);
        grid.setTile(5, 1, world::TileType::ROAD_CORNER_SMALL, 90.0f);
        grid.setTile(5, 2, world::TileType::ROAD_STRAIGHT, 0.0f);
        grid.setTile(5, 3, world::TileType::ROAD_CORNER_SMALL, 180.0f);
        grid.setTile(4, 3, world::TileType::ROAD_STRAIGHT, 270.0f);
        grid.setTile(3, 3, world::TileType::ROAD_STRAIGHT, 270.0f);
        grid.setTile(2, 3, world::TileType::ROAD_STRAIGHT, 270.0f);
        grid.setTile(1, 3, world::TileType::ROAD_CORNER_SMALL, 270.0f);
        grid.setTile(1, 2, world::TileType::ROAD_STRAIGHT, 0.0f);
        grid.updateModelMatrices();

        const std::vector<std::pair<world::BiomeType, std::string>> biomes = {
            { world::BiomeType::GRASS_DAY,   "GRASS_DAY" },
            { world::BiomeType::GRASS_NIGHT, "GRASS_NIGHT" },
            { world::BiomeType::DESERT,      "DESERT" },
            { world::BiomeType::SPACE,       "SPACE" },
            { world::BiomeType::AQUATIC,     "AQUATIC" },
            { world::BiomeType::VOLCANO,     "VOLCANO" }
        };

        for (const auto& [biomeType, biomeName] : biomes) {
            spawner.generateForGrid(grid, biomeType);
            const auto& props = spawner.getProps();

            assert(!props.empty() && "Spawner props list must not be empty for this biome");

            int collidableCount = 0;
            for (const auto& prop : props) {
                assert(!prop.type.empty() && "Prop type must not be empty");

                if (prop.hasCollision) {
                    collidableCount++;
                    assert(prop.aabbMin.x <= prop.aabbMax.x && "Collision aabbMin.x must be <= aabbMax.x");
                    assert(prop.aabbMin.y <= prop.aabbMax.y && "Collision aabbMin.y must be <= aabbMax.y");
                    assert(prop.aabbMin.z <= prop.aabbMax.z && "Collision aabbMin.z must be <= aabbMax.z");
                }

                // Verificar matriz de modelo válida
                for (int c = 0; c < 4; ++c) {
                    for (int r = 0; r < 4; ++r) {
                        assert(!std::isnan(prop.modelMatrix[c][r]) && "ModelMatrix has NaN values");
                    }
                }
            }

            // Validaciones específicas según el bioma
            if (biomeType == world::BiomeType::SPACE) {
                assert(!spawner.getStarLights().empty() && "Space biome must register StarLights");
            } else if (biomeType == world::BiomeType::AQUATIC || biomeType == world::BiomeType::VOLCANO) {
                assert(!spawner.getLampPositions().empty() && "Aquatic and Volcano biomes must register light positions");
            }

            std::cout << "    [Biome] " << biomeName << ": "
                      << props.size() << " props (" << collidableCount
                      << " colisionables), Luces: " << spawner.getLampPositions().size()
                      << ", StarLights: " << spawner.getStarLights().size() << " - OK" << std::endl;
        }

        // Probar sobrecarga retrocompatible con flags booleanos
        spawner.generateForGrid(grid, false, true, false, false, false); // DESERT
        assert(!spawner.getProps().empty() && "generateForGrid with flags must succeed");

        std::cout << "  [CHECK 5] PropSpawner::generateForGrid: Generacion de props en todos los biomas (GRASS_DAY, GRASS_NIGHT, DESERT, SPACE, AQUATIC, VOLCANO) validada con AABBs y matrices consistentes: OK" << std::endl;

        // -------------------------------------------------------------------------
        // 6. Validar Animación y Actualización en Tiempo Real (update)
        // -------------------------------------------------------------------------
        std::cout << "\n[FASE 6] Validacion de Animacion y Actualizacion Dinamica..." << std::endl;
        spawner.generateForGrid(grid, world::BiomeType::SPACE);

        // Buscar floatingStar para comprobar movimiento
        const world::PropInstance* animProp = nullptr;
        for (const auto& prop : spawner.getProps()) {
            if (prop.type == "floatingStar") {
                animProp = &prop;
                break;
            }
        }
        assert(animProp != nullptr && "SPACE biome must have at least one floatingStar");

        glm::mat4 initialMat = animProp->modelMatrix;
        (void)initialMat;
        spawner.update(0.15f);

        // Re-localizar la estrella tras el update y constatar cambio en modelMatrix
        const world::PropInstance* updatedProp = nullptr;
        for (const auto& prop : spawner.getProps()) {
            if (prop.type == "floatingStar") {
                updatedProp = &prop;
                break;
            }
        }
        assert(updatedProp != nullptr);
        (void)updatedProp;
        assert(updatedProp->modelMatrix != initialMat && "floatingStar modelMatrix must be animated after update()");

        std::cout << "  [CHECK 6] PropSpawner::update: Animacion de rotacion y oscilacion procedural en tiempo real: OK" << std::endl;

        // -------------------------------------------------------------------------
        // 7. Validar Pase de Renderizado en GPU (render)
        // -------------------------------------------------------------------------
        std::cout << "\n[FASE 7] Pase de Dibujo de Props en GPU..." << std::endl;
        renderer::Shader litShader;
        std::string vertPath = "shaders/lit.vert";
        std::string fragPath = "shaders/lit.frag";
        if (!fs::exists(vertPath) && fs::exists("../shaders/lit.vert")) {
            vertPath = "../shaders/lit.vert";
            fragPath = "../shaders/lit.frag";
        }

        bool shaderOk = litShader.loadFromFiles(vertPath, fragPath);
        assert(shaderOk && "lit.vert y lit.frag deben compilarse exitosamente");
        (void)shaderOk;

        litShader.use();
        spawner.render(litShader);

        GLenum glErr = glGetError();
        assert(glErr == GL_NO_ERROR && "OpenGL error during PropSpawner::render");
        (void)glErr;

        std::cout << "  [CHECK 7] PropSpawner::render: Pase de dibujo en GPU ejecutado sin errores OpenGL: OK" << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "\n==========================================================" << std::endl;
    std::cout << "  [SUCCESS] Todos los tests unitarios e integrativos de   " << std::endl;
    std::cout << "            PropSpawner pasaron exitosamente (100% PASS)  " << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}
