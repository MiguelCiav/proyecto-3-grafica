#include "world/PropSpawner.hpp"
#include "geometry/OBJLoader.hpp"
#include <iostream>
#include <filesystem>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace fs = std::filesystem;

namespace world {

static geometry::SubMesh createBoxSubMesh(float hx, float hy, float hz) {
    geometry::SubMesh mesh;
    mesh.name = "Procedural_Box";

    // 24 vértices para 6 caras con normales ortogonales precisas
    // Cara +Y (Superior)
    mesh.vertices.push_back({ {-hx,  hy, -hz}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ { hx,  hy, -hz}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ { hx,  hy,  hz}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ {-hx,  hy,  hz}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} });
    // Cara -Y (Inferior)
    mesh.vertices.push_back({ {-hx, -hy,  hz}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ { hx, -hy,  hz}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ { hx, -hy, -hz}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ {-hx, -hy, -hz}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} });
    // Cara +Z (Frontal)
    mesh.vertices.push_back({ {-hx, -hy,  hz}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ { hx, -hy,  hz}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ { hx,  hy,  hz}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ {-hx,  hy,  hz}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} });
    // Cara -Z (Trasera)
    mesh.vertices.push_back({ { hx, -hy, -hz}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ {-hx, -hy, -hz}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ {-hx,  hy, -hz}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ { hx,  hy, -hz}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f} });
    // Cara +X (Derecha)
    mesh.vertices.push_back({ { hx, -hy,  hz}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ { hx, -hy, -hz}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ { hx,  hy, -hz}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ { hx,  hy,  hz}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} });
    // Cara -X (Izquierda)
    mesh.vertices.push_back({ {-hx, -hy, -hz}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ {-hx, -hy,  hz}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ {-hx,  hy,  hz}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ {-hx,  hy, -hz}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} });

    for (int i = 0; i < 6; ++i) {
        int base = i * 4;
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 3);
        mesh.indices.push_back(base + 0);
    }
    return mesh;
}

PropSpawner::PropSpawner() = default;
PropSpawner::~PropSpawner() = default;

