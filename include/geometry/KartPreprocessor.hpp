#pragma once

#include "geometry/KartMeshExtractor.hpp"
#include <string>

namespace geometry {

class KartPreprocessor {
public:
    // Carga, desensambla y recentra los pivotes de las 4 ruedas a (0,0,0)
    static bool processKart(const std::string& objFilePath, KartAssembly& outAssembly);

    // Valida y muestra el reporte de diagnóstico de las ruedas
    static bool validateAndReport(const KartAssembly& assembly, const std::string& kartName);
};

} // namespace geometry
