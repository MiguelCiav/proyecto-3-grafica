#include "world/PropArchetypeRegistry.hpp"
#include "world/PropSpawner.hpp"
#include "renderer/Shader.hpp"
#include <unordered_map>
#include <cmath>

namespace world {

namespace {

static const std::unordered_map<std::string, PropArchetype>& getArchetypeTable() {
    static const std::unordered_map<std::string, PropArchetype> s_table = {
        // Modelos Kenney
        { "cone", { PropTypeId::CONE, "cone", glm::vec3(1.0f), glm::vec3(0.24f, 0.0f, 0.24f), 0.62f, true, false, PropRenderMode::TEXTURED, glm::vec4(1.0f) } },
        { "barrierWall", { PropTypeId::BARRIER_WALL, "barrierWall", glm::vec3(1.0f), glm::vec3(0.50f, 0.0f, 0.14f), 0.55f, true, false, PropRenderMode::VERTEX_COLOR, glm::vec4(1.0f) } },
        { "barrierRed", { PropTypeId::BARRIER_RED, "barrierRed", glm::vec3(1.0f), glm::vec3(0.32f, 0.0f, 0.16f), 0.45f, true, false, PropRenderMode::VERTEX_COLOR, glm::vec4(1.0f) } },
        { "barrierWhite", { PropTypeId::BARRIER_WHITE, "barrierWhite", glm::vec3(1.0f), glm::vec3(0.32f, 0.0f, 0.16f), 0.45f, true, false, PropRenderMode::VERTEX_COLOR, glm::vec4(1.0f) } },
        { "box", { PropTypeId::BOX, "box", glm::vec3(1.0f), glm::vec3(0.35f, 0.0f, 0.35f), 0.70f, true, false, PropRenderMode::TEXTURED, glm::vec4(1.0f) } },
        { "debris-tire", { PropTypeId::DEBRIS_TIRE, "debris-tire", glm::vec3(1.0f), glm::vec3(0.28f, 0.0f, 0.28f), 0.55f, true, false, PropRenderMode::TEXTURED, glm::vec4(1.0f) } },

        // Alumbrado y Señalización
        { "lampPole", { PropTypeId::LAMP_POLE, "lampPole", glm::vec3(1.0f), glm::vec3(0.15f, 0.0f, 0.15f), 4.40f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.24f, 0.26f, 0.30f, 1.0f) } },
        { "lampArm", { PropTypeId::LAMP_ARM, "lampArm", glm::vec3(1.0f), glm::vec3(0.10f, 0.08f, 0.70f), 0.16f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.24f, 0.26f, 0.30f, 1.0f) } },
        { "lampBulb", { PropTypeId::LAMP_BULB, "lampBulb", glm::vec3(1.0f), glm::vec3(0.32f, 0.18f, 0.32f), 0.36f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(1.0f, 0.88f, 0.30f, 1.0f) } },
        { "stadiumTower", { PropTypeId::STADIUM_TOWER, "stadiumTower", glm::vec3(1.0f), glm::vec3(0.40f, 0.0f, 0.40f), 10.0f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.38f, 0.42f, 0.48f, 1.0f) } },
        { "treeTrunk", { PropTypeId::TREE_TRUNK, "treeTrunk", glm::vec3(1.0f), glm::vec3(0.20f, 0.0f, 0.20f), 1.80f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.42f, 0.28f, 0.16f, 1.0f) } },
        { "treeFoliageBottom", { PropTypeId::TREE_FOLIAGE_BOTTOM, "treeFoliageBottom", glm::vec3(1.0f), glm::vec3(0.85f, 0.55f, 0.85f), 1.10f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.18f, 0.48f, 0.22f, 1.0f) } },
        { "treeFoliageTop", { PropTypeId::TREE_FOLIAGE_TOP, "treeFoliageTop", glm::vec3(1.0f), glm::vec3(0.55f, 0.50f, 0.55f), 1.00f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.25f, 0.58f, 0.28f, 1.0f) } },
        { "gantryPillar", { PropTypeId::GANTRY_PILLAR, "gantryPillar", glm::vec3(1.0f), glm::vec3(0.18f, 0.0f, 0.18f), 4.60f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.24f, 0.26f, 0.30f, 1.0f) } },
        { "gantryFooting", { PropTypeId::GANTRY_FOOTING, "gantryFooting", glm::vec3(1.0f), glm::vec3(0.28f, 0.18f, 0.28f), 0.36f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.48f, 0.50f, 0.54f, 1.0f) } },
        { "gantryBeam", { PropTypeId::GANTRY_BEAM, "gantryBeam", glm::vec3(1.0f), glm::vec3(2.70f, 0.16f, 0.20f), 0.32f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.24f, 0.26f, 0.30f, 1.0f) } },
        { "gantryLowerBeam", { PropTypeId::GANTRY_LOWER_BEAM, "gantryLowerBeam", glm::vec3(1.0f), glm::vec3(2.40f, 0.08f, 0.12f), 0.16f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.24f, 0.26f, 0.30f, 1.0f) } },
        { "gantrySign", { PropTypeId::GANTRY_SIGN, "gantrySign", glm::vec3(1.0f), glm::vec3(1.65f, 0.35f, 0.06f), 0.70f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.92f, 0.18f, 0.18f, 1.0f) } },
        { "gantryLight", { PropTypeId::GANTRY_LIGHT, "gantryLight", glm::vec3(1.0f), glm::vec3(0.12f, 0.12f, 0.12f), 0.24f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(1.0f, 0.18f, 0.18f, 1.0f) } },

        // Nocturno
        { "neonArchPillar", { PropTypeId::NEON_ARCH_PILLAR, "neonArchPillar", glm::vec3(1.0f), glm::vec3(0.18f, 0.0f, 0.18f), 4.60f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.12f, 0.14f, 0.18f, 1.0f) } },
        { "neonArchBeam", { PropTypeId::NEON_ARCH_BEAM, "neonArchBeam", glm::vec3(1.0f), glm::vec3(2.70f, 0.16f, 0.20f), 0.32f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.12f, 0.14f, 0.18f, 1.0f) } },
        { "neonArchTrimCyan", { PropTypeId::NEON_ARCH_TRIM_CYAN, "neonArchTrimCyan", glm::vec3(1.0f), glm::vec3(2.70f, 0.06f, 0.24f), 0.12f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.0f, 0.95f, 1.0f, 1.0f) } },
        { "neonArchTrimMagenta", { PropTypeId::NEON_ARCH_TRIM_MAGENTA, "neonArchTrimMagenta", glm::vec3(1.0f), glm::vec3(2.70f, 0.06f, 0.24f), 0.12f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(1.0f, 0.12f, 0.72f, 1.0f) } },
        { "neonBeacon", { PropTypeId::NEON_BEACON, "neonBeacon", glm::vec3(1.0f), glm::vec3(0.16f, 0.0f, 0.16f), 0.85f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.20f, 0.20f, 0.22f, 1.0f) } },
        { "neonBeaconTop", { PropTypeId::NEON_BEACON_TOP, "neonBeaconTop", glm::vec3(1.0f), glm::vec3(0.18f, 0.12f, 0.18f), 0.24f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(1.0f, 0.50f, 0.05f, 1.0f) } },

        // Desierto
        { "cactusTrunk", { PropTypeId::CACTUS_TRUNK, "cactusTrunk", glm::vec3(1.0f), glm::vec3(0.22f, 0.0f, 0.22f), 2.60f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.20f, 0.48f, 0.22f, 1.0f) } },
        { "cactusArmJoint", { PropTypeId::CACTUS_ARM_JOINT, "cactusArmJoint", glm::vec3(1.0f), glm::vec3(0.24f, 0.10f, 0.10f), 0.20f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.20f, 0.48f, 0.22f, 1.0f) } },
        { "cactusArmSpire", { PropTypeId::CACTUS_ARM_SPIRE, "cactusArmSpire", glm::vec3(1.0f), glm::vec3(0.12f, 0.50f, 0.12f), 1.00f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.20f, 0.48f, 0.22f, 1.0f) } },
        { "desertRock", { PropTypeId::DESERT_ROCK, "desertRock", glm::vec3(1.0f), glm::vec3(0.65f, 0.0f, 0.65f), 1.10f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.76f, 0.46f, 0.28f, 1.0f) } },
        { "desertMesaRock", { PropTypeId::DESERT_MESA_ROCK, "desertMesaRock", glm::vec3(1.0f), glm::vec3(1.20f, 0.0f, 1.20f), 1.70f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.76f, 0.46f, 0.28f, 1.0f) } },
        { "desertArchPillar", { PropTypeId::DESERT_ARCH_PILLAR, "desertArchPillar", glm::vec3(1.0f), glm::vec3(0.42f, 0.0f, 0.42f), 5.0f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.70f, 0.42f, 0.25f, 1.0f) } },
        { "desertArchBeam", { PropTypeId::DESERT_ARCH_BEAM, "desertArchBeam", glm::vec3(1.0f), glm::vec3(2.90f, 0.35f, 0.45f), 0.70f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.70f, 0.42f, 0.25f, 1.0f) } },

        // Espacio
        { "spaceWarpPillar", { PropTypeId::SPACE_WARP_PILLAR, "spaceWarpPillar", glm::vec3(1.0f), glm::vec3(0.20f, 0.0f, 0.20f), 4.80f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.85f, 0.88f, 0.94f, 1.0f) } },
        { "spaceWarpBeam", { PropTypeId::SPACE_WARP_BEAM, "spaceWarpBeam", glm::vec3(1.0f), glm::vec3(2.65f, 0.15f, 0.20f), 0.30f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.85f, 0.88f, 0.94f, 1.0f) } },
        { "spaceWarpTrimCyan", { PropTypeId::SPACE_WARP_TRIM_CYAN, "spaceWarpTrimCyan", glm::vec3(1.0f), glm::vec3(2.65f, 0.08f, 0.24f), 0.16f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.0f, 0.95f, 1.0f, 1.0f) } },
        { "spaceWarpTrimMagenta", { PropTypeId::SPACE_WARP_TRIM_MAGENTA, "spaceWarpTrimMagenta", glm::vec3(1.0f), glm::vec3(2.65f, 0.08f, 0.24f), 0.16f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(1.0f, 0.15f, 0.85f, 1.0f) } },
        { "satelliteBody", { PropTypeId::SATELLITE_BODY, "satelliteBody", glm::vec3(1.0f), glm::vec3(0.40f, 0.40f, 0.40f), 0.80f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.95f, 0.80f, 0.25f, 1.0f) } },
        { "satellitePanel", { PropTypeId::SATELLITE_PANEL, "satellitePanel", glm::vec3(1.0f), glm::vec3(1.10f, 0.04f, 0.35f), 0.08f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.10f, 0.35f, 0.85f, 1.0f) } },
        { "satelliteAntenna", { PropTypeId::SATELLITE_ANTENNA, "satelliteAntenna", glm::vec3(1.0f), glm::vec3(0.04f, 0.70f, 0.04f), 1.40f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.95f, 0.80f, 0.25f, 1.0f) } },
        { "spaceBeacon", { PropTypeId::SPACE_BEACON, "spaceBeacon", glm::vec3(1.0f), glm::vec3(0.18f, 0.0f, 0.18f), 0.90f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.20f, 0.22f, 0.28f, 1.0f) } },
        { "spaceBeaconTop", { PropTypeId::SPACE_BEACON_TOP, "spaceBeaconTop", glm::vec3(1.0f), glm::vec3(0.20f, 0.14f, 0.20f), 0.28f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.10f, 0.90f, 1.0f, 1.0f) } },
        { "floatingStar", { PropTypeId::FLOATING_STAR, "floatingStar", glm::vec3(1.0f), glm::vec3(0.55f, 0.55f, 0.18f), 1.10f, false, true, PropRenderMode::SOLID_COLOR, glm::vec4(0.0f, 0.95f, 1.0f, 1.0f) } },

        // Acuático
        { "coralBranchTall", { PropTypeId::CORAL_BRANCH_TALL, "coralBranchTall", glm::vec3(1.0f), glm::vec3(0.20f, 0.0f, 0.20f), 2.20f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(1.0f, 0.45f, 0.18f, 1.0f) } },
        { "coralBranchSide", { PropTypeId::CORAL_BRANCH_SIDE, "coralBranchSide", glm::vec3(1.0f), glm::vec3(0.20f, 0.0f, 0.20f), 2.20f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(1.0f, 0.45f, 0.18f, 1.0f) } },
        { "coralBrain", { PropTypeId::CORAL_BRAIN, "coralBrain", glm::vec3(1.0f), glm::vec3(0.52f, 0.0f, 0.52f), 0.55f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.85f, 0.22f, 0.65f, 1.0f) } },
        { "kelpSegment", { PropTypeId::KELP_SEGMENT, "kelpSegment", glm::vec3(1.0f), glm::vec3(0.24f, 0.85f, 0.06f), 1.70f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.08f, 0.68f, 0.38f, 1.0f) } },
        { "bubbleColumn", { PropTypeId::BUBBLE_COLUMN, "bubbleColumn", glm::vec3(1.0f), glm::vec3(0.30f, 1.0f, 0.30f), 2.0f, false, true, PropRenderMode::SOLID_COLOR, glm::vec4(0.60f, 0.94f, 1.0f, 0.75f) } },
        { "seaRock", { PropTypeId::SEA_ROCK, "seaRock", glm::vec3(1.0f), glm::vec3(0.70f, 0.0f, 0.70f), 1.10f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.12f, 0.22f, 0.28f, 1.0f) } },
        { "seaArchPillar", { PropTypeId::SEA_ARCH_PILLAR, "seaArchPillar", glm::vec3(1.0f), glm::vec3(0.24f, 0.0f, 0.24f), 5.0f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.18f, 0.42f, 0.48f, 1.0f) } },
        { "seaArchBeam", { PropTypeId::SEA_ARCH_BEAM, "seaArchBeam", glm::vec3(1.0f), glm::vec3(2.80f, 0.25f, 0.30f), 0.50f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.18f, 0.42f, 0.48f, 1.0f) } },
        { "seaArchTrim", { PropTypeId::SEA_ARCH_TRIM, "seaArchTrim", glm::vec3(1.0f), glm::vec3(2.80f, 0.08f, 0.36f), 0.16f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.45f, 0.95f, 0.90f, 1.0f) } },
        { "seaPedestal", { PropTypeId::SEA_PEDESTAL, "seaPedestal", glm::vec3(1.0f), glm::vec3(0.24f, 0.0f, 0.24f), 1.50f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.15f, 0.25f, 0.30f, 1.0f) } },
        { "clamShell", { PropTypeId::CLAM_SHELL, "clamShell", glm::vec3(1.0f), glm::vec3(0.52f, 0.25f, 0.52f), 0.50f, false, true, PropRenderMode::SOLID_COLOR, glm::vec4(0.85f, 0.82f, 0.88f, 1.0f) } },
        { "seaPearl", { PropTypeId::SEA_PEARL, "seaPearl", glm::vec3(1.0f), glm::vec3(0.18f, 0.18f, 0.18f), 0.36f, false, true, PropRenderMode::SOLID_COLOR, glm::vec4(0.20f, 0.98f, 0.88f, 1.0f) } },

        // Volcánico
        { "volcanoArchPillar", { PropTypeId::VOLCANO_ARCH_PILLAR, "volcanoArchPillar", glm::vec3(1.0f), glm::vec3(0.26f, 0.0f, 0.26f), 5.20f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.12f, 0.11f, 0.13f, 1.0f) } },
        { "volcanoArchBeam", { PropTypeId::VOLCANO_ARCH_BEAM, "volcanoArchBeam", glm::vec3(1.0f), glm::vec3(2.80f, 0.25f, 0.32f), 0.50f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.12f, 0.11f, 0.13f, 1.0f) } },
        { "volcanoArchTrimLava", { PropTypeId::VOLCANO_ARCH_TRIM_LAVA, "volcanoArchTrimLava", glm::vec3(1.0f), glm::vec3(2.80f, 0.08f, 0.36f), 0.16f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(1.0f, 0.40f, 0.05f, 1.0f) } },
        { "basaltColumn", { PropTypeId::BASALT_COLUMN, "basaltColumn", glm::vec3(1.0f), glm::vec3(0.38f, 0.0f, 0.38f), 3.20f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.16f, 0.15f, 0.18f, 1.0f) } },
        { "volcanoRock", { PropTypeId::VOLCANO_ROCK, "volcanoRock", glm::vec3(1.0f), glm::vec3(0.68f, 0.0f, 0.68f), 1.05f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.24f, 0.16f, 0.14f, 1.0f) } },
        { "lavaChimney", { PropTypeId::LAVA_CHIMNEY, "lavaChimney", glm::vec3(1.0f), glm::vec3(0.28f, 0.0f, 0.28f), 2.20f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.20f, 0.14f, 0.12f, 1.0f) } },
        { "lavaVent", { PropTypeId::LAVA_VENT, "lavaVent", glm::vec3(1.0f), glm::vec3(0.40f, 0.08f, 0.40f), 0.16f, false, false, PropRenderMode::SOLID_COLOR, glm::vec4(1.0f, 0.55f, 0.08f, 1.0f) } },
        { "fireBrazier", { PropTypeId::FIRE_BRAZIER, "fireBrazier", glm::vec3(1.0f), glm::vec3(0.24f, 0.0f, 0.24f), 1.40f, true, false, PropRenderMode::SOLID_COLOR, glm::vec4(0.16f, 0.15f, 0.17f, 1.0f) } },
        { "volcanoFlame", { PropTypeId::VOLCANO_FLAME, "volcanoFlame", glm::vec3(1.0f), glm::vec3(0.36f, 1.05f, 0.36f), 2.10f, false, true, PropRenderMode::SOLID_COLOR, glm::vec4(1.0f, 0.38f, 0.04f, 1.0f) } },
        { "volcanoFlameCore", { PropTypeId::VOLCANO_FLAME_CORE, "volcanoFlameCore", glm::vec3(1.0f), glm::vec3(0.22f, 0.72f, 0.22f), 1.44f, false, true, PropRenderMode::SOLID_COLOR, glm::vec4(1.0f, 0.88f, 0.18f, 1.0f) } },
        { "flameOrb", { PropTypeId::FLAME_ORB, "flameOrb", glm::vec3(1.0f), glm::vec3(0.36f, 1.05f, 0.36f), 2.10f, false, true, PropRenderMode::SOLID_COLOR, glm::vec4(1.0f, 0.38f, 0.04f, 1.0f) } }
    };
    return s_table;
}

