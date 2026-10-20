#pragma once

#include "world/TrackTile.hpp"
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

namespace world {

class TrackGrid {
public:
    static constexpr float TILE_SIZE = 4.0f; // Tamaño de cada celda en unidades de mundo

    TrackGrid(int cols = 12, int rows = 12);

    void resize(int cols, int rows);
    void clear();

    void setTile(int col, int row, TileType type, float rotationDeg = 0.0f, SurfaceType surface = SurfaceType::ASPHALT);
    const TileData* getTile(int col, int row) const;
    TileData* getTile(int col, int row);

    // Consulta celda en base a posición continua en el mundo (x, z)
    const TileData* getTileAtWorldPos(float x, float z) const;

    // Conversión entre cuadrícula y mundo
    glm::vec3 gridToWorld(int col, int row) const;
    glm::ivec2 worldToGrid(float x, float z) const;

    int getCols() const { return m_cols; }
    int getRows() const { return m_rows; }
    const std::vector<TileData>& getAllTiles() const { return m_tiles; }
    std::vector<TileData>& getAllTiles() { return m_tiles; }

    void updateModelMatrices();

private:
    int m_cols{12};
    int m_rows{12};
    std::vector<TileData> m_tiles;
};

} // namespace world
