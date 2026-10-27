#pragma once

#include "geometry/GeometryData.hpp"
#include <vector>

namespace geometry {

class MeshProcessor {
public:
    // Calcula la AABB (mínimo y máximo) de una lista de vértices
    static void computeAABB(const std::vector<Vertex>& vertices, glm::vec3& outMin, glm::vec3& outMax);

    // Calcula el centroide geométrico basado en el centro del AABB
    static glm::vec3 computeCentroid(const std::vector<Vertex>& vertices);

    // Calcula el centroide aritmético (promedio de todos los vértices)
    static glm::vec3 computeAverageCenter(const std::vector<Vertex>& vertices);

    // Recalcula y actualiza la AABB interna de una submalla
    static void updateSubMeshAABB(SubMesh& submesh);

    // Recalcula y actualiza la AABB combinada de un modelo completo
    static void updateModelAABB(ModelData& model);

    // Traslada todos los vértices de una submalla por un offset
    static void translateVertices(SubMesh& submesh, const glm::vec3& offset);

    // Recentra la submalla al origen (0,0,0):
    // 1. Calcula el centroide geométrico de la submalla
    // 2. Almacena dicho centroide en submesh.pivotOffset
    // 3. Resta el centroide a cada vértice (v.position -= centroid)
    // 4. Actualiza minBounds y maxBounds para que queden centrados en (0,0,0)
    static void recenterMeshToOrigin(SubMesh& submesh);

    // Revisa si una submalla tiene normales nulas o inexistentes, y en ese caso
    // calcula automáticamente normales planas para cada triángulo promediando el producto cruz de sus aristas y normalizándolo
    static void generateNormalsIfNeeded(SubMesh& submesh);
};

} // namespace geometry
