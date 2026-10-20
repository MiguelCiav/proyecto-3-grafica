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
        "cone.obj", "barrierRed.obj", "barrierWhite.obj", "barrierWall.obj"
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

    // AABB envolvente estimada para colisiones posteriores
    glm::vec3 halfSize = scale * 0.5f;
    inst.aabbMin = pos - halfSize;
    inst.aabbMax = pos + halfSize;

    m_props.push_back(inst);
}

void PropSpawner::generateForGrid(const TrackGrid& grid) {
    m_props.clear();

    float halfTile = TrackGrid::TILE_SIZE * 0.5f;

    // Colocar conos y barreras a lo largo del circuito
    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::EMPTY) continue;

        // Si es una curva, colocar barreras en el borde exterior
        if (tile.type == TileType::ROAD_CORNER_SMALL) {
            float rot = tile.rotationDegrees;
            glm::vec3 cornerPos = tile.worldPos;

            // Offset hacia la esquina exterior
            glm::vec3 barrierPos = cornerPos + glm::vec3(
                (rot == 0.0f || rot == 90.0f) ? halfTile * 0.8f : -halfTile * 0.8f,
                0.0f,
                (rot == 90.0f || rot == 180.0f) ? halfTile * 0.8f : -halfTile * 0.8f
            );
            addProp("barrierRed", barrierPos, rot + 45.0f, glm::vec3(1.2f));
        }

        // Si es la meta, colocar vallas delimitadoras a los lados
        if (tile.type == TileType::START_FINISH) {
            addProp("barrierWall", tile.worldPos + glm::vec3(0.0f, 0.0f, -halfTile * 0.9f), 90.0f, glm::vec3(1.0f));
            addProp("barrierWall", tile.worldPos + glm::vec3(0.0f, 0.0f,  halfTile * 0.9f), 90.0f, glm::vec3(1.0f));
        }

        // Colocar conos alternados en algunas rectas para crear obstáculos de chicane
        if (tile.type == TileType::ROAD_STRAIGHT && (tile.col % 3 == 0 || tile.row % 3 == 0)) {
            addProp("cone", tile.worldPos + glm::vec3(0.6f, 0.0f, 0.0f), 0.0f, glm::vec3(1.0f));
        }
    }

    std::cout << "[OK] [PropSpawner] Total de obstáculos y barreras generados: " << m_props.size() << std::endl;
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
