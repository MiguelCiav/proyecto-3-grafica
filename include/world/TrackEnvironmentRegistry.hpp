#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "geometry/KartMeshExtractor.hpp"
#include "world/Biome.hpp"

namespace world {

struct DriverProfile {
    std::string id;
    std::string name;
    std::string description;
    std::string sourcePath;
    geometry::SubMesh driverMesh;
    glm::vec4 accentColor{1.0f};
    bool loaded{false};
};

struct VehicleProfile {
    std::string id;
    std::string name;
    std::string category;
    std::string description;
    std::string modelPath;
    glm::vec3 driverOffset{0.0f};
    float maxSpeed{14.5f};
    float acceleration{11.5f};
    float turnSpeed{155.0f};
    float offRoadFriction{0.28f};
    geometry::KartAssembly assembly;
    bool loaded{false};
};

enum class TrackTheme {
    DAY_GRASSLAND,
    NIGHT_NEON,
    DESERT_DUNES,
    COSMIC_SPACE,
    AQUATIC_REEF,
    VOLCANO_CALDERA
};

struct TrackProfile {
    std::string id;
    std::string name;
    std::string difficulty;
    std::string description;
    std::string filePath;
    int cols{12};
    int rows{12};
    bool isNight{false};
    TrackTheme theme{TrackTheme::DAY_GRASSLAND};
};

class TrackEnvironmentRegistry {
public:
    static std::vector<DriverProfile> createDefaultDrivers();
    static std::vector<VehicleProfile> createDefaultVehicles();
    static std::vector<TrackProfile> createDefaultTracks();

    static BiomeType getBiomeForTrack(const TrackProfile& track);
    static glm::vec4 getGroundColorForTrack(const TrackProfile& track);
};

} // namespace world

namespace core {
using world::DriverProfile;
using world::VehicleProfile;
using world::TrackProfile;
using world::TrackTheme;
}
