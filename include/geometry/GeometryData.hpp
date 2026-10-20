#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

namespace geometry {

struct Vertex {
    glm::vec3 position{0.0f};
    glm::vec3 normal{0.0f, 1.0f, 0.0f};
    glm::vec2 texCoords{0.0f};

    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal && texCoords == other.texCoords;
    }
};

struct SubMesh {
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Caja envolvente AABB local
    glm::vec3 minBounds{0.0f};
    glm::vec3 maxBounds{0.0f};

    // Punto de pivote / Offset de montaje relativo al chasis del kart
    glm::vec3 pivotOffset{0.0f};
};

struct ModelData {
    std::string name;
    std::vector<SubMesh> submeshes;

    glm::vec3 minBounds{0.0f};
    glm::vec3 maxBounds{0.0f};
};

} // namespace geometry