static const std::unordered_map<PropTypeId, const PropArchetype*>& getIdTable() {
    static const std::unordered_map<PropTypeId, const PropArchetype*> s_idTable = []() {
        std::unordered_map<PropTypeId, const PropArchetype*> map;
        for (const auto& [name, arch] : getArchetypeTable()) {
            map[arch.id] = &arch;
        }
        return map;
    }();
    return s_idTable;
}

} // namespace

const PropArchetype* PropArchetypeRegistry::getArchetype(const std::string& type) {
    const auto& table = getArchetypeTable();
    auto it = table.find(type);
    if (it != table.end()) {
        return &it->second;
    }
    return nullptr;
}

const PropArchetype* PropArchetypeRegistry::getArchetype(PropTypeId id) {
    const auto& idTable = getIdTable();
    auto it = idTable.find(id);
    if (it != idTable.end()) {
        return it->second;
    }
    return nullptr;
}

PropTypeId PropArchetypeRegistry::getTypeId(const std::string& type) {
    const PropArchetype* arch = getArchetype(type);
    return arch ? arch->id : PropTypeId::UNKNOWN;
}

void PropArchetypeRegistry::calculateAABB(const std::string& type, const glm::vec3& pos,
                                          float rotDeg, const glm::vec3& scale,
                                          glm::vec3& outMin, glm::vec3& outMax) {
    calculateAABB(getTypeId(type), pos, rotDeg, scale, outMin, outMax);
}

