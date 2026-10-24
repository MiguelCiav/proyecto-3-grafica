#include "world/PropSpawner.hpp"
#include "geometry/OBJLoader.hpp"
#include <iostream>
#include <filesystem>
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

    // 2. Mallas procedurales para Postes de Luz (Farolas 3D) y Torres de Estadio
    {
        // Mástil vertical de farola (4.2m de altura)
        renderer::MeshGL poleMesh;
        poleMesh.setup(createBoxSubMesh(0.12f, 2.1f, 0.12f));
        m_propMeshes["lampPole"] = std::move(poleMesh);

        // Brazo horizontal en voladizo (alcanza el carril de la pista)
        renderer::MeshGL armMesh;
        armMesh.setup(createBoxSubMesh(0.10f, 0.08f, 0.70f));
        m_propMeshes["lampArm"] = std::move(armMesh);

        // Foco / Luminaria incandescente de vapor de sodio (cabeza visible de farola)
        renderer::MeshGL bulbMesh;
        bulbMesh.setup(createBoxSubMesh(0.35f, 0.22f, 0.35f));
        m_propMeshes["lampBulb"] = std::move(bulbMesh);

        // Torre de iluminación de estadio para el fondo perimetral (10m de altura)
        renderer::MeshGL towerMesh;
        towerMesh.setup(createBoxSubMesh(0.40f, 5.0f, 0.40f));
        m_propMeshes["stadiumTower"] = std::move(towerMesh);
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

    // AABB estricto: altura completa sobre el asfalto (evita que se traspase por debajo o por encima)
    glm::vec3 halfSize = scale * 0.5f;
    if (type == "cone") {
        halfSize = glm::vec3(0.42f, 0.55f, 0.42f) * scale;
        inst.aabbMin = glm::vec3(pos.x - halfSize.x, 0.0f, pos.z - halfSize.z);
        inst.aabbMax = glm::vec3(pos.x + halfSize.x, 1.20f * scale.y, pos.z + halfSize.z);
    } else if (type == "barrierWall") {
        halfSize = glm::vec3(0.60f, 0.40f, 0.25f) * scale;
        inst.aabbMin = glm::vec3(pos.x - halfSize.x, 0.0f, pos.z - halfSize.z);
        inst.aabbMax = glm::vec3(pos.x + halfSize.x, 1.00f * scale.y, pos.z + halfSize.z);
    } else if (type == "barrierRed" || type == "barrierWhite") {
        halfSize = glm::vec3(0.50f, 0.40f, 0.30f) * scale;
        inst.aabbMin = glm::vec3(pos.x - halfSize.x, 0.0f, pos.z - halfSize.z);
        inst.aabbMax = glm::vec3(pos.x + halfSize.x, 1.00f * scale.y, pos.z + halfSize.z);
    } else if (type == "box") {
        halfSize = glm::vec3(0.42f, 0.42f, 0.42f) * scale;
        inst.aabbMin = glm::vec3(pos.x - halfSize.x, 0.0f, pos.z - halfSize.z);
        inst.aabbMax = glm::vec3(pos.x + halfSize.x, 1.10f * scale.y, pos.z + halfSize.z);
    } else if (type == "debris-tire") {
        halfSize = glm::vec3(0.42f, 0.35f, 0.42f) * scale;
        inst.aabbMin = glm::vec3(pos.x - halfSize.x, 0.0f, pos.z - halfSize.z);
        inst.aabbMax = glm::vec3(pos.x + halfSize.x, 1.00f * scale.y, pos.z + halfSize.z);
    } else if (type == "lampPole") {
        halfSize = glm::vec3(0.25f, 2.2f, 0.25f) * scale;
        inst.aabbMin = glm::vec3(pos.x - halfSize.x, 0.0f, pos.z - halfSize.z);
        inst.aabbMax = glm::vec3(pos.x + halfSize.x, 4.40f * scale.y, pos.z + halfSize.z);
    } else if (type == "stadiumTower") {
        halfSize = glm::vec3(0.60f, 5.0f, 0.60f) * scale;
        inst.aabbMin = glm::vec3(pos.x - halfSize.x, 0.0f, pos.z - halfSize.z);
        inst.aabbMax = glm::vec3(pos.x + halfSize.x, 10.0f * scale.y, pos.z + halfSize.z);
    } else {
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

    // Recopilar tramos de recta para distribuir las farolas 3D de iluminación
    std::vector<const TileData*> straightTiles;
    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::ROAD_STRAIGHT) {
            straightTiles.push_back(&tile);
        }
    }

    // 1. Generar 8 Postes de Alumbrado Público 3D (Farolas con brazos y bombillas incandescentes)
    if (!straightTiles.empty()) {
        size_t count = std::min<size_t>(8, straightTiles.size());
        size_t step = straightTiles.size() / count;

        for (size_t i = 0; i < count; ++i) {
            size_t idx = (i * step + 1) % straightTiles.size();
            const auto* tile = straightTiles[idx];

            bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
            float sideSign = (i % 2 == 0) ? 1.0f : -1.0f;
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);

            // Mástil vertical en el borde de césped
            glm::vec3 poleBase = tile->worldPos + sideDir * (halfTile * 1.15f);
            glm::vec3 poleCenter = poleBase + glm::vec3(0.0f, 2.1f, 0.0f);
            addProp("lampPole", poleCenter, 0.0f, glm::vec3(1.0f));

            // Base de protección con neumáticos en el pie de la farola
            addProp("debris-tire", poleBase + glm::vec3(0.0f, 0.25f, 0.0f), 0.0f, glm::vec3(1.1f));

            // Brazo en voladizo inclinado hacia la pista a 4.2m de altura
            glm::vec3 armPos = poleBase + glm::vec3(0.0f, 4.2f, 0.0f) - sideDir * 0.55f;
            float armRot = isHorizontal ? (sideSign > 0 ? 180.0f : 0.0f) : (sideSign > 0 ? 270.0f : 90.0f);
            addProp("lampArm", armPos, armRot, glm::vec3(1.0f));

            // Foco / Luminaria incandescente de vapor de sodio suspendida sobre el carril
            glm::vec3 bulbPos = poleBase + glm::vec3(0.0f, 4.1f, 0.0f) - sideDir * 1.15f;
            addProp("lampBulb", bulbPos, 0.0f, glm::vec3(1.0f));

            m_lampPositions.push_back(bulbPos);
        }
    }

    // 2. Elementos de Pista (Curvas, Rectas, Meta)
    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::EMPTY) continue;

        // Curvas: Barreras bicolor (Rojo/Blanco), pilas de neumáticos y conos de ápice
        if (tile.type == TileType::ROAD_CORNER_SMALL) {
            float rot = tile.rotationDegrees;
            glm::vec3 cornerPos = tile.worldPos;

            float xSign = (rot == 0.0f || rot == 90.0f) ? 1.0f : -1.0f;
            float zSign = (rot == 0.0f || rot == 270.0f) ? 1.0f : -1.0f;

            // Barrera exterior 1 (Roja)
            glm::vec3 barrier1Pos = cornerPos + glm::vec3(xSign * halfTile * 0.85f, 0.0f, zSign * halfTile * 0.40f);
            addProp("barrierRed", barrier1Pos, rot + 30.0f, glm::vec3(1.2f));

            // Barrera exterior 2 (Blanca)
            glm::vec3 barrier2Pos = cornerPos + glm::vec3(xSign * halfTile * 0.40f, 0.0f, zSign * halfTile * 0.85f);
            addProp("barrierWhite", barrier2Pos, rot + 60.0f, glm::vec3(1.2f));

            // Pila de neumáticos de seguridad en la escapatoria exterior
            glm::vec3 tirePos = cornerPos + glm::vec3(xSign * (halfTile * 0.95f), 0.25f, zSign * (halfTile * 0.95f));
            addProp("debris-tire", tirePos, rot + 45.0f, glm::vec3(1.3f));

            // Cono en el ápice interior de la curva (clipping point)
            glm::vec3 apexPos = cornerPos + glm::vec3(-xSign * halfTile * 0.45f, 0.0f, -zSign * halfTile * 0.45f);
            addProp("cone", apexPos, 0.0f, glm::vec3(1.0f));
        }

        // Recta de Meta / Start-Finish: Muros de boxes y cajas de equipo
        if (tile.type == TileType::START_FINISH) {
            bool isHorizontal = (tile.rotationDegrees == 90.0f || tile.rotationDegrees == 270.0f);
            glm::vec3 wallOffset1 = isHorizontal ? glm::vec3(0.0f, 0.0f, -halfTile * 0.92f) : glm::vec3(-halfTile * 0.92f, 0.0f, 0.0f);
            glm::vec3 wallOffset2 = isHorizontal ? glm::vec3(0.0f, 0.0f,  halfTile * 0.92f) : glm::vec3( halfTile * 0.92f, 0.0f, 0.0f);

            addProp("barrierWall", tile.worldPos + wallOffset1, tile.rotationDegrees, glm::vec3(1.0f));
            addProp("barrierWall", tile.worldPos + wallOffset2, tile.rotationDegrees, glm::vec3(1.0f));

            // Cajas de suministros fuera del muro de meta
            glm::vec3 boxPos = tile.worldPos + wallOffset2 + (isHorizontal ? glm::vec3(1.2f, 0.0f, 0.8f) : glm::vec3(0.8f, 0.0f, 1.2f));
            addProp("box", boxPos, 15.0f, glm::vec3(1.1f));
        }

        // Rectas del circuito: Delimitadores, conos alternados y decoraciones de césped
        if (tile.type == TileType::ROAD_STRAIGHT) {
            bool isHorizontal = (tile.rotationDegrees == 90.0f || tile.rotationDegrees == 270.0f);
            float lateralDist = halfTile * 0.88f;

            if ((tile.col + tile.row) % 3 == 0) {
                float side = ((propIndex++) % 2 == 0) ? 1.0f : -1.0f;
                glm::vec3 propOffset = isHorizontal ? glm::vec3(0.0f, 0.0f, side * lateralDist) : glm::vec3(side * lateralDist, 0.0f, 0.0f);

                if (propIndex % 4 == 0) {
                    addProp("barrierRed", tile.worldPos + propOffset, tile.rotationDegrees, glm::vec3(1.1f));
                } else if (propIndex % 4 == 2) {
                    addProp("barrierWhite", tile.worldPos + propOffset, tile.rotationDegrees, glm::vec3(1.1f));
                } else {
                    addProp("cone", tile.worldPos + propOffset, 0.0f, glm::vec3(1.0f));
                }
            }

            // Cajas o neumáticos decorativos en áreas adyacentes
            if ((tile.col * 7 + tile.row * 13) % 19 == 0) {
                float decorSide = ((tile.col % 2 == 0) ? 1.0f : -1.0f) * (halfTile * 1.35f);
                glm::vec3 decorOffset = isHorizontal ? glm::vec3(0.0f, 0.0f, decorSide) : glm::vec3(decorSide, 0.0f, 0.0f);
                if ((tile.col + tile.row) % 2 == 0) {
                    addProp("box", tile.worldPos + decorOffset, 25.0f, glm::vec3(1.0f));
                } else {
                    addProp("debris-tire", tile.worldPos + decorOffset + glm::vec3(0.0f, 0.25f, 0.0f), 0.0f, glm::vec3(1.2f));
                }
            }
        }
    }

    // 3. Decoración del Fondo y Entorno de Estadio Perimetral (Torres Floodlight y Muros)
    float halfW = (grid.getCols() * TrackGrid::TILE_SIZE) * 0.5f + 6.0f;
    float halfD = (grid.getRows() * TrackGrid::TILE_SIZE) * 0.5f + 6.0f;

    // 4 Torres de Iluminación de Estadio en las esquinas del complejo deportivo
    addProp("stadiumTower", glm::vec3(-halfW, 5.0f, -halfD), 45.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3( halfW, 5.0f, -halfD), -45.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3( halfW, 5.0f,  halfD), 45.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3(-halfW, 5.0f,  halfD), -45.0f, glm::vec3(1.0f));

    // Torres intermedias en los laterales para cerrar el horizonte visual
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

    std::cout << "[OK] [PropSpawner] Total de obstáculos y decoraciones generados: "
              << m_props.size() << " (incluye " << m_lampPositions.size()
              << " farolas 3D y torres de estadio)." << std::endl;
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
            } else if (prop.type == "lampPole" || prop.type == "lampArm") {
                // Mástil de acero industrial oscuro
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.24f, 0.26f, 0.30f, 1.0f));
            } else if (prop.type == "stadiumTower") {
                // Estructura de torre de estadio gris metalizado
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.38f, 0.42f, 0.48f, 1.0f));
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
