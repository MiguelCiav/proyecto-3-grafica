#include "geometry/OBJLoader.hpp"
#include "geometry/MeshProcessor.hpp"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4456 4505 4100 4189)
#endif

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <iostream>
#include <unordered_map>
#include <cstddef>

namespace {

struct VertexKey {
    int p{-1};
    int n{-1};
    int t{-1};
    int m{-1};

    bool operator==(const VertexKey& other) const noexcept {
        return p == other.p && n == other.n && t == other.t && m == other.m;
    }
};

struct VertexKeyHash {
    std::size_t operator()(const VertexKey& k) const noexcept {
        std::size_t seed = 0;
        auto combine = [&seed](int val) {
            seed ^= std::hash<int>{}(val) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
        };
        combine(k.p);
        combine(k.n);
        combine(k.t);
        combine(k.m);
        return seed;
    }
};

} // namespace

namespace geometry {

bool OBJLoader::load(const std::string& filePath, ModelData& outModelData) {
    outModelData = ModelData();
    outModelData.name = filePath;

    tinyobj::ObjReaderConfig config;
    config.triangulate = true;

    size_t lastSlash = filePath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        config.mtl_search_path = filePath.substr(0, lastSlash + 1);
    }

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(filePath, config)) {
        if (!reader.Error().empty()) {
            std::cerr << "[ERROR] [OBJLoader] Error al parsear: " << filePath
                      << " -> " << reader.Error() << std::endl;
        }
        return false;
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    const auto& materials = reader.GetMaterials();

    for (const auto& shape : shapes) {
        SubMesh submesh;
        submesh.name = shape.name;

        // Clave de deduplicacion O(1) con std::unordered_map:
        // vertex, normal, texcoord y material_id para garantizar que limites
        // entre materiales distintos generen vertices independientes con sus respectivos colores.
        std::unordered_map<VertexKey, uint32_t, VertexKeyHash> uniqueVertices;
        uniqueVertices.reserve(shape.mesh.indices.size());
        submesh.vertices.reserve(shape.mesh.indices.size() / 2);
        submesh.indices.reserve(shape.mesh.indices.size());

        size_t indexOffset = 0;
        size_t faceCount = shape.mesh.num_face_vertices.size();

        for (size_t f = 0; f < faceCount; ++f) {
            size_t fv = static_cast<size_t>(shape.mesh.num_face_vertices[f]);
            int matId = (f < shape.mesh.material_ids.size()) ? shape.mesh.material_ids[f] : -1;

            glm::vec4 faceColor(1.0f, 1.0f, 1.0f, 1.0f);
            if (matId >= 0 && matId < static_cast<int>(materials.size())) {
                const auto& mat = materials[matId];
                faceColor = glm::vec4(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1.0f);
            }

            for (size_t v = 0; v < fv; ++v) {
                tinyobj::index_t index = shape.mesh.indices[indexOffset + v];
                VertexKey key{index.vertex_index, index.normal_index, index.texcoord_index, matId};

                auto it = uniqueVertices.find(key);
                if (it == uniqueVertices.end()) {
                    Vertex vertex;

                    if (index.vertex_index >= 0) {
                        vertex.position = glm::vec3(
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2]
                        );
                    }

                    if (index.normal_index >= 0 && !attrib.normals.empty()) {
                        vertex.normal = glm::vec3(
                            attrib.normals[3 * index.normal_index + 0],
                            attrib.normals[3 * index.normal_index + 1],
                            attrib.normals[3 * index.normal_index + 2]
                        );
                    } else {
                        vertex.normal = glm::vec3(0.0f);
                    }

                    if (index.texcoord_index >= 0 && !attrib.texcoords.empty()) {
                        vertex.texCoords = glm::vec2(
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            attrib.texcoords[2 * index.texcoord_index + 1]
                        );
                    }

                    vertex.color = faceColor;

                    uint32_t newIndex = static_cast<uint32_t>(submesh.vertices.size());
                    uniqueVertices.emplace(key, newIndex);
                    submesh.vertices.push_back(vertex);
                    submesh.indices.push_back(newIndex);
                } else {
                    submesh.indices.push_back(it->second);
                }
            }

            indexOffset += fv;
        }

        MeshProcessor::generateNormalsIfNeeded(submesh);
        MeshProcessor::updateSubMeshAABB(submesh);
        outModelData.submeshes.push_back(std::move(submesh));
    }

    MeshProcessor::updateModelAABB(outModelData);
    std::cout << "[OK] [OBJLoader] Modelo cargado: " << filePath
              << " (" << outModelData.submeshes.size() << " submallas)" << std::endl;

    return !outModelData.submeshes.empty();
}

} // namespace geometry