void PropArchetypeRegistry::calculateAABB(PropTypeId id, const glm::vec3& pos,
                                          float rotDeg, const glm::vec3& scale,
                                          glm::vec3& outMin, glm::vec3& outMax) {
    switch (id) {
    case PropTypeId::CONE: {
        float r = 0.24f * scale.x;
        outMin = glm::vec3(pos.x - r, 0.0f, pos.z - r);
        outMax = glm::vec3(pos.x + r, 0.62f * scale.y, pos.z + r);
        break;
    }
    case PropTypeId::BARRIER_WALL: {
        float hx = 0.50f * scale.x;
        float hz = 0.14f * scale.z;
        if (std::abs(rotDeg - 90.0f) < 5.0f || std::abs(rotDeg - 270.0f) < 5.0f) {
            std::swap(hx, hz);
        }
        outMin = glm::vec3(pos.x - hx, 0.0f, pos.z - hz);
        outMax = glm::vec3(pos.x + hx, 0.55f * scale.y, pos.z + hz);
        break;
    }
    case PropTypeId::BARRIER_RED:
    case PropTypeId::BARRIER_WHITE: {
        float hx = 0.32f * scale.x;
        float hz = 0.16f * scale.z;
        outMin = glm::vec3(pos.x - hx, 0.0f, pos.z - hz);
        outMax = glm::vec3(pos.x + hx, 0.45f * scale.y, pos.z + hz);
        break;
    }
    case PropTypeId::BOX: {
        float hs = 0.35f * scale.x;
        outMin = glm::vec3(pos.x - hs, 0.0f, pos.z - hs);
        outMax = glm::vec3(pos.x + hs, 0.70f * scale.y, pos.z + hs);
        break;
    }
    case PropTypeId::DEBRIS_TIRE: {
        float r = 0.28f * scale.x;
        outMin = glm::vec3(pos.x - r, 0.0f, pos.z - r);
        outMax = glm::vec3(pos.x + r, 0.55f * scale.y, pos.z + r);
        break;
    }
    case PropTypeId::LAMP_POLE: {
        outMin = glm::vec3(pos.x - 0.15f, 0.0f, pos.z - 0.15f);
        outMax = glm::vec3(pos.x + 0.15f, 4.40f * scale.y, pos.z + 0.15f);
        break;
    }
    case PropTypeId::TREE_TRUNK: {
        outMin = glm::vec3(pos.x - 0.20f, 0.0f, pos.z - 0.20f);
        outMax = glm::vec3(pos.x + 0.20f, 1.80f * scale.y, pos.z + 0.20f);
        break;
    }
    case PropTypeId::STADIUM_TOWER: {
        outMin = glm::vec3(pos.x - 0.40f, 0.0f, pos.z - 0.40f);
        outMax = glm::vec3(pos.x + 0.40f, 10.0f * scale.y, pos.z + 0.40f);
        break;
    }
    case PropTypeId::GANTRY_PILLAR:
    case PropTypeId::NEON_ARCH_PILLAR: {
        outMin = glm::vec3(pos.x - 0.18f, 0.0f, pos.z - 0.18f);
        outMax = glm::vec3(pos.x + 0.18f, 4.60f * scale.y, pos.z + 0.18f);
        break;
    }
    case PropTypeId::NEON_BEACON: {
        outMin = glm::vec3(pos.x - 0.16f, 0.0f, pos.z - 0.16f);
        outMax = glm::vec3(pos.x + 0.16f, 0.85f * scale.y, pos.z + 0.16f);
        break;
    }
    case PropTypeId::CACTUS_TRUNK: {
        outMin = glm::vec3(pos.x - 0.22f, 0.0f, pos.z - 0.22f);
        outMax = glm::vec3(pos.x + 0.22f, 2.60f * scale.y, pos.z + 0.22f);
        break;
    }
    case PropTypeId::DESERT_ROCK: {
        outMin = glm::vec3(pos.x - 0.65f * scale.x, 0.0f, pos.z - 0.65f * scale.z);
        outMax = glm::vec3(pos.x + 0.65f * scale.x, 1.10f * scale.y, pos.z + 0.65f * scale.z);
        break;
    }
    case PropTypeId::DESERT_MESA_ROCK: {
        outMin = glm::vec3(pos.x - 1.20f * scale.x, 0.0f, pos.z - 1.20f * scale.z);
        outMax = glm::vec3(pos.x + 1.20f * scale.x, 1.70f * scale.y, pos.z + 1.20f * scale.z);
        break;
    }
    case PropTypeId::DESERT_ARCH_PILLAR: {
        outMin = glm::vec3(pos.x - 0.42f, 0.0f, pos.z - 0.42f);
        outMax = glm::vec3(pos.x + 0.42f, 5.0f * scale.y, pos.z + 0.42f);
        break;
    }
    case PropTypeId::SPACE_WARP_PILLAR: {
        outMin = glm::vec3(pos.x - 0.20f, 0.0f, pos.z - 0.20f);
        outMax = glm::vec3(pos.x + 0.20f, 4.80f * scale.y, pos.z + 0.20f);
        break;
    }
    case PropTypeId::SPACE_BEACON: {
        outMin = glm::vec3(pos.x - 0.18f, 0.0f, pos.z - 0.18f);
        outMax = glm::vec3(pos.x + 0.18f, 0.90f * scale.y, pos.z + 0.18f);
        break;
    }
    case PropTypeId::SEA_ARCH_PILLAR: {
        outMin = glm::vec3(pos.x - 0.24f, 0.0f, pos.z - 0.24f);
        outMax = glm::vec3(pos.x + 0.24f, 5.0f * scale.y, pos.z + 0.24f);
        break;
    }
    case PropTypeId::SEA_ROCK: {
        outMin = glm::vec3(pos.x - 0.70f * scale.x, 0.0f, pos.z - 0.70f * scale.z);
        outMax = glm::vec3(pos.x + 0.70f * scale.x, 1.10f * scale.y, pos.z + 0.70f * scale.z);
        break;
    }
    case PropTypeId::SEA_PEDESTAL: {
        outMin = glm::vec3(pos.x - 0.24f, 0.0f, pos.z - 0.24f);
        outMax = glm::vec3(pos.x + 0.24f, 1.50f * scale.y, pos.z + 0.24f);
        break;
    }
    case PropTypeId::CORAL_BRANCH_TALL:
    case PropTypeId::CORAL_BRANCH_SIDE: {
        outMin = glm::vec3(pos.x - 0.20f * scale.x, 0.0f, pos.z - 0.20f * scale.z);
        outMax = glm::vec3(pos.x + 0.20f * scale.x, 2.20f * scale.y, pos.z + 0.20f * scale.z);
        break;
    }
    case PropTypeId::CORAL_BRAIN: {
        outMin = glm::vec3(pos.x - 0.52f * scale.x, 0.0f, pos.z - 0.52f * scale.z);
        outMax = glm::vec3(pos.x + 0.52f * scale.x, 0.55f * scale.y, pos.z + 0.52f * scale.z);
        break;
    }
    case PropTypeId::VOLCANO_ARCH_PILLAR: {
        outMin = glm::vec3(pos.x - 0.26f, 0.0f, pos.z - 0.26f);
        outMax = glm::vec3(pos.x + 0.26f, 5.20f * scale.y, pos.z + 0.26f);
        break;
    }
    case PropTypeId::BASALT_COLUMN: {
        outMin = glm::vec3(pos.x - 0.38f * scale.x, 0.0f, pos.z - 0.38f * scale.z);
        outMax = glm::vec3(pos.x + 0.38f * scale.x, 3.20f * scale.y, pos.z + 0.38f * scale.z);
        break;
    }
    case PropTypeId::VOLCANO_ROCK: {
        outMin = glm::vec3(pos.x - 0.68f * scale.x, 0.0f, pos.z - 0.68f * scale.z);
        outMax = glm::vec3(pos.x + 0.68f * scale.x, 1.05f * scale.y, pos.z + 0.68f * scale.z);
        break;
    }
    case PropTypeId::LAVA_CHIMNEY: {
        outMin = glm::vec3(pos.x - 0.28f * scale.x, 0.0f, pos.z - 0.28f * scale.z);
        outMax = glm::vec3(pos.x + 0.28f * scale.x, 2.20f * scale.y, pos.z + 0.28f * scale.z);
        break;
    }
    case PropTypeId::FIRE_BRAZIER: {
        outMin = glm::vec3(pos.x - 0.24f * scale.x, 0.0f, pos.z - 0.24f * scale.z);
        outMax = glm::vec3(pos.x + 0.24f * scale.x, 1.40f * scale.y, pos.z + 0.24f * scale.z);
        break;
    }
    default: {
        glm::vec3 halfSize = scale * 0.5f;
        outMin = pos - halfSize;
        outMax = pos + halfSize;
        break;
    }
    }
}

