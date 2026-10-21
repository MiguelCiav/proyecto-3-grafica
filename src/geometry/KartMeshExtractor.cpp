#include "geometry/KartMeshExtractor.hpp"
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
#include <algorithm>
#include <map>

namespace geometry {

void KartMeshExtractor::appendSubMesh(SubMesh& target, const SubMesh& source) {
    uint32_t indexOffset = static_cast<uint32_t>(target.vertices.size());
    target.vertices.insert(target.vertices.end(), source.vertices.begin(), source.vertices.end());
    for (uint32_t idx : source.indices) {
        target.indices.push_back(idx + indexOffset);
    }
    if (target.name.empty()) {
        target.name = source.name;
    }
}

bool KartMeshExtractor::extractFromFile(const std::string& objFilePath, KartAssembly& outAssembly) {
    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.triangulate = true;

    // Directorio base para mtl si aplica
    size_t lastSlash = objFilePath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        readerConfig.mtl_search_path = objFilePath.substr(0, lastSlash + 1);
    }

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(objFilePath, readerConfig)) {
        if (!reader.Error().empty()) {
            std::cerr << "[ERROR] [KartMeshExtractor] TinyObjLoader: " << reader.Error() << std::endl;
        }
        return false;
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    ModelData modelData;
    modelData.name = objFilePath;

    for (const auto& shape : shapes) {
        SubMesh submesh;
        submesh.name = shape.name;

        std::map<std::tuple<int, int, int>, uint32_t> uniqueVertices;

        for (const auto& index : shape.mesh.indices) {
            auto key = std::make_tuple(index.vertex_index, index.normal_index, index.texcoord_index);

            if (uniqueVertices.find(key) == uniqueVertices.end()) {
                Vertex vertex;

                // Posición
                if (index.vertex_index >= 0) {
                    vertex.position = glm::vec3(
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    );
                }

                // Normal
                if (index.normal_index >= 0 && !attrib.normals.empty()) {
                    vertex.normal = glm::vec3(
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    );
                }

                // UV
                if (index.texcoord_index >= 0 && !attrib.texcoords.empty()) {
                    vertex.texCoords = glm::vec2(
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]
                    );
                }

                uint32_t newIndex = static_cast<uint32_t>(submesh.vertices.size());
                uniqueVertices[key] = newIndex;
                submesh.vertices.push_back(vertex);
            }

            submesh.indices.push_back(uniqueVertices[key]);
        }

        MeshProcessor::updateSubMeshAABB(submesh);
        modelData.submeshes.push_back(std::move(submesh));
    }

    return extractFromModelData(modelData, outAssembly);
}

bool KartMeshExtractor::extractFromModelData(const ModelData& model, KartAssembly& outAssembly) {
    // Limpiar estructura destino
    outAssembly = KartAssembly();

    for (const auto& sm : model.submeshes) {
        std::string lowerName = sm.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

        if (lowerName.find("wheel") != std::string::npos) {
            if (lowerName.find("front") != std::string::npos && lowerName.find("left") != std::string::npos) {
                appendSubMesh(outAssembly.wheelFrontLeft, sm);
                outAssembly.wheelFrontLeft.name = "wheel-front-left";
            } else if (lowerName.find("front") != std::string::npos && lowerName.find("right") != std::string::npos) {
                appendSubMesh(outAssembly.wheelFrontRight, sm);
                outAssembly.wheelFrontRight.name = "wheel-front-right";
            } else if (lowerName.find("back") != std::string::npos && lowerName.find("left") != std::string::npos) {
                appendSubMesh(outAssembly.wheelBackLeft, sm);
                outAssembly.wheelBackLeft.name = "wheel-back-left";
            } else if (lowerName.find("back") != std::string::npos && lowerName.find("right") != std::string::npos) {
                appendSubMesh(outAssembly.wheelBackRight, sm);
                outAssembly.wheelBackRight.name = "wheel-back-right";
            } else {
                // Submalla de rueda adicional o de repuesto montada en la carroceria (ej: llanta trasera de SUV)
                appendSubMesh(outAssembly.chassis, sm);
            }
        } else if (lowerName.find("character") != std::string::npos || lowerName.find("driver") != std::string::npos) {
            appendSubMesh(outAssembly.driver, sm);
            outAssembly.driver.name = "driver";
        } else {
            // Chasis o carrocería general
            appendSubMesh(outAssembly.chassis, sm);
            if (outAssembly.chassis.name.empty() || outAssembly.chassis.name == "chassis") {
                outAssembly.chassis.name = sm.name;
            }
        }
    }

    // Actualizar cajas envolventes de cada componente
    MeshProcessor::updateSubMeshAABB(outAssembly.chassis);
    MeshProcessor::updateSubMeshAABB(outAssembly.wheelFrontLeft);
    MeshProcessor::updateSubMeshAABB(outAssembly.wheelFrontRight);
    MeshProcessor::updateSubMeshAABB(outAssembly.wheelBackLeft);
    MeshProcessor::updateSubMeshAABB(outAssembly.wheelBackRight);
    MeshProcessor::updateSubMeshAABB(outAssembly.driver);

    return outAssembly.isValid();
}

} // namespace geometry
