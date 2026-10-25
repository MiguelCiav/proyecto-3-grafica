#include "renderer/TrackRenderer.hpp"
#include "geometry/OBJLoader.hpp"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace renderer {

TrackRenderer::TrackRenderer() = default;
TrackRenderer::~TrackRenderer() = default;

void TrackRenderer::createGroundPlane() {
    geometry::SubMesh ground;
    ground.name = "Ground_Plane";

    float size = 350.0f;
    float uvScale = 70.0f;
    float y = -0.05f;

    // Vértices del plano de césped/suelo
    ground.vertices = {
        { {-size, y, -size}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
        { { size, y, -size}, {0.0f, 1.0f, 0.0f}, {uvScale, 0.0f} },
        { { size, y,  size}, {0.0f, 1.0f, 0.0f}, {uvScale, uvScale} },
        { {-size, y,  size}, {0.0f, 1.0f, 0.0f}, {0.0f, uvScale} }
    };

    ground.indices = {
        0, 3, 2,
        2, 1, 0
    };

    m_meshGround.setup(ground);
}

bool TrackRenderer::init(const std::string& trackAssetsDir) {
    std::string straightPath = (fs::path(trackAssetsDir) / "roadStraight.obj").string();
    std::string cornerSmallPath = (fs::path(trackAssetsDir) / "roadCornerSmall.obj").string();
    std::string cornerLargePath = (fs::path(trackAssetsDir) / "roadCornerLarge.obj").string();

    geometry::ModelData straightData;
    if (geometry::OBJLoader::load(straightPath, straightData) && !straightData.submeshes.empty()) {
        m_submeshStraight = straightData.submeshes[0];
    } else {
        std::cerr << "[ERROR] [TrackRenderer] No se pudo cargar: " << straightPath << std::endl;
        return false;
    }

    geometry::ModelData cornerSmallData;
    if (geometry::OBJLoader::load(cornerSmallPath, cornerSmallData) && !cornerSmallData.submeshes.empty()) {
        m_submeshCornerSmall = cornerSmallData.submeshes[0];
    } else {
        std::cerr << "[ERROR] [TrackRenderer] No se pudo cargar: " << cornerSmallPath << std::endl;
        return false;
    }

    geometry::ModelData cornerLargeData;
    if (geometry::OBJLoader::load(cornerLargePath, cornerLargeData) && !cornerLargeData.submeshes.empty()) {
        m_submeshCornerLarge = cornerLargeData.submeshes[0];
    }

    createGroundPlane();
    updateTrackMeshes();

    m_initialized = true;
    std::cout << "[OK] [TrackRenderer] Mallas modulares de pista cargadas exitosamente a la GPU." << std::endl;
    return true;
}

void TrackRenderer::setGroundColor(const glm::vec4& color) {
    m_groundColor = color;
    updateTrackMeshes();
}

void TrackRenderer::updateTrackMeshes() {
    if (m_submeshStraight.vertices.empty()) return;

    auto adaptGrass = [this](geometry::SubMesh submesh) {
        for (auto& v : submesh.vertices) {
            // Adaptar los vértices de césped nativo de Kenney (grass) para que
            // armonicen exactamente con el bioma y color de terreno del circuito activo
            if (std::abs(v.color.r - 0.3019608f) < 0.02f &&
                std::abs(v.color.g - 0.5607843f) < 0.02f &&
                std::abs(v.color.b - 0.4313726f) < 0.02f) {
                v.color = m_groundColor;
            }
        }
        return submesh;
    };

    if (!m_submeshStraight.vertices.empty()) {
        m_meshStraight.setup(adaptGrass(m_submeshStraight));
    }
    if (!m_submeshCornerSmall.vertices.empty()) {
        m_meshCornerSmall.setup(adaptGrass(m_submeshCornerSmall));
    }
    if (!m_submeshCornerLarge.vertices.empty()) {
        m_meshCornerLarge.setup(adaptGrass(m_submeshCornerLarge));
    }
}

void TrackRenderer::render(const world::TrackGrid& grid, const Shader& shader) const {
    if (!m_initialized) return;

    m_renderedTileCount = 0;

    // 1. Dibujar el plano de terreno con su color de bioma uniforme
    shader.setBool("u_UseTexture", false);
    shader.setBool("u_UseVertexColor", false);
    shader.setMat4("u_Model", glm::mat4(1.0f));
    shader.setVec4("u_Color", m_groundColor);
    m_meshGround.draw();

    // 2. Activar colores de vértice nativos para las piezas de pista modular (asfalto, bordillos y suelo)
    shader.setBool("u_UseTexture", false);
    shader.setBool("u_UseVertexColor", true);
    shader.setVec4("u_Color", glm::vec4(1.0f));

    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == world::TileType::EMPTY) continue;

        shader.setMat4("u_Model", tile.modelMatrix);

        switch (tile.type) {
            case world::TileType::ROAD_STRAIGHT:
            case world::TileType::START_FINISH:
                m_meshStraight.draw();
                m_renderedTileCount++;
                break;

            case world::TileType::ROAD_CORNER_SMALL:
                m_meshCornerSmall.draw();
                m_renderedTileCount++;
                break;

            case world::TileType::ROAD_CORNER_LARGE:
                if (m_meshCornerLarge.isLoaded()) {
                    m_meshCornerLarge.draw();
                } else {
                    m_meshCornerSmall.draw();
                }
                m_renderedTileCount++;
                break;

            default:
                break;
        }
    }
}

} // namespace renderer
