#pragma once

#include "world/TrackGrid.hpp"
#include "renderer/Shader.hpp"
#include "renderer/MeshGL.hpp"
#include <memory>
#include <string>

namespace renderer {

class TrackRenderer {
public:
    TrackRenderer();
    ~TrackRenderer();

    // Carga las mallas modulares de la pista a la GPU
    bool init(const std::string& trackAssetsDir = "assets/models/track");

    // Dibuja todas las celdas del circuito y el plano de terreno
    void render(const world::TrackGrid& grid, const Shader& shader) const;

    int getRenderedTileCount() const { return m_renderedTileCount; }
    void setGroundColor(const glm::vec4& color);
    glm::vec4 getGroundColor() const { return m_groundColor; }

private:
    void createGroundPlane();
    void updateTrackMeshes();

    MeshGL m_meshStraight;
    MeshGL m_meshCornerSmall;
    MeshGL m_meshCornerLarge;
    MeshGL m_meshGround;

    geometry::SubMesh m_submeshStraight;
    geometry::SubMesh m_submeshCornerSmall;
    geometry::SubMesh m_submeshCornerLarge;

    mutable int m_renderedTileCount{0};
    bool m_initialized{false};
    glm::vec4 m_groundColor{0.28f, 0.62f, 0.28f, 1.0f};
};

} // namespace renderer
