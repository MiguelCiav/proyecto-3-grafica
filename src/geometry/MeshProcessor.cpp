#include "geometry/MeshProcessor.hpp"
#include <limits>
#include <algorithm>
#include <glm/geometric.hpp>

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

void MeshProcessor::generateNormalsIfNeeded(SubMesh& submesh) {
    if (submesh.vertices.empty()) {
        return;
    }

    // Verificar si la submalla carece de normales o si son nulas (longitud cercana a cero)
    bool hasMissingNormals = false;
    for (const auto& v : submesh.vertices) {
        if (glm::dot(v.normal, v.normal) < 1e-6f) {
            hasMissingNormals = true;
            break;
        }
    }

    if (!hasMissingNormals) {
        return;
    }

    // Reiniciar todas las normales a cero antes de acumular
    for (auto& v : submesh.vertices) {
        v.normal = glm::vec3(0.0f);
    }

    // Calcular el producto cruz de las aristas de cada triángulo y promediar/acumular
    if (!submesh.indices.empty()) {
        const size_t indexCount = submesh.indices.size();
        for (size_t i = 0; i + 2 < indexCount; i += 3) {
            uint32_t i0 = submesh.indices[i];
            uint32_t i1 = submesh.indices[i + 1];
            uint32_t i2 = submesh.indices[i + 2];

            if (i0 >= submesh.vertices.size() || i1 >= submesh.vertices.size() || i2 >= submesh.vertices.size()) {
                continue;
            }

            const glm::vec3& p0 = submesh.vertices[i0].position;
            const glm::vec3& p1 = submesh.vertices[i1].position;
            const glm::vec3& p2 = submesh.vertices[i2].position;

            glm::vec3 edge1 = p1 - p0;
            glm::vec3 edge2 = p2 - p0;
            glm::vec3 normal = glm::cross(edge1, edge2);

            submesh.vertices[i0].normal += normal;
            submesh.vertices[i1].normal += normal;
            submesh.vertices[i2].normal += normal;
        }
    } else {
        const size_t vertexCount = submesh.vertices.size();
        for (size_t i = 0; i + 2 < vertexCount; i += 3) {
            const glm::vec3& p0 = submesh.vertices[i].position;
            const glm::vec3& p1 = submesh.vertices[i + 1].position;
            const glm::vec3& p2 = submesh.vertices[i + 2].position;

            glm::vec3 edge1 = p1 - p0;
            glm::vec3 edge2 = p2 - p0;
            glm::vec3 normal = glm::cross(edge1, edge2);

            submesh.vertices[i].normal += normal;
            submesh.vertices[i + 1].normal += normal;
            submesh.vertices[i + 2].normal += normal;
        }
    }

    // Normalizar las normales calculadas
    for (auto& v : submesh.vertices) {
        float len2 = glm::dot(v.normal, v.normal);
        if (len2 > 1e-6f) {
            v.normal = glm::normalize(v.normal);
        } else {
            v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }
}

} // namespace geometry