bool PropSpawner::init(const std::string& obstaclesDir) {
    std::vector<std::string> modelFiles = {
        "cone.obj", "barrierRed.obj", "barrierWhite.obj", "barrierWall.obj",
        "box.obj", "debris-tire.obj"
    };

    for (const auto& file : modelFiles) {
        std::string fullPath = (fs::path(obstaclesDir) / file).string();
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
    {
        // Farolas 3D: Mástil vertical, brazo y bombilla
        renderer::MeshGL poleMesh;
        poleMesh.setup(createBoxSubMesh(0.12f, 2.1f, 0.12f));
        m_propMeshes["lampPole"] = std::move(poleMesh);

        renderer::MeshGL armMesh;
        armMesh.setup(createBoxSubMesh(0.10f, 0.08f, 0.70f));
        m_propMeshes["lampArm"] = std::move(armMesh);

        renderer::MeshGL bulbMesh;
        bulbMesh.setup(createBoxSubMesh(0.32f, 0.18f, 0.32f));
        m_propMeshes["lampBulb"] = std::move(bulbMesh);

        // Torres de iluminación de estadio
        renderer::MeshGL towerMesh;
        towerMesh.setup(createBoxSubMesh(0.35f, 5.0f, 0.35f));
        m_propMeshes["stadiumTower"] = std::move(towerMesh);

        // Árboles y pinos procedurales de estilo Low-Poly
        renderer::MeshGL trunkMesh;
        trunkMesh.setup(createBoxSubMesh(0.18f, 0.8f, 0.18f));
        m_propMeshes["treeTrunk"] = std::move(trunkMesh);

        renderer::MeshGL folBottomMesh;
        folBottomMesh.setup(createBoxSubMesh(0.85f, 0.55f, 0.85f));
        m_propMeshes["treeFoliageBottom"] = std::move(folBottomMesh);

        renderer::MeshGL folTopMesh;
        folTopMesh.setup(createBoxSubMesh(0.55f, 0.50f, 0.55f));
        m_propMeshes["treeFoliageTop"] = std::move(folTopMesh);

        // Arco de Meta / Gantry pórtico sobre la pista (dimensiones ampliadas para apoyo perfecto)
        renderer::MeshGL gantryPillar;
        gantryPillar.setup(createBoxSubMesh(0.16f, 2.30f, 0.16f));
        m_propMeshes["gantryPillar"] = std::move(gantryPillar);

        renderer::MeshGL gantryFooting;
        gantryFooting.setup(createBoxSubMesh(0.28f, 0.18f, 0.28f));
        m_propMeshes["gantryFooting"] = std::move(gantryFooting);

        renderer::MeshGL gantryBeam;
        gantryBeam.setup(createBoxSubMesh(2.70f, 0.16f, 0.20f));
        m_propMeshes["gantryBeam"] = std::move(gantryBeam);

        renderer::MeshGL gantryLowerBeam;
        gantryLowerBeam.setup(createBoxSubMesh(2.40f, 0.08f, 0.12f));
        m_propMeshes["gantryLowerBeam"] = std::move(gantryLowerBeam);

        renderer::MeshGL gantrySign;
        gantrySign.setup(createBoxSubMesh(1.65f, 0.35f, 0.06f));
        m_propMeshes["gantrySign"] = std::move(gantrySign);

        renderer::MeshGL gantryLight;
        gantryLight.setup(createBoxSubMesh(0.12f, 0.12f, 0.12f));
        m_propMeshes["gantryLight"] = std::move(gantryLight);

        // Elementos exclusivos para circuitos nocturnos (Neon Arches y Balizas)
        renderer::MeshGL neonArchPillar;
        neonArchPillar.setup(createBoxSubMesh(0.16f, 2.30f, 0.16f));
        m_propMeshes["neonArchPillar"] = std::move(neonArchPillar);

        renderer::MeshGL neonArchBeam;
        neonArchBeam.setup(createBoxSubMesh(2.70f, 0.16f, 0.20f));
        m_propMeshes["neonArchBeam"] = std::move(neonArchBeam);

        renderer::MeshGL neonArchTrimCyan;
        neonArchTrimCyan.setup(createBoxSubMesh(2.70f, 0.06f, 0.24f));
        m_propMeshes["neonArchTrimCyan"] = std::move(neonArchTrimCyan);

        renderer::MeshGL neonArchTrimMagenta;
        neonArchTrimMagenta.setup(createBoxSubMesh(2.70f, 0.06f, 0.24f));
        m_propMeshes["neonArchTrimMagenta"] = std::move(neonArchTrimMagenta);

        renderer::MeshGL neonBeacon;
        neonBeacon.setup(createBoxSubMesh(0.15f, 0.40f, 0.15f));
        m_propMeshes["neonBeacon"] = std::move(neonBeacon);

        renderer::MeshGL neonBeaconTop;
        neonBeaconTop.setup(createBoxSubMesh(0.18f, 0.12f, 0.18f));
        m_propMeshes["neonBeaconTop"] = std::move(neonBeaconTop);

        // Elementos procedurales del Desierto (Cactus saguaro 3D y rocas de arenisca)
        renderer::MeshGL cactusTrunk;
        cactusTrunk.setup(createBoxSubMesh(0.18f, 1.25f, 0.18f));
        m_propMeshes["cactusTrunk"] = std::move(cactusTrunk);

        renderer::MeshGL cactusArmJoint;
        cactusArmJoint.setup(createBoxSubMesh(0.24f, 0.10f, 0.10f));
        m_propMeshes["cactusArmJoint"] = std::move(cactusArmJoint);

        renderer::MeshGL cactusArmSpire;
        cactusArmSpire.setup(createBoxSubMesh(0.12f, 0.50f, 0.12f));
        m_propMeshes["cactusArmSpire"] = std::move(cactusArmSpire);

        renderer::MeshGL desertRock;
        desertRock.setup(createBoxSubMesh(0.65f, 0.55f, 0.65f));
        m_propMeshes["desertRock"] = std::move(desertRock);

        renderer::MeshGL desertMesaRock;
        desertMesaRock.setup(createBoxSubMesh(1.20f, 0.85f, 1.10f));
        m_propMeshes["desertMesaRock"] = std::move(desertMesaRock);

        renderer::MeshGL desertArchPillar;
        desertArchPillar.setup(createBoxSubMesh(0.40f, 2.50f, 0.40f));
        m_propMeshes["desertArchPillar"] = std::move(desertArchPillar);

        renderer::MeshGL desertArchBeam;
        desertArchBeam.setup(createBoxSubMesh(2.90f, 0.35f, 0.45f));
        m_propMeshes["desertArchBeam"] = std::move(desertArchBeam);

        // Elementos procedurales del Bioma Espacial (Warp Rings, Satélites y Balizas Galácticas)
        renderer::MeshGL spaceWarpPillar;
        spaceWarpPillar.setup(createBoxSubMesh(0.18f, 2.40f, 0.18f));
        m_propMeshes["spaceWarpPillar"] = std::move(spaceWarpPillar);

        renderer::MeshGL spaceWarpBeam;
        spaceWarpBeam.setup(createBoxSubMesh(2.65f, 0.15f, 0.20f));
        m_propMeshes["spaceWarpBeam"] = std::move(spaceWarpBeam);

        renderer::MeshGL spaceWarpTrimCyan;
        spaceWarpTrimCyan.setup(createBoxSubMesh(2.65f, 0.08f, 0.24f));
        m_propMeshes["spaceWarpTrimCyan"] = std::move(spaceWarpTrimCyan);

        renderer::MeshGL spaceWarpTrimMagenta;
        spaceWarpTrimMagenta.setup(createBoxSubMesh(2.65f, 0.08f, 0.24f));
        m_propMeshes["spaceWarpTrimMagenta"] = std::move(spaceWarpTrimMagenta);

        renderer::MeshGL satelliteBody;
        satelliteBody.setup(createBoxSubMesh(0.40f, 0.40f, 0.40f));
        m_propMeshes["satelliteBody"] = std::move(satelliteBody);

        renderer::MeshGL satellitePanel;
        satellitePanel.setup(createBoxSubMesh(1.10f, 0.04f, 0.35f));
        m_propMeshes["satellitePanel"] = std::move(satellitePanel);

        renderer::MeshGL satelliteAntenna;
        satelliteAntenna.setup(createBoxSubMesh(0.04f, 0.70f, 0.04f));
        m_propMeshes["satelliteAntenna"] = std::move(satelliteAntenna);

        renderer::MeshGL spaceBeacon;
        spaceBeacon.setup(createBoxSubMesh(0.16f, 0.45f, 0.16f));
        m_propMeshes["spaceBeacon"] = std::move(spaceBeacon);

        renderer::MeshGL spaceBeaconTop;
        spaceBeaconTop.setup(createBoxSubMesh(0.20f, 0.14f, 0.20f));
        m_propMeshes["spaceBeaconTop"] = std::move(spaceBeaconTop);
    }

    m_initialized = !m_propMeshes.empty();
    return m_initialized;
}

void PropSpawner::addProp(const std::string& type, const glm::vec3& pos, float rotDeg, const glm::vec3& scale) {
    PropInstance inst;
    inst.type = type;
    inst.position = pos;
    inst.rotationDegrees = rotDeg;
    inst.scale = scale;

    glm::mat4 mat = glm::translate(glm::mat4(1.0f), pos);
    mat = glm::rotate(mat, glm::radians(rotDeg), glm::vec3(0.0f, 1.0f, 0.0f));
    mat = glm::scale(mat, scale);
    inst.modelMatrix = mat;

    // AABBs ajustados con máxima fidelidad a la malla visual (sin buffers sobredimensionados)
    if (type == "cone") {
        // Cono real: radio visual 0.24m, altura 0.60m
        float r = 0.24f * scale.x;
        inst.aabbMin = glm::vec3(pos.x - r, 0.0f, pos.z - r);
        inst.aabbMax = glm::vec3(pos.x + r, 0.62f * scale.y, pos.z + r);
    } else if (type == "barrierWall") {
        float hx = 0.50f * scale.x;
        float hz = 0.14f * scale.z;
        if (std::abs(rotDeg - 90.0f) < 5.0f || std::abs(rotDeg - 270.0f) < 5.0f) {
            std::swap(hx, hz);
        }
        inst.aabbMin = glm::vec3(pos.x - hx, 0.0f, pos.z - hz);
        inst.aabbMax = glm::vec3(pos.x + hx, 0.55f * scale.y, pos.z + hz);
    } else if (type == "barrierRed" || type == "barrierWhite") {
        float hx = 0.32f * scale.x;
        float hz = 0.16f * scale.z;
        inst.aabbMin = glm::vec3(pos.x - hx, 0.0f, pos.z - hz);
        inst.aabbMax = glm::vec3(pos.x + hx, 0.45f * scale.y, pos.z + hz);
    } else if (type == "box") {
        float hs = 0.35f * scale.x;
        inst.aabbMin = glm::vec3(pos.x - hs, 0.0f, pos.z - hs);
        inst.aabbMax = glm::vec3(pos.x + hs, 0.70f * scale.y, pos.z + hs);
    } else if (type == "debris-tire") {
        float r = 0.28f * scale.x;
        inst.aabbMin = glm::vec3(pos.x - r, 0.0f, pos.z - r);
        inst.aabbMax = glm::vec3(pos.x + r, 0.55f * scale.y, pos.z + r);
    } else if (type == "lampPole") {
        inst.aabbMin = glm::vec3(pos.x - 0.15f, 0.0f, pos.z - 0.15f);
        inst.aabbMax = glm::vec3(pos.x + 0.15f, 4.40f * scale.y, pos.z + 0.15f);
    } else if (type == "treeTrunk") {
        inst.aabbMin = glm::vec3(pos.x - 0.20f, 0.0f, pos.z - 0.20f);
        inst.aabbMax = glm::vec3(pos.x + 0.20f, 1.80f * scale.y, pos.z + 0.20f);
    } else if (type == "stadiumTower") {
        inst.aabbMin = glm::vec3(pos.x - 0.40f, 0.0f, pos.z - 0.40f);
        inst.aabbMax = glm::vec3(pos.x + 0.40f, 10.0f * scale.y, pos.z + 0.40f);
    } else if (type == "gantryPillar" || type == "neonArchPillar") {
        inst.aabbMin = glm::vec3(pos.x - 0.18f, 0.0f, pos.z - 0.18f);
        inst.aabbMax = glm::vec3(pos.x + 0.18f, 4.60f * scale.y, pos.z + 0.18f);
    } else if (type == "neonBeacon") {
        inst.aabbMin = glm::vec3(pos.x - 0.16f, 0.0f, pos.z - 0.16f);
        inst.aabbMax = glm::vec3(pos.x + 0.16f, 0.85f * scale.y, pos.z + 0.16f);
    } else if (type == "cactusTrunk") {
        inst.aabbMin = glm::vec3(pos.x - 0.22f, 0.0f, pos.z - 0.22f);
        inst.aabbMax = glm::vec3(pos.x + 0.22f, 2.60f * scale.y, pos.z + 0.22f);
    } else if (type == "desertRock") {
        inst.aabbMin = glm::vec3(pos.x - 0.65f * scale.x, 0.0f, pos.z - 0.65f * scale.z);
        inst.aabbMax = glm::vec3(pos.x + 0.65f * scale.x, 1.10f * scale.y, pos.z + 0.65f * scale.z);
    } else if (type == "desertMesaRock") {
        inst.aabbMin = glm::vec3(pos.x - 1.20f * scale.x, 0.0f, pos.z - 1.20f * scale.z);
        inst.aabbMax = glm::vec3(pos.x + 1.20f * scale.x, 1.70f * scale.y, pos.z + 1.20f * scale.z);
    } else if (type == "desertArchPillar") {
        inst.aabbMin = glm::vec3(pos.x - 0.42f, 0.0f, pos.z - 0.42f);
        inst.aabbMax = glm::vec3(pos.x + 0.42f, 5.0f * scale.y, pos.z + 0.42f);
    } else if (type == "spaceWarpPillar") {
        inst.aabbMin = glm::vec3(pos.x - 0.20f, 0.0f, pos.z - 0.20f);
        inst.aabbMax = glm::vec3(pos.x + 0.20f, 4.80f * scale.y, pos.z + 0.20f);
    } else if (type == "spaceBeacon") {
        inst.aabbMin = glm::vec3(pos.x - 0.18f, 0.0f, pos.z - 0.18f);
        inst.aabbMax = glm::vec3(pos.x + 0.18f, 0.90f * scale.y, pos.z + 0.18f);
    } else {
        // Elementos suspendidos (vigas a 4m de altura, luces, letrero de meta):
        glm::vec3 halfSize = scale * 0.5f;
        inst.aabbMin = pos - halfSize;
        inst.aabbMax = pos + halfSize;
    }

    m_props.push_back(inst);
}

void PropSpawner::generateForGrid(const TrackGrid& grid, bool isNight, bool isDesert, bool isSpace) {
    m_props.clear();
    m_lampPositions.clear();

    float halfTile = TrackGrid::TILE_SIZE * 0.5f;
    int propIndex = 0;

    // Recopilar losas de la pista
    std::vector<const TileData*> straightTiles;
    const TileData* startFinishTile = nullptr;

    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::START_FINISH) {
            startFinishTile = &tile;
        } else if (tile.type == TileType::ROAD_STRAIGHT) {
            straightTiles.push_back(&tile);
        }
    }

    if (isSpace) {
        // =========================================================
        // GENERACION BIOMA ESPACIAL: Rainbow Cosmic Highway
        // =========================================================

        // 1. Gran Portal de Lanzamiento Orbital en la Salida / Meta
        if (startFinishTile) {
            bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
            float beamRot = isHorizontal ? 90.0f : 0.0f;
            float pillarOffset = 2.45f;
            glm::vec3 pillarAOffset = isHorizontal ? glm::vec3(0.0f, 0.0f,  pillarOffset) : glm::vec3( pillarOffset, 0.0f, 0.0f);
            glm::vec3 pillarBOffset = isHorizontal ? glm::vec3(0.0f, 0.0f, -pillarOffset) : glm::vec3(-pillarOffset, 0.0f, 0.0f);

            glm::vec3 posA = startFinishTile->worldPos + pillarAOffset;
            glm::vec3 posB = startFinishTile->worldPos + pillarBOffset;

            addProp("spaceWarpPillar", glm::vec3(posA.x, 2.40f, posA.z), beamRot);
            addProp("spaceWarpPillar", glm::vec3(posB.x, 2.40f, posB.z), beamRot);

            glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 4.80f, 0.0f);
            addProp("spaceWarpBeam", beamPos, beamRot);
            addProp("spaceWarpTrimCyan", beamPos + glm::vec3(0.0f, 0.18f, 0.0f), beamRot);
            addProp("spaceWarpTrimMagenta", beamPos - glm::vec3(0.0f, 0.18f, 0.0f), beamRot);
        }

        // 2. Anillos de Hipervelocidad (Warp Rings) a lo largo de las grandes rectas
        for (size_t i = 0; i < straightTiles.size(); ++i) {
            const auto* tile = straightTiles[i];
            bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
            float beamRot = isHorizontal ? 90.0f : 0.0f;

            // Cada 6 losas de recta, desplegar un pórtico Warp Ring completo
            if (i % 6 == 2) {
                float pillarOffset = 2.40f;
                glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
                glm::vec3 posA = tile->worldPos + sideDir * pillarOffset;
                glm::vec3 posB = tile->worldPos - sideDir * pillarOffset;

                addProp("spaceWarpPillar", glm::vec3(posA.x, 2.40f, posA.z), beamRot);
                addProp("spaceWarpPillar", glm::vec3(posB.x, 2.40f, posB.z), beamRot);

                glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 4.70f, 0.0f);
                addProp("spaceWarpBeam", beamPos, beamRot);
                if (i % 12 == 2) {
                    addProp("spaceWarpTrimCyan", beamPos + glm::vec3(0.0f, 0.15f, 0.0f), beamRot);
                } else {
                    addProp("spaceWarpTrimMagenta", beamPos + glm::vec3(0.0f, 0.15f, 0.0f), beamRot);
                }
            }

            // Satélites flotantes de comunicación orbital a los lados
            if (i % 8 == 4) {
                float sideSign = (i % 16 == 4) ? 1.0f : -1.0f;
                glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);
                glm::vec3 satPos = tile->worldPos + sideDir * 5.5f + glm::vec3(0.0f, 3.8f, 0.0f);
                float satRot = static_cast<float>((i * 67) % 360);

                addProp("satelliteBody", satPos, satRot);
                addProp("satellitePanel", satPos, satRot);
                addProp("satelliteAntenna", satPos + glm::vec3(0.0f, 0.55f, 0.0f), satRot);
            }
        }

        // 3. Balizas de Navegación Estelar en los bordes de todas las curvas
        for (const auto& tile : grid.getAllTiles()) {
            if (tile.type == TileType::ROAD_CORNER_SMALL || tile.type == TileType::ROAD_CORNER_LARGE) {
                float rotRad = glm::radians(tile.rotationDegrees);
                glm::vec3 outerDir(std::cos(rotRad + 0.785f), 0.0f, std::sin(rotRad + 0.785f));

                glm::vec3 bPos = tile.worldPos + outerDir * 2.30f;
                addProp("spaceBeacon", bPos, tile.rotationDegrees);
                addProp("spaceBeaconTop", bPos + glm::vec3(0.0f, 0.45f, 0.0f), tile.rotationDegrees);
            }
        }

        std::cout << "[OK] [PropSpawner] Total de elementos generados: "
                  << m_props.size() << " (Portales Warp hiperespaciales, satelites orbitales y balizas estelares)." << std::endl;
        return;
    }

    if (isDesert) {
        // =========================================================
        // GENERACION BIOMA DESIERTO: Rally Dakar / Valle de Dunas
        // =========================================================

        // 1. Arco Natural de Piedra y Arenisca en la Salida / Meta
        if (startFinishTile) {
            bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
            float beamRot = isHorizontal ? 90.0f : 0.0f;
            float pillarOffset = 2.45f;
            glm::vec3 pillarAOffset = isHorizontal ? glm::vec3(0.0f, 0.0f,  pillarOffset) : glm::vec3( pillarOffset, 0.0f, 0.0f);
            glm::vec3 pillarBOffset = isHorizontal ? glm::vec3(0.0f, 0.0f, -pillarOffset) : glm::vec3(-pillarOffset, 0.0f, 0.0f);

            glm::vec3 posA = startFinishTile->worldPos + pillarAOffset;
            glm::vec3 posB = startFinishTile->worldPos + pillarBOffset;

            // Pilares de piedra arenisca natural
            addProp("desertArchPillar", glm::vec3(posA.x, 2.50f, posA.z), beamRot);
            addProp("desertArchPillar", glm::vec3(posB.x, 2.50f, posB.z), beamRot);

            // Viga superior de roca tallada conectando los pilares
            glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 5.10f, 0.0f);
            addProp("desertArchBeam", beamPos, beamRot);

            // Monolitos protectores en las bases del arco
            addProp("desertRock", posA + glm::vec3(0.8f, 0.55f, 0.0f), 25.0f, glm::vec3(1.1f));
            addProp("desertRock", posB - glm::vec3(0.8f, -0.55f, 0.0f), 65.0f, glm::vec3(1.1f));
        }

        // 2. Obstaculos y decoraciones en curvas: Barreras de neumaticos, conos y rocas de arenisca
        for (const auto& tile : grid.getAllTiles()) {
            if (tile.type == TileType::ROAD_CORNER_SMALL || tile.type == TileType::ROAD_CORNER_LARGE) {
                float rotRad = glm::radians(tile.rotationDegrees);
                glm::vec3 outerDir(std::cos(rotRad + 0.785f), 0.0f, std::sin(rotRad + 0.785f));
                
                // Conos / neumaticos de aviso
                addProp("debris-tire", tile.worldPos + outerDir * 2.2f, tile.rotationDegrees, glm::vec3(1.3f));
                addProp("cone", tile.worldPos - outerDir * 1.8f, tile.rotationDegrees + 45.0f, glm::vec3(1.1f));

                // Formaciones rocosas naturales de arenisca a los bordes
                addProp("desertRock", tile.worldPos + outerDir * 3.8f, tile.rotationDegrees + 30.0f, glm::vec3(1.3f, 1.2f, 1.3f));
                addProp("desertMesaRock", tile.worldPos + outerDir * 5.8f, tile.rotationDegrees + 75.0f, glm::vec3(1.4f, 1.5f, 1.2f));
            }
        }

        // 3. Bosques de Cactus Saguaro 3D y Monolitos a lo largo de las rectas
        for (size_t i = 0; i < straightTiles.size(); ++i) {
            const auto* tile = straightTiles[i];
            bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
            float sideSign = (i % 2 == 0) ? 1.0f : -1.0f;
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);

            // Generar Cactus Saguaro 3D cada 2 losas
            if (i % 2 == 0) {
                float dist = 3.6f + (i % 3) * 0.8f;
                glm::vec3 cPos = tile->worldPos + sideDir * dist;
                float cactusRot = static_cast<float>((i * 53) % 360);

                // Tronco principal
                addProp("cactusTrunk", cPos + glm::vec3(0.0f, 1.25f, 0.0f), cactusRot);
                // Brazo izquierdo (ramificacion horizontal + vertical hacia arriba)
                float rad = glm::radians(cactusRot);
                glm::vec3 leftDir(-std::sin(rad), 0.0f, std::cos(rad));
                glm::vec3 rightDir(std::sin(rad), 0.0f, -std::cos(rad));

                addProp("cactusArmJoint", cPos + leftDir * 0.28f + glm::vec3(0.0f, 1.55f, 0.0f), cactusRot);
                addProp("cactusArmSpire", cPos + leftDir * 0.42f + glm::vec3(0.0f, 1.95f, 0.0f), cactusRot);

                // Brazo derecho
                addProp("cactusArmJoint", cPos + rightDir * 0.28f + glm::vec3(0.0f, 1.15f, 0.0f), cactusRot);
                addProp("cactusArmSpire", cPos + rightDir * 0.42f + glm::vec3(0.0f, 1.55f, 0.0f), cactusRot);

                // Roca pequena en la base del cactus
                if (i % 4 == 0) {
                    addProp("desertRock", cPos + sideDir * 0.9f + glm::vec3(0.0f, 0.5f, 0.0f), cactusRot + 45.0f, glm::vec3(0.7f));
                }
            } else if (i % 3 == 0) {
                // Rocas de arenisca naturales en los laterales de la pista
                float dist = 4.2f + (i % 2) * 1.2f;
                glm::vec3 rockPos = tile->worldPos - sideDir * dist;
                addProp("desertRock", rockPos + glm::vec3(0.0f, 0.45f, 0.0f), static_cast<float>((i * 71) % 360), glm::vec3(1.1f, 0.8f, 1.0f));
            }
        }

        std::cout << "[OK] [PropSpawner] Total de elementos generados: "
                  << m_props.size() << " (Arco natural de arenisca, cactus saguaro 3D y rocas de arenisca del desierto)." << std::endl;
        return;
    }

    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::ROAD_STRAIGHT) {
            straightTiles.push_back(&tile);
        } else if (tile.type == TileType::START_FINISH) {
            startFinishTile = &tile;
        }
    }

    // 1. Arco de Meta / Gantry Monumental sobre la línea de meta
    if (startFinishTile != nullptr) {
        bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
        glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        // Cuando isHorizontal es true, el carril corre por X; los bordes están en Z.
        // Dado que la viga sin rotar se extiende en X, rotarla 90 grados la orienta en Z, conectando ambos pilares
        float beamRot = isHorizontal ? 90.0f : 0.0f;
        float signRot = isHorizontal ? 90.0f : 0.0f;

        // Pilares izquierdo y derecho anclados en el césped a cada lado del carril (4.7m de luz)
        glm::vec3 leftPillar = startFinishTile->worldPos + sideDir * 2.35f + glm::vec3(0.0f, 2.30f, 0.0f);
        glm::vec3 rightPillar = startFinishTile->worldPos - sideDir * 2.35f + glm::vec3(0.0f, 2.30f, 0.0f);
        addProp("gantryPillar", leftPillar, 0.0f, glm::vec3(1.0f));
        addProp("gantryPillar", rightPillar, 0.0f, glm::vec3(1.0f));

        // Zapatas de hormigón en la base de los pilares
        glm::vec3 leftFoot = startFinishTile->worldPos + sideDir * 2.35f + glm::vec3(0.0f, 0.18f, 0.0f);
        glm::vec3 rightFoot = startFinishTile->worldPos - sideDir * 2.35f + glm::vec3(0.0f, 0.18f, 0.0f);
        addProp("gantryFooting", leftFoot, 0.0f, glm::vec3(1.0f));
        addProp("gantryFooting", rightFoot, 0.0f, glm::vec3(1.0f));

        // Viga transversal superior en celosía descansando sólidamente sobre los pilares a 4.76m de altura (5.4m de largo)
        glm::vec3 beamPos = startFinishTile->worldPos + glm::vec3(0.0f, 4.76f, 0.0f);
        addProp("gantryBeam", beamPos, beamRot, glm::vec3(1.0f));

        // Viga inferior de soporte y unión a 3.90m
        glm::vec3 lowerBeamPos = startFinishTile->worldPos + glm::vec3(0.0f, 3.90f, 0.0f);
        addProp("gantryLowerBeam", lowerBeamPos, beamRot, glm::vec3(1.0f));

        // Letrero luminoso de START / FINISH en el centro entre ambas vigas
        glm::vec3 signPos = startFinishTile->worldPos + glm::vec3(0.0f, 4.33f, 0.0f);
        addProp("gantrySign", signPos, signRot, glm::vec3(1.0f));

        // 5 Luces de semáforo de salida de carrera F1 suspendidas bajo la estructura
        for (int i = -2; i <= 2; ++i) {
            glm::vec3 lightPos = startFinishTile->worldPos + sideDir * (i * 0.60f) + glm::vec3(0.0f, 3.72f, 0.0f);
            addProp("gantryLight", lightPos, beamRot, glm::vec3(1.0f));
        }
    }

    // 2. Generar Farolas 3D Ampliamente Espaciadas (Mínimo 28m de separación entre postes)
    if (!straightTiles.empty()) {
        const float MIN_LAMP_DIST = 28.0f; // Al menos 28 metros entre farolas (7 losas de separación)
        const size_t MAX_LAMPS = 6;        // Máximo 6 farolas estratégicas para no sobrecargar de luz

        std::vector<const TileData*> selectedLamps;
        size_t stride = std::max<size_t>(1, straightTiles.size() / 16);
        for (size_t i = 0; i < straightTiles.size() && selectedLamps.size() < MAX_LAMPS; i += stride) {
            const auto* cand = straightTiles[i];
            bool tooClose = false;
            for (const auto* existing : selectedLamps) {
                if (glm::distance(cand->worldPos, existing->worldPos) < MIN_LAMP_DIST) {
                    tooClose = true;
                    break;
                }
            }
            if (!tooClose) {
                selectedLamps.push_back(cand);
            }
        }

        for (size_t i = 0; i < selectedLamps.size(); ++i) {
            const auto* tile = selectedLamps[i];
            bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
            float sideSign = (i % 2 == 0) ? 1.0f : -1.0f;
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);

            // Mástil vertical en el césped
            glm::vec3 poleBase = tile->worldPos + sideDir * (halfTile * 1.15f);
            glm::vec3 poleCenter = poleBase + glm::vec3(0.0f, 2.1f, 0.0f);
            addProp("lampPole", poleCenter, 0.0f, glm::vec3(1.0f));

            // Base de protección con neumáticos
            addProp("debris-tire", poleBase + glm::vec3(0.0f, 0.25f, 0.0f), 0.0f, glm::vec3(1.0f));

            // Brazo en voladizo inclinado a 4.2m
            glm::vec3 armPos = poleBase + glm::vec3(0.0f, 4.2f, 0.0f) - sideDir * 0.55f;
            float armRot = isHorizontal ? (sideSign > 0 ? 180.0f : 0.0f) : (sideSign > 0 ? 270.0f : 90.0f);
            addProp("lampArm", armPos, armRot, glm::vec3(1.0f));

            // Foco / Luminaria incandescente suspendida sobre la pista
            glm::vec3 bulbPos = poleBase + glm::vec3(0.0f, 4.1f, 0.0f) - sideDir * 1.15f;
            addProp("lampBulb", bulbPos, 0.0f, glm::vec3(1.0f));

            m_lampPositions.push_back(bulbPos);
        }
    }

    // 3. Arcos de Neón Luminosos Exclusivos para Circuitos Nocturnos
    if (isNight && straightTiles.size() >= 6) {
        size_t idx1 = straightTiles.size() / 3;
        size_t idx2 = (straightTiles.size() * 2) / 3;
        const TileData* neonTiles[2] = { straightTiles[idx1], straightTiles[idx2] };
        const char* trims[2] = { "neonArchTrimCyan", "neonArchTrimMagenta" };

        for (int k = 0; k < 2; ++k) {
            const auto* nTile = neonTiles[k];
            bool isH = (nTile->rotationDegrees == 90.0f || nTile->rotationDegrees == 270.0f);
            glm::vec3 sDir = isH ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
            float bRot = isH ? 90.0f : 0.0f;

            glm::vec3 lp = nTile->worldPos + sDir * 2.35f + glm::vec3(0.0f, 2.30f, 0.0f);
            glm::vec3 rp = nTile->worldPos - sDir * 2.35f + glm::vec3(0.0f, 2.30f, 0.0f);
            addProp("neonArchPillar", lp, 0.0f, glm::vec3(1.0f));
            addProp("neonArchPillar", rp, 0.0f, glm::vec3(1.0f));

            glm::vec3 bp = nTile->worldPos + glm::vec3(0.0f, 4.60f, 0.0f);
            addProp("neonArchBeam", bp, bRot, glm::vec3(1.0f));

            glm::vec3 tp = nTile->worldPos + glm::vec3(0.0f, 4.40f, 0.0f);
            addProp(trims[k], tp, bRot, glm::vec3(1.0f));
        }
    }

    // 4. Elementos de Pista (Curvas, Rectas, Meta)
    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::EMPTY) continue;

        // Curvas:
        if (tile.type == TileType::ROAD_CORNER_SMALL) {
            float rot = tile.rotationDegrees;
            glm::vec3 cornerPos = tile.worldPos;

            float xSign = (rot == 0.0f || rot == 90.0f) ? 1.0f : -1.0f;
            float zSign = (rot == 0.0f || rot == 270.0f) ? 1.0f : -1.0f;

            if (isNight) {
                // Baliza luminosa en el ápice interior de la curva
                glm::vec3 beaconPos = cornerPos + glm::vec3(-xSign * halfTile * 0.52f, 0.0f, -zSign * halfTile * 0.52f);
                addProp("neonBeacon", beaconPos, 0.0f, glm::vec3(1.0f));
                addProp("neonBeaconTop", beaconPos + glm::vec3(0.0f, 0.45f, 0.0f), 0.0f, glm::vec3(1.0f));

                // Doble pila de neumáticos de seguridad en la escapatoria exterior
                glm::vec3 tirePos1 = cornerPos + glm::vec3(xSign * (halfTile * 1.05f), 0.25f, zSign * (halfTile * 1.05f));
                glm::vec3 tirePos2 = cornerPos + glm::vec3(xSign * (halfTile * 1.05f), 0.65f, zSign * (halfTile * 1.05f));
                addProp("debris-tire", tirePos1, rot + 45.0f, glm::vec3(1.2f));
                addProp("debris-tire", tirePos2, rot + 45.0f, glm::vec3(1.0f));

                glm::vec3 barPos = cornerPos + glm::vec3(xSign * halfTile * 0.88f, 0.0f, zSign * halfTile * 0.40f);
                addProp("barrierRed", barPos, rot + 30.0f, glm::vec3(1.1f));
            } else {
                // Modo diurno: Barreras bicolor (Rojo/Blanco), neumáticos de seguridad y conos de ápice
                glm::vec3 barrier1Pos = cornerPos + glm::vec3(xSign * halfTile * 0.88f, 0.0f, zSign * halfTile * 0.40f);
                addProp("barrierRed", barrier1Pos, rot + 30.0f, glm::vec3(1.1f));

                glm::vec3 barrier2Pos = cornerPos + glm::vec3(xSign * halfTile * 0.40f, 0.0f, zSign * halfTile * 0.88f);
                addProp("barrierWhite", barrier2Pos, rot + 60.0f, glm::vec3(1.1f));

                glm::vec3 tirePos = cornerPos + glm::vec3(xSign * (halfTile * 1.05f), 0.25f, zSign * (halfTile * 1.05f));
                addProp("debris-tire", tirePos, rot + 45.0f, glm::vec3(1.1f));

                glm::vec3 apexPos = cornerPos + glm::vec3(-xSign * halfTile * 0.52f, 0.0f, -zSign * halfTile * 0.52f);
                addProp("cone", apexPos, 0.0f, glm::vec3(1.0f));
            }
        }

        // Recta de Meta: Muros de boxes y cajas de equipo
        if (tile.type == TileType::START_FINISH) {
            bool isHorizontal = (tile.rotationDegrees == 90.0f || tile.rotationDegrees == 270.0f);
            glm::vec3 wallOffset1 = isHorizontal ? glm::vec3(0.0f, 0.0f, -halfTile * 0.95f) : glm::vec3(-halfTile * 0.95f, 0.0f, 0.0f);
            glm::vec3 wallOffset2 = isHorizontal ? glm::vec3(0.0f, 0.0f,  halfTile * 0.95f) : glm::vec3( halfTile * 0.95f, 0.0f, 0.0f);

            addProp("barrierWall", tile.worldPos + wallOffset1, tile.rotationDegrees, glm::vec3(1.0f));
            addProp("barrierWall", tile.worldPos + wallOffset2, tile.rotationDegrees, glm::vec3(1.0f));

            glm::vec3 boxPos = tile.worldPos + wallOffset2 + (isHorizontal ? glm::vec3(1.2f, 0.0f, 0.8f) : glm::vec3(0.8f, 0.0f, 1.2f));
            addProp("box", boxPos, 15.0f, glm::vec3(1.0f));
        }

        // Rectas del circuito: Conos y delimitadores a los lados
        if (tile.type == TileType::ROAD_STRAIGHT) {
            bool isHorizontal = (tile.rotationDegrees == 90.0f || tile.rotationDegrees == 270.0f);
            float lateralDist = halfTile * 0.92f;

            if ((tile.col + tile.row) % 3 == 0) {
                float side = ((propIndex++) % 2 == 0) ? 1.0f : -1.0f;
                glm::vec3 propOffset = isHorizontal ? glm::vec3(0.0f, 0.0f, side * lateralDist) : glm::vec3(side * lateralDist, 0.0f, 0.0f);

                if (propIndex % 4 == 0) {
                    addProp("barrierRed", tile.worldPos + propOffset, tile.rotationDegrees, glm::vec3(1.0f));
                } else if (propIndex % 4 == 2) {
                    addProp("barrierWhite", tile.worldPos + propOffset, tile.rotationDegrees, glm::vec3(1.0f));
                } else {
                    addProp("cone", tile.worldPos + propOffset, 0.0f, glm::vec3(1.0f));
                }
            }

            if ((tile.col * 7 + tile.row * 13) % 19 == 0) {
                float decorSide = ((tile.col % 2 == 0) ? 1.0f : -1.0f) * (halfTile * 1.45f);
                glm::vec3 decorOffset = isHorizontal ? glm::vec3(0.0f, 0.0f, decorSide) : glm::vec3(decorSide, 0.0f, 0.0f);
                if ((tile.col + tile.row) % 2 == 0) {
                    addProp("box", tile.worldPos + decorOffset, 25.0f, glm::vec3(0.9f));
                } else {
                    addProp("debris-tire", tile.worldPos + decorOffset + glm::vec3(0.0f, 0.25f, 0.0f), 0.0f, glm::vec3(1.0f));
                }
            }
        }
    }

    // 5. Bosques y Pinos Low-Poly en las Áreas Abiertas de Césped (Infield & Outfield)
    int treeMod = isNight ? 6 : 3;
    for (int r = 1; r < grid.getRows() - 1; ++r) {
        for (int c = 1; c < grid.getCols() - 1; ++c) {
            const auto* current = grid.getTile(c, r);
            if (current && current->type == TileType::EMPTY) {
                bool nearRoad = false;
                for (int dr = -1; dr <= 1 && !nearRoad; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        const auto* n = grid.getTile(c + dc, r + dr);
                        if (n && n->type != TileType::EMPTY) {
                            nearRoad = true;
                            break;
                        }
                    }
                }

                if (!nearRoad && (c * 13 + r * 17) % treeMod == 0) {
                    glm::vec3 treePos = current->worldPos + glm::vec3(
                        ((c % 3) - 1.0f) * 0.8f,
                        0.0f,
                        ((r % 3) - 1.0f) * 0.8f
                    );
                    float treeRot = static_cast<float>((c * 37 + r * 53) % 360);

                    addProp("treeTrunk", treePos + glm::vec3(0.0f, 0.8f, 0.0f), treeRot, glm::vec3(1.0f));
                    addProp("treeFoliageBottom", treePos + glm::vec3(0.0f, 1.85f, 0.0f), treeRot + 15.0f, glm::vec3(1.0f));
                    addProp("treeFoliageTop", treePos + glm::vec3(0.0f, 2.70f, 0.0f), treeRot + 30.0f, glm::vec3(1.0f));
                }
            }
        }
    }

    // 6. Decoración del Fondo y Entorno de Estadio Perimetral (Torres Floodlight y Muros)
    float halfW = (grid.getCols() * TrackGrid::TILE_SIZE) * 0.5f + 7.0f;
    float halfD = (grid.getRows() * TrackGrid::TILE_SIZE) * 0.5f + 7.0f;

    // 4 Torres de Iluminación en las esquinas
    addProp("stadiumTower", glm::vec3(-halfW, 5.0f, -halfD), 45.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3( halfW, 5.0f, -halfD), -45.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3( halfW, 5.0f,  halfD), 45.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3(-halfW, 5.0f,  halfD), -45.0f, glm::vec3(1.0f));

    // Torres intermedias en los laterales
    addProp("stadiumTower", glm::vec3(0.0f, 5.0f, -halfD), 0.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3(0.0f, 5.0f,  halfD), 0.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3(-halfW, 5.0f, 0.0f), 90.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3( halfW, 5.0f, 0.0f), 90.0f, glm::vec3(1.0f));

    // Vallas de contención de estadio alrededor de la pista
    for (float x = -halfW + 12.0f; x <= halfW - 12.0f; x += 12.0f) {
        addProp("barrierWall", glm::vec3(x, 0.0f, -halfD), 0.0f, glm::vec3(2.5f));
        addProp("barrierWall", glm::vec3(x, 0.0f,  halfD), 0.0f, glm::vec3(2.5f));
    }
    for (float z = -halfD + 12.0f; z <= halfD - 12.0f; z += 12.0f) {
        addProp("barrierWall", glm::vec3(-halfW, 0.0f, z), 90.0f, glm::vec3(2.5f));
        addProp("barrierWall", glm::vec3( halfW, 0.0f, z), 90.0f, glm::vec3(2.5f));
    }

    std::cout << "[OK] [PropSpawner] Total de elementos generados: "
              << m_props.size() << " (Arco monumental, farolas, torres, "
              << (isNight ? "arcos de neon y balizas nocturnas)." : "pinos y defensas diurnas).") << std::endl;
}

