#include "geometry/KartPreprocessor.hpp"
#include "geometry/MeshProcessor.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>

namespace geometry {

bool KartPreprocessor::processKart(const std::string& objFilePath, KartAssembly& outAssembly) {
    if (!KartMeshExtractor::extractFromFile(objFilePath, outAssembly)) {
        std::cerr << "[ERROR] [KartPreprocessor] No se pudo extraer la geometría de: " << objFilePath << std::endl;
        return false;
    }

    // Recentrar los pivotes de las 4 ruedas al origen (0,0,0)
    // Guardando el centroide previo como pivotOffset de montaje
    MeshProcessor::recenterMeshToOrigin(outAssembly.wheelFrontLeft);
    MeshProcessor::recenterMeshToOrigin(outAssembly.wheelFrontRight);
    MeshProcessor::recenterMeshToOrigin(outAssembly.wheelBackLeft);
    MeshProcessor::recenterMeshToOrigin(outAssembly.wheelBackRight);

    return true;
}

bool KartPreprocessor::validateAndReport(const KartAssembly& assembly, const std::string& kartName) {
    std::cout << "\n========================================================" << std::endl;
    std::cout << "  REPORTE DIAGNÓSTICO DE GEOMETRÍA - " << kartName << std::endl;
    std::cout << "  (Validación de Regla Cero Blender - Pivotes en C++)   " << std::endl;
    std::cout << "========================================================" << std::endl;

    std::cout << "[CHASIS] Vértices: " << assembly.chassis.vertices.size()
              << " | Índices: " << assembly.chassis.indices.size() << std::endl;
    std::cout << "         AABB Min: (" << assembly.chassis.minBounds.x << ", " << assembly.chassis.minBounds.y << ", " << assembly.chassis.minBounds.z << ")" << std::endl;
    std::cout << "         AABB Max: (" << assembly.chassis.maxBounds.x << ", " << assembly.chassis.maxBounds.y << ", " << assembly.chassis.maxBounds.z << ")" << std::endl;

    const SubMesh* wheels[4] = {
        &assembly.wheelFrontLeft,
        &assembly.wheelFrontRight,
        &assembly.wheelBackLeft,
        &assembly.wheelBackRight
    };
    const char* wheelNames[4] = {
        "Rueda Delantera Izquierda (FL)",
        "Rueda Delantera Derecha   (FR)",
        "Rueda Trasera Izquierda   (BL)",
        "Rueda Trasera Derecha     (BR)"
    };

    bool allPivotsAtOrigin = true;
    constexpr float EPSILON = 1e-4f;

    for (int i = 0; i < 4; ++i) {
        glm::vec3 centroid = MeshProcessor::computeCentroid(wheels[i]->vertices);
        float distFromOrigin = glm::length(centroid);
        bool atOrigin = (distFromOrigin < EPSILON);

        if (!atOrigin) allPivotsAtOrigin = false;

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "[" << wheelNames[i] << "]" << std::endl;
        std::cout << "  - Vértices: " << wheels[i]->vertices.size() << " | Índices: " << wheels[i]->indices.size() << std::endl;
        std::cout << "  - Offset de montaje (pivotOffset): ("
                  << wheels[i]->pivotOffset.x << ", "
                  << wheels[i]->pivotOffset.y << ", "
                  << wheels[i]->pivotOffset.z << ")" << std::endl;
        std::cout << "  - Centroide geométrico actual:      ("
                  << centroid.x << ", " << centroid.y << ", " << centroid.z << ") -> "
                  << (atOrigin ? "[VERIFICADO EN (0,0,0)]" : "[DESALINEADO]") << std::endl;
    }

    if (!assembly.driver.vertices.empty()) {
        std::cout << "[PILOTO] Vértices: " << assembly.driver.vertices.size()
                  << " | Índices: " << assembly.driver.indices.size() << std::endl;
    }

    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "ESTADO: " << (allPivotsAtOrigin ? "[ÉXITO] Todas las ruedas tienen pivote en (0,0,0)" : "[ERROR] Desalineación detectada") << std::endl;
    std::cout << "========================================================\n" << std::endl;

    return allPivotsAtOrigin;
}

} // namespace geometry
