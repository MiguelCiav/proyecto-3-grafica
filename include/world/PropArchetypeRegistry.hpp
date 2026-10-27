#pragma once

#include <string>
#include <glm/glm.hpp>
#include <cstdint>

namespace renderer {
class Shader;
}

namespace world {

struct PropInstance;

enum class PropRenderMode : uint8_t {
    TEXTURED,
    VERTEX_COLOR,
    SOLID_COLOR
};

enum class PropTypeId : uint16_t {
    CONE,
    BARRIER_RED,
    BARRIER_WHITE,
    BARRIER_WALL,
    BOX,
    DEBRIS_TIRE,

    LAMP_POLE,
    LAMP_ARM,
    LAMP_BULB,
    STADIUM_TOWER,
    TREE_TRUNK,
    TREE_FOLIAGE_BOTTOM,
    TREE_FOLIAGE_TOP,
    GANTRY_PILLAR,
    GANTRY_FOOTING,
    GANTRY_BEAM,
    GANTRY_LOWER_BEAM,
    GANTRY_SIGN,
    GANTRY_LIGHT,

    NEON_ARCH_PILLAR,
    NEON_ARCH_BEAM,
    NEON_ARCH_TRIM_CYAN,
    NEON_ARCH_TRIM_MAGENTA,
    NEON_BEACON,
    NEON_BEACON_TOP,

    CACTUS_TRUNK,
    CACTUS_ARM_JOINT,
    CACTUS_ARM_SPIRE,
    DESERT_ROCK,
    DESERT_MESA_ROCK,
    DESERT_ARCH_PILLAR,
    DESERT_ARCH_BEAM,

    SPACE_WARP_PILLAR,
    SPACE_WARP_BEAM,
    SPACE_WARP_TRIM_CYAN,
    SPACE_WARP_TRIM_MAGENTA,
    SATELLITE_BODY,
    SATELLITE_PANEL,
    SATELLITE_ANTENNA,
    SPACE_BEACON,
    SPACE_BEACON_TOP,
    FLOATING_STAR,

    CORAL_BRANCH_TALL,
    CORAL_BRANCH_SIDE,
    CORAL_BRAIN,
    KELP_SEGMENT,
    BUBBLE_COLUMN,
    SEA_ROCK,
    SEA_ARCH_PILLAR,
    SEA_ARCH_BEAM,
    SEA_ARCH_TRIM,
    SEA_PEDESTAL,
    CLAM_SHELL,
    SEA_PEARL,

    VOLCANO_ARCH_PILLAR,
    VOLCANO_ARCH_BEAM,
    VOLCANO_ARCH_TRIM_LAVA,
    BASALT_COLUMN,
    VOLCANO_ROCK,
    LAVA_CHIMNEY,
    LAVA_VENT,
    FIRE_BRAZIER,
    VOLCANO_FLAME,
    VOLCANO_FLAME_CORE,
    FLAME_ORB,

    UNKNOWN
};

struct PropArchetype {
    PropTypeId id{PropTypeId::UNKNOWN};
    std::string typeName;
    glm::vec3 defaultScale{1.0f};
    glm::vec3 aabbHalfExtents{0.0f};
    float aabbHeight{1.0f};
    bool hasCollisionDefault{true};
    bool hasAnimation{false};
    PropRenderMode renderMode{PropRenderMode::TEXTURED};
    glm::vec4 defaultColor{1.0f};
};

class PropArchetypeRegistry {
public:
    static const PropArchetype* getArchetype(const std::string& type);
    static const PropArchetype* getArchetype(PropTypeId id);
    static PropTypeId getTypeId(const std::string& type);

    static void calculateAABB(const std::string& type, const glm::vec3& pos,
                              float rotDeg, const glm::vec3& scale,
                              glm::vec3& outMin, glm::vec3& outMax);

    static void calculateAABB(PropTypeId id, const glm::vec3& pos,
                              float rotDeg, const glm::vec3& scale,
                              glm::vec3& outMin, glm::vec3& outMax);

    static void applyMaterial(const renderer::Shader& shader, const PropInstance& prop);
};

} // namespace world
