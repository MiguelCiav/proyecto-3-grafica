#include "world/PropSpawner.hpp"
#include "geometry/OBJLoader.hpp"
#include <iostream>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>

namespace fs = std::filesystem;

namespace world {

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

    // AABB ajustado con precisión a la geometría real de cada prop
    glm::vec3 halfSize = scale * 0.5f;
    if (type == "cone") {
        halfSize = glm::vec3(0.20f, 0.30f, 0.20f) * scale;
    } else if (type == "barrierWall") {
        halfSize = glm::vec3(0.55f, 0.35f, 0.15f) * scale;
    } else if (type == "barrierRed" || type == "barrierWhite") {
        halfSize = glm::vec3(0.45f, 0.35f, 0.25f) * scale;
    } else if (type == "box") {
        halfSize = glm::vec3(0.36f, 0.36f, 0.36f) * scale;
    } else if (type == "debris-tire") {
        halfSize = glm::vec3(0.30f, 0.30f, 0.30f) * scale;
    }
    inst.aabbMin = pos - halfSize;
    inst.aabbMax = pos + halfSize;

    m_props.push_back(inst);
}

void PropSpawner::generateForGrid(const TrackGrid& grid) {
    m_props.clear();

    float halfTile = TrackGrid::TILE_SIZE * 0.5f;
    int propIndex = 0;

    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::EMPTY) continue;

        // 1. Curvas del circuito: Barreras bicolor (Rojo/Blanco), pilas de neumáticos y conos de vértice
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

        // 2. Recta de Meta / Start-Finish: Muros de boxes y cajas de equipo
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

        // 3. Rectas del circuito: Delimitadores, conos alternados y decoraciones de césped
        if (tile.type == TileType::ROAD_STRAIGHT) {
            bool isHorizontal = (tile.rotationDegrees == 90.0f || tile.rotationDegrees == 270.0f);
            float lateralDist = halfTile * 0.88f;

            // Colocar barreras alternadas y conos a los lados de la pista cada 2 o 3 losas
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

            // Cajas o neumáticos decorativos en áreas abiertas adyacentes a la pista
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

    std::cout << "[OK] [PropSpawner] Total de obstáculos y decoraciones generados: " << m_props.size() << std::endl;
}

void PropSpawner::render(const renderer::Shader& shader) const {
    if (!m_initialized) return;

    for (const auto& prop : m_props) {
        auto it = m_propMeshes.find(prop.type);
        if (it != m_propMeshes.end()) {
            shader.setMat4("u_Model", prop.modelMatrix);
            shader.setVec4("u_Color", glm::vec4(1.0f));
            it->second.draw();
        }
    }
}

} // namespace world
