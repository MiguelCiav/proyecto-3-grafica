#pragma once

#include "world/TrackGrid.hpp"
#include <vector>
#include <glm/glm.hpp>

namespace world {

enum class ZoneType {
    BOOST,
    OIL,
    FINISH_LINE
};

struct SpecialZone {
    ZoneType type{ZoneType::BOOST};
    glm::vec3 minBounds{0.0f};
    glm::vec3 maxBounds{0.0f};
    float speedMultiplier{1.5f};
    float durationSeconds{2.0f};

    bool contains(const glm::vec3& point) const {
        return (point.x >= minBounds.x && point.x <= maxBounds.x &&
                point.y >= (minBounds.y - 0.5f) && point.y <= (maxBounds.y + 1.5f) &&
                point.z >= minBounds.z && point.z <= maxBounds.z);
    }
};

class ZoneTriggerManager {
public:
    void addZone(const SpecialZone& zone);
    void clear();

    const SpecialZone* checkTrigger(const glm::vec3& position) const;
    const std::vector<SpecialZone>& getZones() const { return m_zones; }

    // Genera automáticamente zonas de meta y boost a partir de las celdas del circuito
    void autoGenerateFromGrid(const TrackGrid& grid);

private:
    std::vector<SpecialZone> m_zones;
};

} // namespace world