void PropSpawner::render(const renderer::Shader& shader) const {
    if (!m_initialized) return;

    for (const auto& prop : m_props) {
        auto it = m_propMeshes.find(prop.type);
        if (it != m_propMeshes.end()) {
            shader.setMat4("u_Model", prop.modelMatrix);

            if (prop.type == "lampBulb") {
                // Luminaria incandescente de vapor de sodio (ámbar brillante)
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.88f, 0.30f, 1.0f));
            } else if (prop.type == "neonArchTrimCyan") {
                // Ribete emisivo neon cian brillante
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.0f, 0.95f, 1.0f, 1.0f));
            } else if (prop.type == "neonArchTrimMagenta") {
                // Ribete emisivo neon magenta electrico
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.12f, 0.72f, 1.0f));
            } else if (prop.type == "neonBeaconTop") {
                // Cabezal luminoso ambar/naranja de baliza de curva
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.50f, 0.05f, 1.0f));
            } else if (prop.type == "neonBeacon") {
                // Base de baliza de curva
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.20f, 0.20f, 0.22f, 1.0f));
            } else if (prop.type == "gantryLight") {
                // Luces de salida de carrera rojas
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.18f, 0.18f, 1.0f));
            } else if (prop.type == "gantryFooting") {
                // Zapata de hormigon gris medio
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.48f, 0.50f, 0.54f, 1.0f));
            } else if (prop.type == "neonArchPillar" || prop.type == "neonArchBeam") {
                // Estructura metalica oscura para arcos de neon
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.12f, 0.14f, 0.18f, 1.0f));
            } else if (prop.type == "lampPole" || prop.type == "lampArm" || prop.type == "gantryPillar" || prop.type == "gantryBeam" || prop.type == "gantryLowerBeam") {
                // Estructura metálica industrial gris oscuro
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.24f, 0.26f, 0.30f, 1.0f));
            } else if (prop.type == "gantrySign") {
                // Cartel de Meta rojo vibrante de carreras
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.92f, 0.18f, 0.18f, 1.0f));
            } else if (prop.type == "stadiumTower") {
                // Estructura de torre de estadio gris metalizado
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.38f, 0.42f, 0.48f, 1.0f));
            } else if (prop.type == "treeTrunk") {
                // Tronco de madera marrón oscuro
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.42f, 0.28f, 0.16f, 1.0f));
            } else if (prop.type == "treeFoliageBottom") {
                // Follaje de pino verde bosque
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.18f, 0.48f, 0.22f, 1.0f));
            } else if (prop.type == "treeFoliageTop") {
                // Follaje de pino verde vivo superior
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.25f, 0.58f, 0.28f, 1.0f));
            } else if (prop.type == "cactusTrunk" || prop.type == "cactusArmJoint" || prop.type == "cactusArmSpire") {
                // Cactus Saguaro verde desierto vivo
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.20f, 0.48f, 0.22f, 1.0f));
            } else if (prop.type == "desertRock" || prop.type == "desertMesaRock") {
                // Formaciones de arenisca y monolitos rojizo/ocre del canon
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.76f, 0.46f, 0.28f, 1.0f));
            } else if (prop.type == "desertArchPillar" || prop.type == "desertArchBeam") {
                // Arco de piedra natural arenisca tallada
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.70f, 0.42f, 0.25f, 1.0f));
            } else if (prop.type == "spaceWarpTrimCyan") {
                // Emisivo cian hiperespacial brillante
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.0f, 0.95f, 1.0f, 1.0f));
            } else if (prop.type == "spaceWarpTrimMagenta") {
                // Emisivo magenta hiperespacial brillante
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.15f, 0.85f, 1.0f));
            } else if (prop.type == "spaceWarpPillar" || prop.type == "spaceWarpBeam") {
                // Estructura de aleacion espacial blanca/gris metalizado pulido
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.85f, 0.88f, 0.94f, 1.0f));
            } else if (prop.type == "satellitePanel") {
                // Panel solar azul cobalto fotovoltaico
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.10f, 0.35f, 0.85f, 1.0f));
            } else if (prop.type == "satelliteBody" || prop.type == "satelliteAntenna") {
                // Chasis de satelite dorado termico (manta de mylar oro)
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.95f, 0.80f, 0.25f, 1.0f));
            } else if (prop.type == "spaceBeaconTop") {
                // Baliza estroboscopica cian de curva
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.10f, 0.90f, 1.0f, 1.0f));
            } else if (prop.type == "spaceBeacon") {
                // Base metalica de baliza
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.20f, 0.22f, 0.28f, 1.0f));
            } else {
                // Mallas texturizadas con la paleta de Kenney
                shader.setBool("u_UseTexture", true);
                shader.setVec4("u_Color", glm::vec4(1.0f));
            }

            it->second.draw();
        }
    }
}

} // namespace world
