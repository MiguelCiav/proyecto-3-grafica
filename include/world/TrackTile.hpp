#pragma once

#include <string>
#include <glm/glm.hpp>

namespace world {

enum class TileType {
    EMPTY,
    ROAD_STRAIGHT,
    ROAD_CORNER_SMALL,
    ROAD_CORNER_LARGE,
    START_FINISH,
    OFFROAD_GRASS
};

enum class SurfaceType {
    ASPHALT,
    GRASS,
    BOOST_PAD,
    OIL_SLICK
};

struct TileData {
    TileType type{TileType::EMPTY};
    SurfaceType surface{SurfaceType::ASPHALT};
    float rotationDegrees{0.0f}; // 0, 90, 180, 270
    int col{0};
    int row{0};
    glm::vec3 worldPos{0.0f};
    glm::mat4 modelMatrix{1.0f};
};

} // namespace world
