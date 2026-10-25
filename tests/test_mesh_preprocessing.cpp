#include "geometry/KartPreprocessor.hpp"
#include "geometry/OBJLoader.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Preprocesamiento de Karts y Recentrado de Pivotes " << std::endl;
    std::cout << "  Fase 2 - Issue #8 (Regla Cero Blender)                  " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // 1. Probar kart-oobi
    geometry::KartAssembly kartOobi;
    bool successOobi = geometry::KartPreprocessor::processKart("assets/models/karts/kart-oobi.obj", kartOobi);
    if (!successOobi) {
        std::cerr << "[FAIL] No se pudo procesar kart-oobi.obj" << std::endl;
        return 1;
    }
    bool validOobi = geometry::KartPreprocessor::validateAndReport(kartOobi, "kart-oobi (Verde)");
    if (!validOobi) {
        std::cerr << "[FAIL] Validación fallida para kart-oobi.obj" << std::endl;
        return 1;
    }

    // 2. Probar kart-oodi
    geometry::KartAssembly kartOodi;
    bool successOodi = geometry::KartPreprocessor::processKart("assets/models/karts/kart-oodi.obj", kartOodi);
    if (!successOodi) {
        std::cerr << "[FAIL] No se pudo procesar kart-oodi.obj" << std::endl;
        return 1;
    }
    bool validOodi = geometry::KartPreprocessor::validateAndReport(kartOodi, "kart-oodi (Azul)");
    if (!validOodi) {
        std::cerr << "[FAIL] Validación fallida para kart-oodi.obj" << std::endl;
        return 1;
    }

    // 3. Probar carga de colores difusos de materiales nativos (.mtl) en piezas de pista
    geometry::ModelData straightData;
    bool loadedStraight = geometry::OBJLoader::load("assets/models/track/roadStraight.obj", straightData);
    if (!loadedStraight || straightData.submeshes.empty()) {
        std::cerr << "[FAIL] No se pudo cargar roadStraight.obj" << std::endl;
        return 1;
    }

    bool hasRoadColor = false;
    bool hasGreyColor = false;
    bool hasGrassColor = false;
    for (const auto& v : straightData.submeshes[0].vertices) {
        if (std::abs(v.color.r - 0.2666667f) < 0.01f && std::abs(v.color.g - 0.2666667f) < 0.01f) {
            hasRoadColor = true;
        }
        if (std::abs(v.color.r - 0.945098f) < 0.01f && std::abs(v.color.g - 0.9490196f) < 0.01f) {
            hasGreyColor = true;
        }
        if (std::abs(v.color.r - 0.3019608f) < 0.01f && std::abs(v.color.g - 0.5607843f) < 0.01f) {
            hasGrassColor = true;
        }
    }

    if (!hasRoadColor || !hasGreyColor || !hasGrassColor) {
        std::cerr << "[FAIL] Los colores de material de roadStraight (.mtl) no fueron asignados correctamente a los vértices." << std::endl;
        return 1;
    }
    std::cout << "  [CHECK 3] Materiales de pista (.mtl) cargados con colores de vertice independientes (Asfalto, Bordillo, Cesped) OK." << std::endl;

    std::cout << "[SUCCESS] Todos los karts procesados y validados con éxito. Hito 2 superado!" << std::endl;
    return 0;
}
