#include "world/PropSpawner.hpp"
#include "world/PropArchetypeRegistry.hpp"
#include "world/BiomeSpawners.hpp"
#include "geometry/OBJLoader.hpp"
#include "geometry/ProceduralGeometry.hpp"
#include <iostream>
#include <filesystem>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace fs = std::filesystem;

namespace world {

PropSpawner::PropSpawner() = default;
PropSpawner::~PropSpawner() = default;

bool PropSpawner::init(const std::string& obstaclesDir) {
    // 1. Cargar modelos Kenney OBJ
    std::vector<std::string> modelFiles = {
        "cone.obj", "barrierRed.obj", "barrierWhite.obj", "barrierWall.obj",
        "box.obj", "debris-tire.obj"
    };

    for (const auto& file : modelFiles) {
        std::string fullPath = (fs::path(obstaclesDir) / file).generic_string();
        geometry::ModelData data;
        if (geometry::OBJLoader::load(fullPath, data) && !data.submeshes.empty()) {
            std::string key = fs::path(file).stem().string();
            renderer::MeshGL mesh;
            mesh.setup(data.submeshes[0]);
            m_propMeshes[key] = std::move(mesh);
            std::cout << "[OK] [PropSpawner] Prop cargado: " << key << std::endl;
        } else {
            std::cerr << "[AVISO] [PropSpawner] No se pudo cargar prop: " << fullPath << std::endl;
        }
    }

    // 2. Mallas procedurales adicionales
    auto addBox = [&](const std::string& key, float hx, float hy, float hz) {
        renderer::MeshGL mesh;
        mesh.setup(geometry::ProceduralGeometry::createBox(hx, hy, hz));
        m_propMeshes[key] = std::move(mesh);
    };

    auto addMesh = [&](const std::string& key, const geometry::SubMesh& submesh) {
        renderer::MeshGL mesh;
        mesh.setup(submesh);
        m_propMeshes[key] = std::move(mesh);
    };

    // Farolas y Torres de estadio
    addBox("lampPole", 0.12f, 2.1f, 0.12f);
    addBox("lampArm", 0.10f, 0.08f, 0.70f);
    addBox("lampBulb", 0.32f, 0.18f, 0.32f);
    addBox("stadiumTower", 0.35f, 5.0f, 0.35f);

    // Vegetación Low-Poly
    addBox("treeTrunk", 0.18f, 0.8f, 0.18f);
    addBox("treeFoliageBottom", 0.85f, 0.55f, 0.85f);
    addBox("treeFoliageTop", 0.55f, 0.50f, 0.55f);

    // Arco de Meta / Gantry
    addBox("gantryPillar", 0.16f, 2.30f, 0.16f);
    addBox("gantryFooting", 0.28f, 0.18f, 0.28f);
    addBox("gantryBeam", 2.70f, 0.16f, 0.20f);
    addBox("gantryLowerBeam", 2.40f, 0.08f, 0.12f);
    addBox("gantrySign", 1.65f, 0.35f, 0.06f);
    addBox("gantryLight", 0.12f, 0.12f, 0.12f);

    // Nocturno (Neon)
    addBox("neonArchPillar", 0.16f, 2.30f, 0.16f);
    addBox("neonArchBeam", 2.70f, 0.16f, 0.20f);
    addBox("neonArchTrimCyan", 2.70f, 0.06f, 0.24f);
    addBox("neonArchTrimMagenta", 2.70f, 0.06f, 0.24f);
    addBox("neonBeacon", 0.15f, 0.40f, 0.15f);
    addBox("neonBeaconTop", 0.18f, 0.12f, 0.18f);

    // Desierto
    addBox("cactusTrunk", 0.18f, 1.25f, 0.18f);
    addBox("cactusArmJoint", 0.24f, 0.10f, 0.10f);
    addBox("cactusArmSpire", 0.12f, 0.50f, 0.12f);
    addBox("desertRock", 0.65f, 0.55f, 0.65f);
    addBox("desertMesaRock", 1.20f, 0.85f, 1.10f);
    addBox("desertArchPillar", 0.40f, 2.50f, 0.40f);
    addBox("desertArchBeam", 2.90f, 0.35f, 0.45f);

    // Espacio
    addBox("spaceWarpPillar", 0.18f, 2.40f, 0.18f);
    addBox("spaceWarpBeam", 2.65f, 0.15f, 0.20f);
    addBox("spaceWarpTrimCyan", 2.65f, 0.08f, 0.24f);
    addBox("spaceWarpTrimMagenta", 2.65f, 0.08f, 0.24f);
    addBox("satelliteBody", 0.40f, 0.40f, 0.40f);
    addBox("satellitePanel", 1.10f, 0.04f, 0.35f);
    addBox("satelliteAntenna", 0.04f, 0.70f, 0.04f);
    addBox("spaceBeacon", 0.16f, 0.45f, 0.16f);
    addBox("spaceBeaconTop", 0.20f, 0.14f, 0.20f);
    addMesh("floatingStar", geometry::ProceduralGeometry::createStar(0.55f, 0.22f, 0.18f));

    // Acuático
    addBox("coralBranchTall", 0.14f, 1.10f, 0.14f);
    addBox("coralBranchSide", 0.12f, 0.50f, 0.12f);
    addMesh("coralBrain", geometry::ProceduralGeometry::createFacetedDome(0.52f, 0.45f, 3, 8));
    addBox("kelpSegment", 0.24f, 0.85f, 0.06f);
    addMesh("bubbleColumn", geometry::ProceduralGeometry::createBubbleColumn());
    addBox("seaRock", 0.70f, 0.55f, 0.70f);
    addBox("seaArchPillar", 0.22f, 2.50f, 0.22f);
    addBox("seaArchBeam", 2.80f, 0.25f, 0.30f);
    addBox("seaArchTrim", 2.80f, 0.08f, 0.36f);
    addBox("seaPedestal", 0.22f, 0.75f, 0.22f);
    addMesh("clamShell", geometry::ProceduralGeometry::createClamShell(0.52f, 68.0f));
    addMesh("seaPearl", geometry::ProceduralGeometry::createSphere(0.18f, 6, 12));

    // Volcánico
    addBox("volcanoArchPillar", 0.24f, 2.60f, 0.24f);
    addBox("volcanoArchBeam", 2.80f, 0.25f, 0.32f);
    addBox("volcanoArchTrimLava", 2.80f, 0.08f, 0.36f);
    addBox("basaltColumn", 0.35f, 1.60f, 0.35f);
    addBox("volcanoRock", 0.65f, 0.50f, 0.65f);
    addBox("lavaChimney", 0.26f, 1.10f, 0.26f);
    addBox("lavaVent", 0.40f, 0.08f, 0.40f);
    addMesh("fireBrazier", geometry::ProceduralGeometry::createBrazier(0.24f, 0.38f, 1.40f));
    addMesh("volcanoFlame", geometry::ProceduralGeometry::createFlame(0.36f, 1.05f, false));
    addMesh("volcanoFlameCore", geometry::ProceduralGeometry::createFlame(0.22f, 0.72f, true));
    addMesh("flameOrb", geometry::ProceduralGeometry::createFlame(0.36f, 1.05f, false));

    m_initialized = !m_propMeshes.empty();
    return m_initialized;
}

void PropSpawner::generateForGrid(const TrackGrid& grid, bool isNight, bool isDesert, bool isSpace, bool isAquatic, bool isVolcano) {
    BiomeType biome = BiomeType::GRASS_DAY;
    if (isVolcano) biome = BiomeType::VOLCANO;
    else if (isAquatic) biome = BiomeType::AQUATIC;
    else if (isSpace) biome = BiomeType::SPACE;
    else if (isDesert) biome = BiomeType::DESERT;
    else if (isNight) biome = BiomeType::GRASS_NIGHT;
    generateForGrid(grid, biome);
}

void PropSpawner::generateForGrid(const TrackGrid& grid, BiomeType biome) {
    clear();
    auto spawner = BiomePropSpawnerFactory::create(biome);
    if (spawner) {
        spawner->spawnProps(grid, *this);
    }
}

void PropSpawner::addProp(const std::string& type, const glm::vec3& pos, float rotDeg,
                          const glm::vec3& scale, bool hasCollision,
                          const glm::vec4& customColor, bool useCustomColor) {
    PropInstance inst;
    inst.type = type;
    inst.position = pos;
    inst.rotationDegrees = rotDeg;
    inst.scale = scale;
    inst.hasCollision = hasCollision;
    inst.customColor = customColor;
    inst.useCustomColor = useCustomColor;

    glm::mat4 mat = glm::translate(glm::mat4(1.0f), pos);
    mat = glm::rotate(mat, glm::radians(rotDeg), glm::vec3(0.0f, 1.0f, 0.0f));
    mat = glm::scale(mat, scale);
    inst.modelMatrix = mat;

    if (!hasCollision) {
        inst.aabbMin = glm::vec3(0.0f);
        inst.aabbMax = glm::vec3(0.0f);
    } else {
        PropArchetypeRegistry::calculateAABB(type, pos, rotDeg, scale, inst.aabbMin, inst.aabbMax);
    }

    m_props.push_back(inst);
}

void PropSpawner::update(float dt) {
    m_animTime += dt;
    m_starRotation += dt * 65.0f;
    if (m_starRotation >= 360.0f) {
        m_starRotation -= 360.0f;
    }

    for (auto& prop : m_props) {
        if (prop.type == "floatingStar") {
            float bob = std::sin(m_animTime * 2.8f + prop.position.x * 0.4f + prop.position.z * 0.4f) * 0.08f;
            glm::vec3 animatedPos = prop.position + glm::vec3(0.0f, bob, 0.0f);

            glm::mat4 mat = glm::translate(glm::mat4(1.0f), animatedPos);
            mat = glm::rotate(mat, glm::radians(m_starRotation + prop.rotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
            mat = glm::scale(mat, prop.scale);
            prop.modelMatrix = mat;
        } else if (prop.type == "seaPearl") {
            float pulse = 1.0f + 0.045f * std::sin(m_animTime * 2.8f + prop.position.x * 0.5f);
            glm::mat4 mat = glm::translate(glm::mat4(1.0f), prop.position);
            mat = glm::scale(mat, prop.scale * pulse);
            prop.modelMatrix = mat;
        } else if (prop.type == "clamShell") {
            float breath = 1.0f + 0.025f * std::sin(m_animTime * 2.2f + prop.position.x * 0.5f);
            float sway = 1.8f * std::sin(m_animTime * 1.6f + prop.position.z * 0.4f);
            glm::mat4 mat = glm::translate(glm::mat4(1.0f), prop.position);
            mat = glm::rotate(mat, glm::radians(prop.rotationDegrees + sway), glm::vec3(0.0f, 1.0f, 0.0f));
            mat = glm::scale(mat, prop.scale * glm::vec3(breath, breath, breath));
            prop.modelMatrix = mat;
        } else if (prop.type == "bubbleColumn") {
            float phase = prop.position.x * 1.7f + prop.position.z * 1.3f;
            float yAscend = std::fmod(m_animTime * 1.5f + phase, 1.2f) * 0.75f;
            float wobbleX = 0.12f * std::sin(m_animTime * 3.5f + phase);
            float wobbleZ = 0.10f * std::cos(m_animTime * 4.2f + phase);
            float pulse = 1.0f + 0.06f * std::sin(m_animTime * 5.0f + phase);

            glm::mat4 mat = glm::translate(glm::mat4(1.0f), prop.position + glm::vec3(wobbleX, yAscend, wobbleZ));
            mat = glm::scale(mat, prop.scale * glm::vec3(pulse, 1.0f, pulse));
            prop.modelMatrix = mat;
        } else if (prop.type == "volcanoFlame" || prop.type == "volcanoFlameCore" || prop.type == "flameOrb") {
            float phase = prop.position.x * 2.3f + prop.position.z * 1.7f;
            float flickerY = 1.0f + 0.15f * std::sin(m_animTime * 14.5f + phase) + 0.07f * std::cos(m_animTime * 23.0f + phase * 1.5f);
            float flickerXZ = 1.0f + 0.08f * std::cos(m_animTime * 11.0f + phase);
            float swayX = 4.5f * std::sin(m_animTime * 5.5f + phase);
            float swayZ = 3.5f * std::cos(m_animTime * 6.5f + phase);
            float slowDrift = m_animTime * 28.0f + prop.rotationDegrees;

            glm::mat4 mat = glm::translate(glm::mat4(1.0f), prop.position);
            mat = glm::rotate(mat, glm::radians(swayX), glm::vec3(1.0f, 0.0f, 0.0f));
            mat = glm::rotate(mat, glm::radians(swayZ), glm::vec3(0.0f, 0.0f, 1.0f));
            mat = glm::rotate(mat, glm::radians(slowDrift), glm::vec3(0.0f, 1.0f, 0.0f));
            mat = glm::scale(mat, prop.scale * glm::vec3(flickerXZ, flickerY, flickerXZ));
            prop.modelMatrix = mat;
        }
    }
}

void PropSpawner::render(const renderer::Shader& shader) const {
    if (!m_initialized) return;

    for (const auto& prop : m_props) {
        auto it = m_propMeshes.find(prop.type);
        if (it != m_propMeshes.end()) {
            shader.setMat4("u_Model", prop.modelMatrix);
            PropArchetypeRegistry::applyMaterial(shader, prop);
            it->second.draw();
        }
    }
}

} // namespace world
