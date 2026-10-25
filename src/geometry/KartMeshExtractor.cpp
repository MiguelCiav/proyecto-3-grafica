#include "geometry/KartMeshExtractor.hpp"
#include "geometry/MeshProcessor.hpp"
#include "geometry/OBJLoader.hpp"
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
    ModelData modelData;
    if (!OBJLoader::load(objFilePath, modelData)) {
        std::cerr << "[ERROR] [KartMeshExtractor] Error cargando modelo OBJ: " << objFilePath << std::endl;
        return false;
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
