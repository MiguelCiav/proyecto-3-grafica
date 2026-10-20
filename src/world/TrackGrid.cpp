#include "world/TrackGrid.hpp"
#include <cmath>

namespace world {

TrackGrid::TrackGrid(int cols, int rows) {
    resize(cols, rows);
}

void TrackGrid::resize(int cols, int rows) {
    m_cols = std::max(1, cols);
    m_rows = std::max(1, rows);
    m_tiles.clear();
    m_tiles.resize(m_cols * m_rows);

    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            int idx = r * m_cols + c;
            m_tiles[idx].col = c;
            m_tiles[idx].row = r;
            m_tiles[idx].type = TileType::EMPTY;
            m_tiles[idx].surface = SurfaceType::GRASS;
            m_tiles[idx].rotationDegrees = 0.0f;
            m_tiles[idx].worldPos = gridToWorld(c, r);
        }
    }
    updateModelMatrices();
}

void TrackGrid::clear() {
    for (auto& t : m_tiles) {
        t.type = TileType::EMPTY;
        t.surface = SurfaceType::GRASS;
        t.rotationDegrees = 0.0f;
    }
    updateModelMatrices();
}

glm::vec3 TrackGrid::gridToWorld(int col, int row) const {
    // Centrar la cuadrícula alrededor del origen (0,0)
    float halfWidth = (m_cols * TILE_SIZE) * 0.5f;
    float halfDepth = (m_rows * TILE_SIZE) * 0.5f;

    float x = (static_cast<float>(col) * TILE_SIZE) - halfWidth + (TILE_SIZE * 0.5f);
    float z = (static_cast<float>(row) * TILE_SIZE) - halfDepth + (TILE_SIZE * 0.5f);
    return glm::vec3(x, 0.0f, z);
}

glm::ivec2 TrackGrid::worldToGrid(float x, float z) const {
    float halfWidth = (m_cols * TILE_SIZE) * 0.5f;
    float halfDepth = (m_rows * TILE_SIZE) * 0.5f;

    int col = static_cast<int>(std::floor((x + halfWidth) / TILE_SIZE));
    int row = static_cast<int>(std::floor((z + halfDepth) / TILE_SIZE));

    return glm::ivec2(col, row);
}

void TrackGrid::setTile(int col, int row, TileType type, float rotationDeg, SurfaceType surface) {
    if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) return;

    int idx = row * m_cols + col;
    m_tiles[idx].type = type;
    m_tiles[idx].rotationDegrees = rotationDeg;
    m_tiles[idx].surface = surface;
    m_tiles[idx].worldPos = gridToWorld(col, row);
}

const TileData* TrackGrid::getTile(int col, int row) const {
    if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) return nullptr;
    return &m_tiles[row * m_cols + col];
}

TileData* TrackGrid::getTile(int col, int row) {
    if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) return nullptr;
    return &m_tiles[row * m_cols + col];
}

const TileData* TrackGrid::getTileAtWorldPos(float x, float z) const {
    glm::ivec2 gridCoord = worldToGrid(x, z);
    return getTile(gridCoord.x, gridCoord.y);
}

void TrackGrid::updateModelMatrices() {
    for (auto& t : m_tiles) {
        t.worldPos = gridToWorld(t.col, t.row);

        // Trasladar a la posición de la celda
        glm::mat4 mat = glm::translate(glm::mat4(1.0f), t.worldPos);
        // Rotar sobre el eje Y
        mat = glm::rotate(mat, glm::radians(t.rotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
        // Escalar por TILE_SIZE para que la pieza de Kenney (1x1) ocupe 4x4 unidades
        mat = glm::scale(mat, glm::vec3(TILE_SIZE));
        
        // Compensación de pivote de las piezas de Kenney (para centrar de [-1,0] y [0,1] a [-0.5, 0.5])
        mat = glm::translate(mat, glm::vec3(0.5f, 0.0f, -0.5f));

        t.modelMatrix = mat;
    }
}

} // namespace world
