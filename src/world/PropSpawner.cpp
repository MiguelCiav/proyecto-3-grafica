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

        // Arco de Meta / Gantry pórtico sobre la pista
        renderer::MeshGL gantryPillar;
        gantryPillar.setup(createBoxSubMesh(0.14f, 2.3f, 0.14f));
        m_propMeshes["gantryPillar"] = std::move(gantryPillar);

        renderer::MeshGL gantryBeam;
        gantryBeam.setup(createBoxSubMesh(2.5f, 0.14f, 0.18f));
        m_propMeshes["gantryBeam"] = std::move(gantryBeam);

        renderer::MeshGL gantrySign;
        gantrySign.setup(createBoxSubMesh(1.5f, 0.42f, 0.06f));
        m_propMeshes["gantrySign"] = std::move(gantrySign);
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
    } else if (type == "gantryPillar") {
        inst.aabbMin = glm::vec3(pos.x - 0.16f, 0.0f, pos.z - 0.16f);
        inst.aabbMax = glm::vec3(pos.x + 0.16f, 4.60f * scale.y, pos.z + 0.16f);
    } else {
        // Elementos suspendidos (brazos de luz, bombillas, follaje alto, letrero de meta):
        // Se definen en un plano superior para que el kart no colisione en el suelo
        glm::vec3 halfSize = scale * 0.5f;
        inst.aabbMin = pos - halfSize;
        inst.aabbMax = pos + halfSize;
    }

    m_props.push_back(inst);
}

void PropSpawner::generateForGrid(const TrackGrid& grid) {
    m_props.clear();
    m_lampPositions.clear();

    float halfTile = TrackGrid::TILE_SIZE * 0.5f;
    int propIndex = 0;

    // Recopilar losas de la pista
    std::vector<const TileData*> straightTiles;
    const TileData* startFinishTile = nullptr;

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
        float beamRot = isHorizontal ? 0.0f : 90.0f;

        // Pilares izquierdo y derecho anclados en el césped a cada lado del carril
        glm::vec3 leftPillar = startFinishTile->worldPos + sideDir * 2.35f + glm::vec3(0.0f, 2.3f, 0.0f);
        glm::vec3 rightPillar = startFinishTile->worldPos - sideDir * 2.35f + glm::vec3(0.0f, 2.3f, 0.0f);
        addProp("gantryPillar", leftPillar, 0.0f, glm::vec3(1.0f));
        addProp("gantryPillar", rightPillar, 0.0f, glm::vec3(1.0f));

        // Viga transversal superior en celosía a 4.6m de altura
        glm::vec3 beamPos = startFinishTile->worldPos + glm::vec3(0.0f, 4.6f, 0.0f);
        addProp("gantryBeam", beamPos, beamRot, glm::vec3(1.0f));

        // Letrero luminoso de START / FINISH en el centro suspendido sobre la pista
        glm::vec3 signPos = startFinishTile->worldPos + glm::vec3(0.0f, 4.35f, 0.0f);
        addProp("gantrySign", signPos, beamRot, glm::vec3(1.0f));
    }

    // 2. Generar 8 Postes de Alumbrado Público 3D (Farolas con brazos y bombillas)
    if (!straightTiles.empty()) {
        size_t count = std::min<size_t>(8, straightTiles.size());
        size_t step = straightTiles.size() / count;

        for (size_t i = 0; i < count; ++i) {
            size_t idx = (i * step + 1) % straightTiles.size();
            const auto* tile = straightTiles[idx];

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

    // 3. Elementos de Pista (Curvas, Rectas, Meta)
    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::EMPTY) continue;

        // Curvas: Barreras bicolor (Rojo/Blanco), neumáticos de seguridad y conos de ápice
        if (tile.type == TileType::ROAD_CORNER_SMALL) {
            float rot = tile.rotationDegrees;
            glm::vec3 cornerPos = tile.worldPos;

            float xSign = (rot == 0.0f || rot == 90.0f) ? 1.0f : -1.0f;
            float zSign = (rot == 0.0f || rot == 270.0f) ? 1.0f : -1.0f;

            // Barrera exterior 1 (Roja)
            glm::vec3 barrier1Pos = cornerPos + glm::vec3(xSign * halfTile * 0.88f, 0.0f, zSign * halfTile * 0.40f);
            addProp("barrierRed", barrier1Pos, rot + 30.0f, glm::vec3(1.1f));

            // Barrera exterior 2 (Blanca)
            glm::vec3 barrier2Pos = cornerPos + glm::vec3(xSign * halfTile * 0.40f, 0.0f, zSign * halfTile * 0.88f);
            addProp("barrierWhite", barrier2Pos, rot + 60.0f, glm::vec3(1.1f));

            // Pila de neumáticos de seguridad en la escapatoria exterior
            glm::vec3 tirePos = cornerPos + glm::vec3(xSign * (halfTile * 1.05f), 0.25f, zSign * (halfTile * 1.05f));
            addProp("debris-tire", tirePos, rot + 45.0f, glm::vec3(1.1f));

            // Cono en el ápice interior de la curva (clipping point)
            glm::vec3 apexPos = cornerPos + glm::vec3(-xSign * halfTile * 0.52f, 0.0f, -zSign * halfTile * 0.52f);
            addProp("cone", apexPos, 0.0f, glm::vec3(1.0f));
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

    // 4. Bosques y Pinos Low-Poly en las Áreas Abiertas de Césped (Infield & Outfield)
    for (int r = 1; r < grid.getRows() - 1; ++r) {
        for (int c = 1; c < grid.getCols() - 1; ++c) {
            const auto* current = grid.getTile(c, r);
            if (current && current->type == TileType::EMPTY) {
                // Verificar que no sea inmediatamente adyacente a la pista para dejar zona de escape
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

                // Si es un área verde despejada, plantar un pino de competición
                if (!nearRoad && (c * 13 + r * 17) % 3 == 0) {
                    glm::vec3 treePos = current->worldPos + glm::vec3(
                        ((c % 3) - 1.0f) * 0.8f,
                        0.0f,
                        ((r % 3) - 1.0f) * 0.8f
                    );
                    float treeRot = static_cast<float>((c * 37 + r * 53) % 360);

                    // Tronco
                    addProp("treeTrunk", treePos + glm::vec3(0.0f, 0.8f, 0.0f), treeRot, glm::vec3(1.0f));
                    // Capa inferior de follaje verde
                    addProp("treeFoliageBottom", treePos + glm::vec3(0.0f, 1.85f, 0.0f), treeRot + 15.0f, glm::vec3(1.0f));
                    // Capa superior de follaje verde
                    addProp("treeFoliageTop", treePos + glm::vec3(0.0f, 2.70f, 0.0f), treeRot + 30.0f, glm::vec3(1.0f));
                }
            }
        }
    }

    // 5. Decoración del Fondo y Entorno de Estadio Perimetral (Torres Floodlight y Muros)
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

    std::cout << "[OK] [PropSpawner] Total de elementos y decoraciones generados: "
              << m_props.size() << " (incluye Arco de Meta, farolas 3D, pinos y torres)." << std::endl;
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
            } else if (prop.type == "lampPole" || prop.type == "lampArm" || prop.type == "gantryPillar" || prop.type == "gantryBeam") {
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
