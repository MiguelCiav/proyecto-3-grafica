#pragma once

#include "world/TrackGrid.hpp"
#include <string>

namespace world {

class LevelLoader {
public:
    // Carga un circuito desde archivo de texto
    static bool loadFromFile(const std::string& filePath, TrackGrid& outGrid);

    // Guarda el circuito actual a archivo
    static bool saveToFile(const std::string& filePath, const TrackGrid& grid);

    // Genera un circuito cerrado de carreras de prueba (circuito oval con chicana)
    static void buildDefaultCircuit(TrackGrid& outGrid);
};

} // namespace world
