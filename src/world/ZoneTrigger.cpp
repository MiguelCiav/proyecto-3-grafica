#include "world/ZoneTrigger.hpp"
#include <iostream>

namespace world {

void ZoneTriggerManager::addZone(const SpecialZone& zone) {
    m_zones.push_back(zone);
}

void ZoneTriggerManager::clear() {
    m_zones.clear();
}

const SpecialZone* ZoneTriggerManager::checkTrigger(const glm::vec3& position) const {
    for (const auto& zone : m_zones) {
        if (zone.contains(position)) {
            return &zone;
        }
    }
    return nullptr;
}

void ZoneTriggerManager::autoGenerateFromGrid(const TrackGrid& grid) {
    clear();

    float halfTile = TrackGrid::TILE_SIZE * 0.5f;

    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::START_FINISH) {
            SpecialZone finishLine;
            finishLine.type = ZoneType::FINISH_LINE;
            finishLine.minBounds = tile.worldPos - glm::vec3(halfTile, 0.5f, halfTile);
            finishLine.maxBounds = tile.worldPos + glm::vec3(halfTile, 2.0f, halfTile);
            finishLine.speedMultiplier = 1.0f;
            finishLine.durationSeconds = 0.0f;
            addZone(finishLine);
            std::cout << "[OK] [ZoneTrigger] Zona de Meta registrada en ("
                      << tile.worldPos.x << ", " << tile.worldPos.z << ")" << std::endl;
        } else if (tile.surface == SurfaceType::BOOST_PAD) {
            SpecialZone boost;
            boost.type = ZoneType::BOOST;
            boost.minBounds = tile.worldPos - glm::vec3(halfTile, 0.5f, halfTile);
            boost.maxBounds = tile.worldPos + glm::vec3(halfTile, 2.0f, halfTile);
            boost.speedMultiplier = 1.6f;
            boost.durationSeconds = 2.0f;
            addZone(boost);
        }
    }

    std::cout << "[OK] [ZoneTrigger] Zonas especiales configuradas: " << m_zones.size() << std::endl;
}

} // namespace world
