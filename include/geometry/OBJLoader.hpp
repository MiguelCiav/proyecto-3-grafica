#pragma once

#include "geometry/GeometryData.hpp"
#include <string>

namespace geometry {

class OBJLoader {
public:
    // Carga cualquier modelo OBJ y rellena la estructura ModelData
    static bool load(const std::string& filePath, ModelData& outModelData);
};

} // namespace geometry