void PropArchetypeRegistry::applyMaterial(const renderer::Shader& shader, const PropInstance& prop) {
    if (prop.useCustomColor) {
        shader.setBool("u_UseVertexColor", false);
        shader.setBool("u_UseTexture", false);
        shader.setVec4("u_Color", prop.customColor);
        return;
    }

    const PropArchetype* arch = getArchetype(prop.type);
    if (!arch) {
        shader.setBool("u_UseVertexColor", false);
        shader.setBool("u_UseTexture", true);
        shader.setVec4("u_Color", glm::vec4(1.0f));
        return;
    }

    switch (arch->renderMode) {
    case PropRenderMode::VERTEX_COLOR:
        shader.setBool("u_UseTexture", false);
        shader.setBool("u_UseVertexColor", true);
        shader.setVec4("u_Color", glm::vec4(1.0f));
        break;
    case PropRenderMode::SOLID_COLOR:
        shader.setBool("u_UseVertexColor", false);
        shader.setBool("u_UseTexture", false);
        shader.setVec4("u_Color", arch->defaultColor);
        break;
    case PropRenderMode::TEXTURED:
    default:
        shader.setBool("u_UseVertexColor", false);
        shader.setBool("u_UseTexture", true);
        shader.setVec4("u_Color", glm::vec4(1.0f));
        break;
    }
}

} // namespace world
