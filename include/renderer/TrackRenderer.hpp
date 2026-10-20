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

private:
    void createGroundPlane();

    MeshGL m_meshStraight;
    MeshGL m_meshCornerSmall;
    MeshGL m_meshCornerLarge;
    MeshGL m_meshGround;

    mutable int m_renderedTileCount{0};
    bool m_initialized{false};
};

} // namespace renderer
