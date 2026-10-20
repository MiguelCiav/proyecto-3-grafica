#pragma once

#include "geometry/GeometryData.hpp"
#include <string>

namespace geometry {

struct KartAssembly {
    SubMesh chassis;
    SubMesh wheelFrontLeft;
    SubMesh wheelFrontRight;
    SubMesh wheelBackLeft;
    SubMesh wheelBackRight;
    SubMesh driver;

    bool isValid() const {
        return !chassis.vertices.empty() &&
               !wheelFrontLeft.vertices.empty() &&
               !wheelFrontRight.vertices.empty() &&
               !wheelBackLeft.vertices.empty() &&
               !wheelBackRight.vertices.empty();
    }
};

class KartMeshExtractor {
public:
    // Carga un archivo OBJ de kart y clasifica sus grupos en la estructura semántica KartAssembly
    static bool extractFromFile(const std::string& objFilePath, KartAssembly& outAssembly);

    // Clasifica una estructura ModelData previamente cargada en KartAssembly
    static bool extractFromModelData(const ModelData& model, KartAssembly& outAssembly);

private:
    static void appendSubMesh(SubMesh& target, const SubMesh& source);
};

} // namespace geometry
