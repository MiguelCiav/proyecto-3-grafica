#include "geometry/MeshProcessor.hpp"
#include <limits>
#include <algorithm>

namespace geometry {

void MeshProcessor::computeAABB(const std::vector<Vertex>& vertices, glm::vec3& outMin, glm::vec3& outMax) {
    if (vertices.empty()) {
        outMin = glm::vec3(0.0f);
        outMax = glm::vec3(0.0f);
        return;
    }

    outMin = glm::vec3(std::numeric_limits<float>::max());
    outMax = glm::vec3(std::numeric_limits<float>::lowest());

    for (const auto& v : vertices) {
        outMin = glm::min(outMin, v.position);
        outMax = glm::max(outMax, v.position);
    }
}

glm::vec3 MeshProcessor::computeCentroid(const std::vector<Vertex>& vertices) {
    if (vertices.empty()) {
        return glm::vec3(0.0f);
    }
    glm::vec3 minB(0.0f), maxB(0.0f);
    computeAABB(vertices, minB, maxB);
    return (minB + maxB) * 0.5f;
}

glm::vec3 MeshProcessor::computeAverageCenter(const std::vector<Vertex>& vertices) {
    if (vertices.empty()) {
        return glm::vec3(0.0f);
    }
    glm::vec3 sum(0.0f);
    for (const auto& v : vertices) {
        sum += v.position;
    }
    return sum / static_cast<float>(vertices.size());
}

void MeshProcessor::updateSubMeshAABB(SubMesh& submesh) {
    computeAABB(submesh.vertices, submesh.minBounds, submesh.maxBounds);
}

void MeshProcessor::updateModelAABB(ModelData& model) {
    if (model.submeshes.empty()) {
        model.minBounds = glm::vec3(0.0f);
        model.maxBounds = glm::vec3(0.0f);
        return;
    }

    model.minBounds = glm::vec3(std::numeric_limits<float>::max());
    model.maxBounds = glm::vec3(std::numeric_limits<float>::lowest());

    for (auto& sm : model.submeshes) {
        updateSubMeshAABB(sm);
        model.minBounds = glm::min(model.minBounds, sm.minBounds);
        model.maxBounds = glm::max(model.maxBounds, sm.maxBounds);
    }
}

void MeshProcessor::translateVertices(SubMesh& submesh, const glm::vec3& offset) {
    for (auto& v : submesh.vertices) {
        v.position += offset;
    }
    submesh.minBounds += offset;
    submesh.maxBounds += offset;
}

void MeshProcessor::recenterMeshToOrigin(SubMesh& submesh) {
    if (submesh.vertices.empty()) return;

    // Calcular el centroide geométrico de la pieza
    glm::vec3 center = computeCentroid(submesh.vertices);

    // Guardar el centroide como offset de montaje relativo
    submesh.pivotOffset = center;

    // Trasladar todos los vértices restando el centroide
    for (auto& v : submesh.vertices) {
        v.position -= center;
    }

    // Actualizar AABB local centrada en (0,0,0)
    updateSubMeshAABB(submesh);
}

} // namespace geometry
