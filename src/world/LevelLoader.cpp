#include "world/LevelLoader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace world {

void LevelLoader::buildDefaultCircuit(TrackGrid& grid) {
    // Crear una grilla de 12x12 celdas
    grid.resize(12, 12);
    grid.clear();

    // Circuito cerrado tipo oval con rectas y 4 curvas en las esquinas:
    // Recta Superior (fila 2, cols 3 a 8) -> ROAD_STRAIGHT orientada horizontalmente (90°)
    for (int c = 3; c <= 8; ++c) {
        grid.setTile(c, 2, TileType::ROAD_STRAIGHT, 90.0f, SurfaceType::ASPHALT);
    }
    // Esquina Superior Derecha (col 9, fila 2) -> ROAD_CORNER_SMALL (rotación 0°)
    grid.setTile(9, 2, TileType::ROAD_CORNER_SMALL, 90.0f, SurfaceType::ASPHALT);

    // Recta Derecha (col 9, filas 3 a 8) -> ROAD_STRAIGHT vertical (0°)
    for (int r = 3; r <= 8; ++r) {
        grid.setTile(9, r, TileType::ROAD_STRAIGHT, 0.0f, SurfaceType::ASPHALT);
    }
    // Esquina Inferior Derecha (col 9, fila 9) -> ROAD_CORNER_SMALL (rotación 90°)
    grid.setTile(9, 9, TileType::ROAD_CORNER_SMALL, 0.0f, SurfaceType::ASPHALT);

    // Recta Inferior (fila 9, cols 8 hacia 3) -> ROAD_STRAIGHT horizontal (90°)
    for (int c = 8; c >= 4; --c) {
        grid.setTile(c, 9, TileType::ROAD_STRAIGHT, 90.0f, SurfaceType::ASPHALT);
    }
    // Zona de Meta / Start-Finish (col 3, fila 9)
    grid.setTile(3, 9, TileType::START_FINISH, 90.0f, SurfaceType::ASPHALT);

    // Esquina Inferior Izquierda (col 2, fila 9) -> ROAD_CORNER_SMALL (rotación 180°)
    grid.setTile(2, 9, TileType::ROAD_CORNER_SMALL, 270.0f, SurfaceType::ASPHALT);

    // Recta Izquierda (col 2, filas 8 hacia 3) -> ROAD_STRAIGHT vertical (0°)
    for (int r = 8; r >= 3; --r) {
        grid.setTile(2, r, TileType::ROAD_STRAIGHT, 0.0f, SurfaceType::ASPHALT);
    }
    // Esquina Superior Izquierda (col 2, fila 2) -> ROAD_CORNER_SMALL (rotación 270°)
    grid.setTile(2, 2, TileType::ROAD_CORNER_SMALL, 180.0f, SurfaceType::ASPHALT);

    grid.updateModelMatrices();
    std::cout << "[OK] [LevelLoader] Circuito cerrado por defecto construido exitosamente (12x12)." << std::endl;
}

bool LevelLoader::saveToFile(const std::string& filePath, const TrackGrid& grid) {
    std::ofstream out(filePath);
    if (!out.is_open()) return false;

    out << "# Mario Kart CGI - Track Layout File\n";
    out << "GRID " << grid.getCols() << " " << grid.getRows() << "\n";

    for (const auto& t : grid.getAllTiles()) {
        if (t.type != TileType::EMPTY) {
            out << "TILE " << t.col << " " << t.row << " "
                << static_cast<int>(t.type) << " "
                << t.rotationDegrees << " "
                << static_cast<int>(t.surface) << "\n";
        }
    }
    return true;
}

bool LevelLoader::loadFromFile(const std::string& filePath, TrackGrid& outGrid) {
    std::ifstream in(filePath);
    if (!in.is_open()) {
        std::cerr << "[AVISO] [LevelLoader] No se pudo abrir: " << filePath
                  << ", generando circuito cerrado por defecto." << std::endl;
        buildDefaultCircuit(outGrid);
        saveToFile(filePath, outGrid);
        return true;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "GRID") {
            int cols = 12, rows = 12;
            ss >> cols >> rows;
            outGrid.resize(cols, rows);
            outGrid.clear();
        } else if (tag == "TILE") {
            int col = 0, row = 0, typeInt = 0, surfInt = 0;
            float rot = 0.0f;
            ss >> col >> row >> typeInt >> rot >> surfInt;
            outGrid.setTile(col, row, static_cast<TileType>(typeInt), rot, static_cast<SurfaceType>(surfInt));
        }
    }

    outGrid.updateModelMatrices();
    std::cout << "[OK] [LevelLoader] Circuito cargado desde: " << filePath << std::endl;
    return true;
}

} // namespace world
