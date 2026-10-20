#include "geometry/KartPreprocessor.hpp"
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

    std::cout << "[SUCCESS] Todos los karts procesados y validados con éxito. Hito 2 superado!" << std::endl;
    return 0;
}
