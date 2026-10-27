#include "world/BiomeSpawners.hpp"
#include "world/PropSpawner.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace world {

std::unique_ptr<IBiomePropSpawner> BiomePropSpawnerFactory::create(BiomeType biome) {
    switch (biome) {
    case BiomeType::GRASS_NIGHT:
        return std::make_unique<GrasslandPropSpawner>(true);
    case BiomeType::DESERT:
        return std::make_unique<DesertPropSpawner>();
    case BiomeType::SPACE:
        return std::make_unique<SpacePropSpawner>();
    case BiomeType::AQUATIC:
        return std::make_unique<AquaticPropSpawner>();
    case BiomeType::VOLCANO:
        return std::make_unique<VolcanoPropSpawner>();
    case BiomeType::GRASS_DAY:
    default:
        return std::make_unique<GrasslandPropSpawner>(false);
    }
}

// ==============================================================================
// 1. Grassland (Día y Noche)
// ==============================================================================

GrasslandPropSpawner::GrasslandPropSpawner(bool isNight)
    : m_isNight(isNight) {}

void GrasslandPropSpawner::spawnProps(const TrackGrid& grid, PropSpawner& spawner) {
    float halfTile = TrackGrid::TILE_SIZE * 0.5f;
    int propIndex = 0;

    std::vector<const TileData*> straightTiles;
    const TileData* startFinishTile = nullptr;

    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::START_FINISH) {
            startFinishTile = &tile;
        } else if (tile.type == TileType::ROAD_STRAIGHT) {
            straightTiles.push_back(&tile);
        }
    }

    // 1. Arco de Meta / Gantry Monumental sobre la línea de meta
    if (startFinishTile != nullptr) {
        bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
        glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        float beamRot = isHorizontal ? 90.0f : 0.0f;
        float signRot = isHorizontal ? 90.0f : 0.0f;

        // Pilares izquierdo y derecho anclados en el césped a cada lado del carril (4.7m de luz)
        glm::vec3 leftPillar = startFinishTile->worldPos + sideDir * 2.35f + glm::vec3(0.0f, 2.30f, 0.0f);
        glm::vec3 rightPillar = startFinishTile->worldPos - sideDir * 2.35f + glm::vec3(0.0f, 2.30f, 0.0f);
        spawner.addProp("gantryPillar", leftPillar, 0.0f, glm::vec3(1.0f), true);
        spawner.addProp("gantryPillar", rightPillar, 0.0f, glm::vec3(1.0f), true);

        // Zapatas de hormigón en la base de los pilares (sin colisión separada del pilar)
        glm::vec3 leftFoot = startFinishTile->worldPos + sideDir * 2.35f + glm::vec3(0.0f, 0.18f, 0.0f);
        glm::vec3 rightFoot = startFinishTile->worldPos - sideDir * 2.35f + glm::vec3(0.0f, 0.18f, 0.0f);
        spawner.addProp("gantryFooting", leftFoot, 0.0f, glm::vec3(1.0f), false);
        spawner.addProp("gantryFooting", rightFoot, 0.0f, glm::vec3(1.0f), false);

        // Viga transversal superior en celosía descansando sólidamente sobre los pilares a 4.76m de altura (sin colisión con vehículos)
        glm::vec3 beamPos = startFinishTile->worldPos + glm::vec3(0.0f, 4.76f, 0.0f);
        spawner.addProp("gantryBeam", beamPos, beamRot, glm::vec3(1.0f), false);

        // Viga inferior de soporte y unión a 3.90m
        glm::vec3 lowerBeamPos = startFinishTile->worldPos + glm::vec3(0.0f, 3.90f, 0.0f);
        spawner.addProp("gantryLowerBeam", lowerBeamPos, beamRot, glm::vec3(1.0f), false);

        // Letrero luminoso de START / FINISH en el centro entre ambas vigas
        glm::vec3 signPos = startFinishTile->worldPos + glm::vec3(0.0f, 4.33f, 0.0f);
        spawner.addProp("gantrySign", signPos, signRot, glm::vec3(1.0f), false);

        // 5 Luces de semáforo de salida de carrera F1 suspendidas bajo la estructura
        for (int i = -2; i <= 2; ++i) {
            glm::vec3 lightPos = startFinishTile->worldPos + sideDir * (i * 0.60f) + glm::vec3(0.0f, 3.72f, 0.0f);
            spawner.addProp("gantryLight", lightPos, beamRot, glm::vec3(1.0f), false);
        }
    }

    // 2. Generar Farolas 3D Ampliamente Espaciadas (Mínimo 28m de separación entre postes)
    if (!straightTiles.empty()) {
        const float MIN_LAMP_DIST = 28.0f;
        const size_t MAX_LAMPS = 6;

        std::vector<const TileData*> selectedLamps;
        size_t stride = std::max<size_t>(1, straightTiles.size() / 16);
        for (size_t i = 0; i < straightTiles.size() && selectedLamps.size() < MAX_LAMPS; i += stride) {
            const auto* cand = straightTiles[i];
            bool tooClose = false;
            for (const auto* existing : selectedLamps) {
                if (glm::distance(cand->worldPos, existing->worldPos) < MIN_LAMP_DIST) {
                    tooClose = true;
                    break;
                }
            }
            if (!tooClose) {
                selectedLamps.push_back(cand);
            }
        }

        for (size_t i = 0; i < selectedLamps.size(); ++i) {
            const auto* tile = selectedLamps[i];
            bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
            float sideSign = (i % 2 == 0) ? 1.0f : -1.0f;
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);

            glm::vec3 poleBase = tile->worldPos + sideDir * (halfTile * 1.15f);
            glm::vec3 poleCenter = poleBase + glm::vec3(0.0f, 2.1f, 0.0f);
            spawner.addProp("lampPole", poleCenter, 0.0f, glm::vec3(1.0f), true);

            spawner.addProp("debris-tire", poleBase + glm::vec3(0.0f, 0.25f, 0.0f), 0.0f, glm::vec3(1.0f), true);

            glm::vec3 armPos = poleBase + glm::vec3(0.0f, 4.2f, 0.0f) - sideDir * 0.55f;
            float armRot = isHorizontal ? (sideSign > 0 ? 180.0f : 0.0f) : (sideSign > 0 ? 270.0f : 90.0f);
            spawner.addProp("lampArm", armPos, armRot, glm::vec3(1.0f), false);

            glm::vec3 bulbPos = poleBase + glm::vec3(0.0f, 4.1f, 0.0f) - sideDir * 1.15f;
            spawner.addProp("lampBulb", bulbPos, 0.0f, glm::vec3(1.0f), false);

            spawner.addLampPosition(bulbPos);
        }
    }

    // 3. Arcos de Neón Luminosos Exclusivos para Circuitos Nocturnos
    if (m_isNight && straightTiles.size() >= 6) {
        size_t idx1 = straightTiles.size() / 3;
        size_t idx2 = (straightTiles.size() * 2) / 3;
        const TileData* neonTiles[2] = { straightTiles[idx1], straightTiles[idx2] };
        const char* trims[2] = { "neonArchTrimCyan", "neonArchTrimMagenta" };

        for (int k = 0; k < 2; ++k) {
            const auto* nTile = neonTiles[k];
            bool isH = (nTile->rotationDegrees == 90.0f || nTile->rotationDegrees == 270.0f);
            glm::vec3 sDir = isH ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
            float bRot = isH ? 90.0f : 0.0f;

            glm::vec3 lp = nTile->worldPos + sDir * 2.35f + glm::vec3(0.0f, 2.30f, 0.0f);
            glm::vec3 rp = nTile->worldPos - sDir * 2.35f + glm::vec3(0.0f, 2.30f, 0.0f);
            spawner.addProp("neonArchPillar", lp, 0.0f, glm::vec3(1.0f), true);
            spawner.addProp("neonArchPillar", rp, 0.0f, glm::vec3(1.0f), true);

            glm::vec3 bp = nTile->worldPos + glm::vec3(0.0f, 4.60f, 0.0f);
            spawner.addProp("neonArchBeam", bp, bRot, glm::vec3(1.0f), false);

            glm::vec3 tp = nTile->worldPos + glm::vec3(0.0f, 4.40f, 0.0f);
            spawner.addProp(trims[k], tp, bRot, glm::vec3(1.0f), false);
        }
    }

    // 4. Elementos de Pista (Curvas, Rectas, Meta)
    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::EMPTY) continue;

        // Curvas (fuera de la calzada de asfalto):
        if (tile.type == TileType::ROAD_CORNER_SMALL) {
            float rot = tile.rotationDegrees;
            float rotRad = glm::radians(rot);
            glm::vec3 cornerPos = tile.worldPos;
            glm::vec3 outerDir(std::cos(rotRad + 0.785f), 0.0f, std::sin(rotRad + 0.785f));

            if (m_isNight) {
                glm::vec3 beaconPos = cornerPos - outerDir * 2.15f;
                spawner.addProp("neonBeacon", beaconPos, 0.0f, glm::vec3(1.0f), true);
                spawner.addProp("neonBeaconTop", beaconPos + glm::vec3(0.0f, 0.45f, 0.0f), 0.0f, glm::vec3(1.0f), false);

                glm::vec3 tirePos1 = cornerPos + outerDir * 2.55f + glm::vec3(0.0f, 0.25f, 0.0f);
                glm::vec3 tirePos2 = cornerPos + outerDir * 2.55f + glm::vec3(0.0f, 0.65f, 0.0f);
                spawner.addProp("debris-tire", tirePos1, rot + 45.0f, glm::vec3(1.2f));
                spawner.addProp("debris-tire", tirePos2, rot + 45.0f, glm::vec3(1.0f));

                glm::vec3 barPos = cornerPos + outerDir * 3.10f;
                spawner.addProp("barrierRed", barPos, rot + 45.0f, glm::vec3(2.5f, 3.5f, 2.5f));
            } else {
                glm::vec3 apexPos = cornerPos - outerDir * 2.10f;
                spawner.addProp("cone", apexPos, 0.0f, glm::vec3(1.0f));

                glm::vec3 tirePos = cornerPos + outerDir * 2.50f + glm::vec3(0.0f, 0.25f, 0.0f);
                spawner.addProp("debris-tire", tirePos, rot + 45.0f, glm::vec3(1.1f));

                glm::vec3 barrier1Pos = cornerPos + outerDir * 3.05f;
                spawner.addProp("barrierRed", barrier1Pos, rot + 45.0f, glm::vec3(2.5f, 3.5f, 2.5f));

                glm::vec3 barrier2Pos = cornerPos + outerDir * 3.55f;
                spawner.addProp("barrierWhite", barrier2Pos, rot + 45.0f, glm::vec3(2.5f, 3.5f, 2.5f));
            }
        }

        // Rectas del circuito: Conos y delimitadores a los lados fuera del asfalto
        if (tile.type == TileType::ROAD_STRAIGHT) {
            if (startFinishTile != nullptr && glm::distance(tile.worldPos, startFinishTile->worldPos) < 9.0f) {
                continue;
            }

            bool isHorizontal = (tile.rotationDegrees == 90.0f || tile.rotationDegrees == 270.0f);
            float lateralDist = halfTile * 1.25f;
            float parallelWallRot = isHorizontal ? 0.0f : 90.0f;

            if ((tile.col + tile.row) % 3 == 0) {
                float side = ((propIndex++) % 2 == 0) ? 1.0f : -1.0f;
                glm::vec3 propOffset = isHorizontal ? glm::vec3(0.0f, 0.0f, side * lateralDist) : glm::vec3(side * lateralDist, 0.0f, 0.0f);

                if (propIndex % 4 == 0) {
                    spawner.addProp("barrierRed", tile.worldPos + propOffset, parallelWallRot, glm::vec3(2.5f, 3.5f, 2.5f));
                } else if (propIndex % 4 == 2) {
                    spawner.addProp("barrierWhite", tile.worldPos + propOffset, parallelWallRot, glm::vec3(2.5f, 3.5f, 2.5f));
                } else {
                    spawner.addProp("cone", tile.worldPos + propOffset, 0.0f, glm::vec3(1.0f));
                }
            }

            if ((tile.col * 7 + tile.row * 13) % 19 == 0) {
                float decorSide = ((tile.col % 2 == 0) ? 1.0f : -1.0f) * (halfTile * 1.45f);
                glm::vec3 decorOffset = isHorizontal ? glm::vec3(0.0f, 0.0f, decorSide) : glm::vec3(decorSide, 0.0f, 0.0f);
                if ((tile.col + tile.row) % 2 == 0) {
                    spawner.addProp("box", tile.worldPos + decorOffset, 25.0f, glm::vec3(0.9f));
                } else {
                    spawner.addProp("debris-tire", tile.worldPos + decorOffset + glm::vec3(0.0f, 0.25f, 0.0f), 0.0f, glm::vec3(1.0f));
                }
            }
        }
    }

    // 5. Bosques y Pinos Low-Poly en las Áreas Abiertas de Césped (Infield & Outfield)
    int treeMod = m_isNight ? 6 : 3;
    for (int r = 1; r < grid.getRows() - 1; ++r) {
        for (int c = 1; c < grid.getCols() - 1; ++c) {
            const auto* current = grid.getTile(c, r);
            if (current && current->type == TileType::EMPTY) {
                bool nearRoad = false;
                for (int dr = -1; dr <= 1 && !nearRoad; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        const auto* n = grid.getTile(c + dc, r + dr);
                        if (n && n->type != TileType::EMPTY) {
                            nearRoad = true;
                            break;
                        }
                    }
                }

                if (!nearRoad && (c * 13 + r * 17) % treeMod == 0) {
                    glm::vec3 treePos = current->worldPos + glm::vec3(
                        ((c % 3) - 1.0f) * 0.8f,
                        0.0f,
                        ((r % 3) - 1.0f) * 0.8f
                    );
                    float treeRot = static_cast<float>((c * 37 + r * 53) % 360);

                    spawner.addProp("treeTrunk", treePos + glm::vec3(0.0f, 0.8f, 0.0f), treeRot, glm::vec3(1.0f), true);
                    spawner.addProp("treeFoliageBottom", treePos + glm::vec3(0.0f, 1.85f, 0.0f), treeRot + 15.0f, glm::vec3(1.0f), false);
                    spawner.addProp("treeFoliageTop", treePos + glm::vec3(0.0f, 2.70f, 0.0f), treeRot + 30.0f, glm::vec3(1.0f), false);
                }
            }
        }
    }

    // 6. Decoración del Fondo y Entorno de Estadio Perimetral (Torres Floodlight y Muros)
    float halfW = (grid.getCols() * TrackGrid::TILE_SIZE) * 0.5f + 7.0f;
    float halfD = (grid.getRows() * TrackGrid::TILE_SIZE) * 0.5f + 7.0f;

    // 4 Torres de Iluminación en las esquinas
    spawner.addProp("stadiumTower", glm::vec3(-halfW, 5.0f, -halfD), 45.0f, glm::vec3(1.0f));
    spawner.addProp("stadiumTower", glm::vec3( halfW, 5.0f, -halfD), -45.0f, glm::vec3(1.0f));
    spawner.addProp("stadiumTower", glm::vec3( halfW, 5.0f,  halfD), 45.0f, glm::vec3(1.0f));
    spawner.addProp("stadiumTower", glm::vec3(-halfW, 5.0f,  halfD), -45.0f, glm::vec3(1.0f));

    // Torres intermedias en los laterales
    spawner.addProp("stadiumTower", glm::vec3(0.0f, 5.0f, -halfD), 0.0f, glm::vec3(1.0f));
    spawner.addProp("stadiumTower", glm::vec3(0.0f, 5.0f,  halfD), 0.0f, glm::vec3(1.0f));
    spawner.addProp("stadiumTower", glm::vec3(-halfW, 5.0f, 0.0f), 90.0f, glm::vec3(1.0f));
    spawner.addProp("stadiumTower", glm::vec3( halfW, 5.0f, 0.0f), 90.0f, glm::vec3(1.0f));

    // Vallas de contención de estadio alrededor de la pista
    for (float x = -halfW + 12.0f; x <= halfW - 12.0f; x += 12.0f) {
        spawner.addProp("barrierWall", glm::vec3(x, 0.0f, -halfD), 0.0f, glm::vec3(5.0f, 7.0f, 5.0f));
        spawner.addProp("barrierWall", glm::vec3(x, 0.0f,  halfD), 0.0f, glm::vec3(5.0f, 7.0f, 5.0f));
    }
    for (float z = -halfD + 12.0f; z <= halfD - 12.0f; z += 12.0f) {
        spawner.addProp("barrierWall", glm::vec3(-halfW, 0.0f, z), 90.0f, glm::vec3(5.0f, 7.0f, 5.0f));
        spawner.addProp("barrierWall", glm::vec3( halfW, 0.0f, z), 90.0f, glm::vec3(5.0f, 7.0f, 5.0f));
    }

    std::cout << "[OK] [PropSpawner] Total de elementos generados: "
              << spawner.getProps().size() << " (Arco monumental, farolas, torres, "
              << (m_isNight ? "arcos de neon y balizas nocturnas)." : "pinos y defensas diurnas).") << std::endl;
}

// ==============================================================================
// 2. Desert
// ==============================================================================

void DesertPropSpawner::spawnProps(const TrackGrid& grid, PropSpawner& spawner) {
    std::vector<const TileData*> straightTiles;
    const TileData* startFinishTile = nullptr;

    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::START_FINISH) {
            startFinishTile = &tile;
        } else if (tile.type == TileType::ROAD_STRAIGHT) {
            straightTiles.push_back(&tile);
        }
    }

    // 1. Arco Natural de Piedra y Arenisca en la Salida / Meta
    if (startFinishTile) {
        bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
        float beamRot = isHorizontal ? 90.0f : 0.0f;
        float pillarOffset = 2.60f;
        glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 pillarAOffset = sideDir * pillarOffset;
        glm::vec3 pillarBOffset = -sideDir * pillarOffset;

        glm::vec3 posA = startFinishTile->worldPos + pillarAOffset;
        glm::vec3 posB = startFinishTile->worldPos + pillarBOffset;

        spawner.addProp("desertArchPillar", glm::vec3(posA.x, 2.50f, posA.z), beamRot, glm::vec3(1.0f), true);
        spawner.addProp("desertArchPillar", glm::vec3(posB.x, 2.50f, posB.z), beamRot, glm::vec3(1.0f), true);

        glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 5.10f, 0.0f);
        spawner.addProp("desertArchBeam", beamPos, beamRot, glm::vec3(1.0f), false);

        spawner.addProp("desertRock", posA + sideDir * 1.1f, 25.0f, glm::vec3(1.1f), true);
        spawner.addProp("desertRock", posB - sideDir * 1.1f, 65.0f, glm::vec3(1.1f), true);
    }

    // 2. Obstáculos y decoraciones en curvas: Barreras de neumáticos, conos y rocas de arenisca
    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::ROAD_CORNER_SMALL || tile.type == TileType::ROAD_CORNER_LARGE) {
            float rotRad = glm::radians(tile.rotationDegrees);
            glm::vec3 outerDir(std::cos(rotRad + 0.785f), 0.0f, std::sin(rotRad + 0.785f));

            spawner.addProp("debris-tire", tile.worldPos + outerDir * 2.35f, tile.rotationDegrees, glm::vec3(1.3f), true);
            spawner.addProp("cone", tile.worldPos - outerDir * 1.8f, tile.rotationDegrees + 45.0f, glm::vec3(1.1f), true);

            spawner.addProp("desertRock", tile.worldPos + outerDir * 3.8f, tile.rotationDegrees + 30.0f, glm::vec3(1.3f, 1.2f, 1.3f), true);
            spawner.addProp("desertMesaRock", tile.worldPos + outerDir * 5.8f, tile.rotationDegrees + 75.0f, glm::vec3(1.4f, 1.5f, 1.2f), true);
        }
    }

    // 3. Bosques de Cactus Saguaro 3D y Monolitos a lo largo de las rectas
    for (size_t i = 0; i < straightTiles.size(); ++i) {
        const auto* tile = straightTiles[i];
        bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
        float sideSign = (i % 2 == 0) ? 1.0f : -1.0f;
        glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);

        if (i % 2 == 0) {
            float dist = 3.6f + (i % 3) * 0.8f;
            glm::vec3 cPos = tile->worldPos + sideDir * dist;
            float cactusRot = static_cast<float>((i * 53) % 360);

            spawner.addProp("cactusTrunk", cPos + glm::vec3(0.0f, 1.25f, 0.0f), cactusRot, glm::vec3(1.0f), true);

            float rad = glm::radians(cactusRot);
            glm::vec3 leftDir(-std::sin(rad), 0.0f, std::cos(rad));
            glm::vec3 rightDir(std::sin(rad), 0.0f, -std::cos(rad));

            spawner.addProp("cactusArmJoint", cPos + leftDir * 0.28f + glm::vec3(0.0f, 1.55f, 0.0f), cactusRot, glm::vec3(1.0f), false);
            spawner.addProp("cactusArmSpire", cPos + leftDir * 0.42f + glm::vec3(0.0f, 1.95f, 0.0f), cactusRot, glm::vec3(1.0f), false);

            spawner.addProp("cactusArmJoint", cPos + rightDir * 0.28f + glm::vec3(0.0f, 1.15f, 0.0f), cactusRot, glm::vec3(1.0f), false);
            spawner.addProp("cactusArmSpire", cPos + rightDir * 0.42f + glm::vec3(0.0f, 1.55f, 0.0f), cactusRot, glm::vec3(1.0f), false);

            if (i % 4 == 0) {
                spawner.addProp("desertRock", cPos + sideDir * 0.9f + glm::vec3(0.0f, 0.5f, 0.0f), cactusRot + 45.0f, glm::vec3(0.7f), true);
            }
        } else if (i % 3 == 0) {
            float dist = 4.2f + (i % 2) * 1.2f;
            glm::vec3 rockPos = tile->worldPos - sideDir * dist;
            spawner.addProp("desertRock", rockPos + glm::vec3(0.0f, 0.45f, 0.0f), static_cast<float>((i * 71) % 360), glm::vec3(1.1f, 0.8f, 1.0f), true);
        }
    }

    std::cout << "[OK] [PropSpawner] Total de elementos generados: "
              << spawner.getProps().size() << " (Arco natural de arenisca, cactus saguaro 3D y rocas de arenisca del desierto)." << std::endl;
}

// ==============================================================================
// 3. Space
// ==============================================================================

void SpacePropSpawner::spawnProps(const TrackGrid& grid, PropSpawner& spawner) {
    spawner.clearStarLights();

    std::vector<const TileData*> straightTiles;
    const TileData* startFinishTile = nullptr;

    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::START_FINISH) {
            startFinishTile = &tile;
        } else if (tile.type == TileType::ROAD_STRAIGHT) {
            straightTiles.push_back(&tile);
        }
    }

    // 1. Gran Portal de Lanzamiento Orbital en la Salida / Meta
    if (startFinishTile) {
        bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
        float beamRot = isHorizontal ? 90.0f : 0.0f;
        float pillarOffset = 2.45f;
        glm::vec3 pillarAOffset = isHorizontal ? glm::vec3(0.0f, 0.0f,  pillarOffset) : glm::vec3( pillarOffset, 0.0f, 0.0f);
        glm::vec3 pillarBOffset = isHorizontal ? glm::vec3(0.0f, 0.0f, -pillarOffset) : glm::vec3(-pillarOffset, 0.0f, 0.0f);

        glm::vec3 posA = startFinishTile->worldPos + pillarAOffset;
        glm::vec3 posB = startFinishTile->worldPos + pillarBOffset;

        spawner.addProp("spaceWarpPillar", glm::vec3(posA.x, 2.40f, posA.z), beamRot, glm::vec3(1.0f), true);
        spawner.addProp("spaceWarpPillar", glm::vec3(posB.x, 2.40f, posB.z), beamRot, glm::vec3(1.0f), true);

        glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 4.80f, 0.0f);
        spawner.addProp("spaceWarpBeam", beamPos, beamRot, glm::vec3(1.0f), false);
        spawner.addProp("spaceWarpTrimCyan", beamPos + glm::vec3(0.0f, 0.18f, 0.0f), beamRot, glm::vec3(1.0f), false);
        spawner.addProp("spaceWarpTrimMagenta", beamPos - glm::vec3(0.0f, 0.18f, 0.0f), beamRot, glm::vec3(1.0f), false);
    }

    // 2. Anillos de Hipervelocidad (Warp Rings) a lo largo de las grandes rectas
    for (size_t i = 0; i < straightTiles.size(); ++i) {
        const auto* tile = straightTiles[i];
        bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
        float beamRot = isHorizontal ? 90.0f : 0.0f;

        if (i % 6 == 2) {
            float pillarOffset = 2.40f;
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 posA = tile->worldPos + sideDir * pillarOffset;
            glm::vec3 posB = tile->worldPos - sideDir * pillarOffset;

            spawner.addProp("spaceWarpPillar", glm::vec3(posA.x, 2.40f, posA.z), beamRot, glm::vec3(1.0f), true);
            spawner.addProp("spaceWarpPillar", glm::vec3(posB.x, 2.40f, posB.z), beamRot, glm::vec3(1.0f), true);

            glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 4.70f, 0.0f);
            spawner.addProp("spaceWarpBeam", beamPos, beamRot, glm::vec3(1.0f), false);
            if (i % 12 == 2) {
                spawner.addProp("spaceWarpTrimCyan", beamPos + glm::vec3(0.0f, 0.15f, 0.0f), beamRot, glm::vec3(1.0f), false);
            } else {
                spawner.addProp("spaceWarpTrimMagenta", beamPos + glm::vec3(0.0f, 0.15f, 0.0f), beamRot, glm::vec3(1.0f), false);
            }
        }

        // Satélites flotantes de comunicación orbital a los lados
        if (i % 8 == 4) {
            float sideSign = (i % 16 == 4) ? 1.0f : -1.0f;
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);
            glm::vec3 satPos = tile->worldPos + sideDir * 5.5f + glm::vec3(0.0f, 3.8f, 0.0f);
            float satRot = static_cast<float>((i * 67) % 360);

            spawner.addProp("satelliteBody", satPos, satRot, glm::vec3(1.0f), false);
            spawner.addProp("satellitePanel", satPos, satRot, glm::vec3(1.0f), false);
            spawner.addProp("satelliteAntenna", satPos + glm::vec3(0.0f, 0.55f, 0.0f), satRot, glm::vec3(1.0f), false);
        }
    }

    // 3. Balizas de Navegación Estelar en los bordes de todas las curvas
    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::ROAD_CORNER_SMALL || tile.type == TileType::ROAD_CORNER_LARGE) {
            float rotRad = glm::radians(tile.rotationDegrees);
            glm::vec3 outerDir(std::cos(rotRad + 0.785f), 0.0f, std::sin(rotRad + 0.785f));

            glm::vec3 bPos = tile.worldPos + outerDir * 2.30f;
            spawner.addProp("spaceBeacon", bPos, tile.rotationDegrees, glm::vec3(1.0f), true);
            spawner.addProp("spaceBeaconTop", bPos + glm::vec3(0.0f, 0.45f, 0.0f), tile.rotationDegrees, glm::vec3(1.0f), false);
        }
    }

    // 4. Estrellas Flotantes Neón en la Pista
    struct NeonStarTheme {
        glm::vec4 meshColor;
        glm::vec3 lightColor;
    };

    const std::vector<NeonStarTheme> neonThemes = {
        { glm::vec4(0.0f, 0.98f, 1.0f, 1.0f),  glm::vec3(0.05f, 0.95f, 1.0f) },
        { glm::vec4(1.0f, 0.12f, 0.88f, 1.0f), glm::vec3(1.0f, 0.10f, 0.85f) },
        { glm::vec4(1.0f, 0.85f, 0.08f, 1.0f), glm::vec3(1.0f, 0.80f, 0.08f) },
        { glm::vec4(0.12f, 1.0f, 0.45f, 1.0f), glm::vec3(0.12f, 1.0f, 0.40f) },
        { glm::vec4(0.75f, 0.20f, 1.0f, 1.0f), glm::vec3(0.75f, 0.18f, 1.0f) },
        { glm::vec4(0.0f, 0.85f, 0.95f, 1.0f), glm::vec3(0.05f, 0.85f, 0.95f) },
        { glm::vec4(1.0f, 0.45f, 0.05f, 1.0f), glm::vec3(1.0f, 0.42f, 0.05f) },
        { glm::vec4(0.95f, 0.08f, 0.65f, 1.0f), glm::vec3(0.95f, 0.08f, 0.65f) }
    };

    size_t starCount = 0;
    size_t starStride = std::max<size_t>(1, straightTiles.size() / 8);

    for (size_t i = 0; i < straightTiles.size() && starCount < 8; i += starStride) {
        size_t tileIdx = i;
        if (tileIdx % 6 == 2 && tileIdx + 1 < straightTiles.size()) {
            tileIdx++;
        }
        const auto* sTile = straightTiles[tileIdx];

        bool isHorizontal = (sTile->rotationDegrees == 90.0f || sTile->rotationDegrees == 270.0f);
        glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);

        float sideSign = (starCount % 2 == 0) ? 1.0f : -1.0f;
        float lateralOffset = 2.45f;
        float hoverHeight = 1.65f;

        glm::vec3 starPos = sTile->worldPos + sideDir * (sideSign * lateralOffset) + glm::vec3(0.0f, hoverHeight, 0.0f);
        const auto& theme = neonThemes[starCount % neonThemes.size()];

        spawner.addProp("floatingStar", starPos, static_cast<float>(starCount * 45), glm::vec3(1.0f),
                        false, theme.meshColor, true);

        spawner.addStarLight(starPos, theme.lightColor);
        starCount++;
    }

    std::cout << "[OK] [PropSpawner] Total de elementos generados: "
              << spawner.getProps().size() << " (Portales Warp, satelites, balizas y "
              << starCount << " estrellas flotantes neon no-blancas)." << std::endl;
}

// ==============================================================================
// 4. Aquatic
// ==============================================================================

void AquaticPropSpawner::spawnProps(const TrackGrid& grid, PropSpawner& spawner) {
    std::vector<const TileData*> straightTiles;
    const TileData* startFinishTile = nullptr;

    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::START_FINISH) {
            startFinishTile = &tile;
        } else if (tile.type == TileType::ROAD_STRAIGHT) {
            straightTiles.push_back(&tile);
        }
    }

    // 1. Gran Portal Atlante de Coral y Nácar en la Salida / Meta
    if (startFinishTile) {
        bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
        float beamRot = isHorizontal ? 90.0f : 0.0f;
        float pillarOffset = 2.65f;
        glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 pillarAOffset = sideDir * pillarOffset;
        glm::vec3 pillarBOffset = -sideDir * pillarOffset;

        glm::vec3 posA = startFinishTile->worldPos + pillarAOffset;
        glm::vec3 posB = startFinishTile->worldPos + pillarBOffset;

        spawner.addProp("seaArchPillar", glm::vec3(posA.x, 2.50f, posA.z), beamRot, glm::vec3(1.0f), true);
        spawner.addProp("seaArchPillar", glm::vec3(posB.x, 2.50f, posB.z), beamRot, glm::vec3(1.0f), true);

        glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 5.00f, 0.0f);
        spawner.addProp("seaArchBeam", beamPos, beamRot, glm::vec3(1.0f), false);
        spawner.addProp("seaArchTrim", beamPos - glm::vec3(0.0f, 0.20f, 0.0f), beamRot, glm::vec3(1.0f), false);

        spawner.addProp("seaRock", posA + sideDir * 1.2f, 30.0f, glm::vec3(1.2f), true);
        spawner.addProp("seaRock", posB - sideDir * 1.2f, 60.0f, glm::vec3(1.2f), true);

        float rotA = glm::degrees(std::atan2(-sideDir.x, -sideDir.z));
        float rotB = glm::degrees(std::atan2(sideDir.x, sideDir.z));

        spawner.addProp("seaPedestal", posA + sideDir * 1.8f + glm::vec3(0.0f, 0.75f, 0.0f), 0.0f, glm::vec3(1.0f), true);
        spawner.addProp("clamShell", posA + sideDir * 1.8f + glm::vec3(0.0f, 1.50f, 0.0f), rotA, glm::vec3(1.0f), false);
        spawner.addProp("seaPearl", posA + sideDir * 1.8f + glm::vec3(0.0f, 1.54f, 0.0f), 0.0f, glm::vec3(1.0f), false);

        spawner.addProp("seaPedestal", posB - sideDir * 1.8f + glm::vec3(0.0f, 0.75f, 0.0f), 0.0f, glm::vec3(1.0f), true);
        spawner.addProp("clamShell", posB - sideDir * 1.8f + glm::vec3(0.0f, 1.50f, 0.0f), rotB, glm::vec3(1.0f), false);
        spawner.addProp("seaPearl", posB - sideDir * 1.8f + glm::vec3(0.0f, 1.54f, 0.0f), 0.0f, glm::vec3(1.0f), false);

        spawner.addLampPosition(posA + sideDir * 1.8f + glm::vec3(0.0f, 1.54f, 0.0f));
        spawner.addLampPosition(posB - sideDir * 1.8f + glm::vec3(0.0f, 1.54f, 0.0f));
    }

    // 2. Obstáculos y Decoraciones del Arrecife en Curvas
    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::ROAD_CORNER_SMALL || tile.type == TileType::ROAD_CORNER_LARGE) {
            float rotRad = glm::radians(tile.rotationDegrees);
            glm::vec3 outerDir(std::cos(rotRad + 0.785f), 0.0f, std::sin(rotRad + 0.785f));

            spawner.addProp("cone", tile.worldPos - outerDir * 1.9f, tile.rotationDegrees + 45.0f, glm::vec3(1.15f), true);
            spawner.addProp("debris-tire", tile.worldPos + outerDir * 2.45f, tile.rotationDegrees, glm::vec3(1.3f), true);

            spawner.addProp("seaRock", tile.worldPos + outerDir * 4.0f, tile.rotationDegrees + 25.0f, glm::vec3(1.3f), true);
            spawner.addProp("coralBrain", tile.worldPos + outerDir * 3.2f + glm::vec3(0.0f, 0.40f, 0.0f), tile.rotationDegrees + 60.0f, glm::vec3(1.2f), true);
            spawner.addProp("bubbleColumn", tile.worldPos + outerDir * 5.2f + glm::vec3(0.0f, 1.80f, 0.0f), 0.0f, glm::vec3(1.0f), false);
        }
    }

    // 3. Bosques de Algas Kelp, Ramas de Coral 3D y Pedestales con Perlas en las Rectas
    int pearlCount = 0;
    for (size_t i = 0; i < straightTiles.size(); ++i) {
        const auto* tile = straightTiles[i];
        bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
        float sideSign = (i % 2 == 0) ? 1.0f : -1.0f;
        glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);

        float kelpDist = 3.6f + (i % 3) * 0.7f;
        glm::vec3 kelpPos = tile->worldPos + sideDir * kelpDist;
        float kelpRot = static_cast<float>((i * 47) % 360);

        spawner.addProp("kelpSegment", kelpPos + glm::vec3(0.0f, 0.85f, 0.0f), kelpRot, glm::vec3(1.0f), false);
        spawner.addProp("kelpSegment", kelpPos + glm::vec3(0.0f, 2.35f, 0.0f), kelpRot + 25.0f, glm::vec3(0.9f, 0.85f, 0.9f), false);
        spawner.addProp("kelpSegment", kelpPos + glm::vec3(0.0f, 3.75f, 0.0f), kelpRot + 50.0f, glm::vec3(0.8f, 0.80f, 0.8f), false);

        if (i % 2 == 0) {
            glm::vec3 coralPos = tile->worldPos - sideDir * (3.8f + (i % 2) * 0.6f);
            float coralRot = static_cast<float>((i * 63) % 360);
            spawner.addProp("coralBranchTall", coralPos + glm::vec3(0.0f, 1.10f, 0.0f), coralRot, glm::vec3(1.0f), true);
            spawner.addProp("coralBranchSide", coralPos + glm::vec3(0.25f, 0.75f, 0.0f), coralRot + 45.0f, glm::vec3(1.0f), true);
            spawner.addProp("coralBranchSide", coralPos + glm::vec3(-0.25f, 0.95f, 0.0f), coralRot - 45.0f, glm::vec3(1.0f), true);
            spawner.addProp("coralBrain", coralPos + glm::vec3(0.6f, 0.40f, 0.3f), coralRot + 90.0f, glm::vec3(0.9f), true);
        }

        if (i % 5 == 1 && pearlCount < 6) {
            float pearlDist = 2.65f;
            glm::vec3 pPos = tile->worldPos + sideDir * pearlDist;
            spawner.addProp("seaPedestal", pPos + glm::vec3(0.0f, 0.75f, 0.0f), 0.0f, glm::vec3(1.0f), true);

            float clamFacingRot = glm::degrees(std::atan2(-sideDir.x, -sideDir.z));
            spawner.addProp("clamShell", pPos + glm::vec3(0.0f, 1.50f, 0.0f), clamFacingRot, glm::vec3(1.0f), false);
            spawner.addProp("seaPearl", pPos + glm::vec3(0.0f, 1.54f, 0.0f), 0.0f, glm::vec3(1.0f), false);

            spawner.addLampPosition(pPos + glm::vec3(0.0f, 1.54f, 0.0f));
            pearlCount++;
        }

        if (i % 4 == 2) {
            spawner.addProp("bubbleColumn", tile->worldPos - sideDir * 4.6f + glm::vec3(0.0f, 0.1f, 0.0f), 0.0f, glm::vec3(1.0f), false);
        }
    }

    std::cout << "[OK] [PropSpawner] Total de elementos generados: "
              << spawner.getProps().size() << " (Arco Atlante, " << pearlCount
              << " perlas bioluminiscentes, bosques de algas kelp y corales 3D)." << std::endl;
}

// ==============================================================================
// 5. Volcano
// ==============================================================================

void VolcanoPropSpawner::spawnProps(const TrackGrid& grid, PropSpawner& spawner) {
    std::vector<const TileData*> straightTiles;
    const TileData* startFinishTile = nullptr;

    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::START_FINISH) {
            startFinishTile = &tile;
        } else if (tile.type == TileType::ROAD_STRAIGHT) {
            straightTiles.push_back(&tile);
        }
    }

    // 1. Gran Portal del Dragón en Basalto y Magma en la Salida / Meta
    if (startFinishTile) {
        bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
        float beamRot = isHorizontal ? 90.0f : 0.0f;
        float pillarOffset = 2.65f;
        glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 pillarAOffset = sideDir * pillarOffset;
        glm::vec3 pillarBOffset = -sideDir * pillarOffset;

        glm::vec3 posA = startFinishTile->worldPos + pillarAOffset;
        glm::vec3 posB = startFinishTile->worldPos + pillarBOffset;

        spawner.addProp("volcanoArchPillar", glm::vec3(posA.x, 2.60f, posA.z), beamRot, glm::vec3(1.0f), true);
        spawner.addProp("volcanoArchPillar", glm::vec3(posB.x, 2.60f, posB.z), beamRot, glm::vec3(1.0f), true);

        glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 5.20f, 0.0f);
        spawner.addProp("volcanoArchBeam", beamPos, beamRot, glm::vec3(1.0f), false);
        spawner.addProp("volcanoArchTrimLava", beamPos - glm::vec3(0.0f, 0.20f, 0.0f), beamRot, glm::vec3(1.0f), false);

        spawner.addProp("volcanoRock", posA + sideDir * 1.2f, 35.0f, glm::vec3(1.2f), true);
        spawner.addProp("volcanoRock", posB - sideDir * 1.2f, 65.0f, glm::vec3(1.2f), true);

        spawner.addProp("fireBrazier", posA + sideDir * 1.8f + glm::vec3(0.0f, 0.70f, 0.0f), 0.0f, glm::vec3(1.0f), true);
        spawner.addProp("volcanoFlame", posA + sideDir * 1.8f + glm::vec3(0.0f, 1.25f, 0.0f), 0.0f, glm::vec3(1.0f), false);
        spawner.addProp("volcanoFlameCore", posA + sideDir * 1.8f + glm::vec3(0.0f, 1.28f, 0.0f), 0.0f, glm::vec3(0.9f), false);

        spawner.addProp("fireBrazier", posB - sideDir * 1.8f + glm::vec3(0.0f, 0.70f, 0.0f), 0.0f, glm::vec3(1.0f), true);
        spawner.addProp("volcanoFlame", posB - sideDir * 1.8f + glm::vec3(0.0f, 1.25f, 0.0f), 0.0f, glm::vec3(1.0f), false);
        spawner.addProp("volcanoFlameCore", posB - sideDir * 1.8f + glm::vec3(0.0f, 1.28f, 0.0f), 0.0f, glm::vec3(0.9f), false);

        spawner.addLampPosition(posA + sideDir * 1.8f + glm::vec3(0.0f, 1.65f, 0.0f));
        spawner.addLampPosition(posB - sideDir * 1.8f + glm::vec3(0.0f, 1.65f, 0.0f));
    }

    // 2. Obstáculos y Decoraciones Volcánicas en Curvas
    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::ROAD_CORNER_SMALL || tile.type == TileType::ROAD_CORNER_LARGE) {
            float rotRad = glm::radians(tile.rotationDegrees);
            glm::vec3 outerDir(std::cos(rotRad + 0.785f), 0.0f, std::sin(rotRad + 0.785f));

            spawner.addProp("debris-tire", tile.worldPos + outerDir * 2.40f, tile.rotationDegrees, glm::vec3(1.3f), true);
            spawner.addProp("cone", tile.worldPos - outerDir * 1.85f, tile.rotationDegrees + 45.0f, glm::vec3(1.15f), true);

            spawner.addProp("basaltColumn", tile.worldPos + outerDir * 3.8f + glm::vec3(0.0f, 1.60f, 0.0f), tile.rotationDegrees + 20.0f, glm::vec3(1.0f), true);
            spawner.addProp("volcanoRock", tile.worldPos + outerDir * 5.2f, tile.rotationDegrees + 70.0f, glm::vec3(1.3f), true);
            spawner.addProp("lavaVent", tile.worldPos + outerDir * 2.8f + glm::vec3(0.0f, 0.05f, 0.0f), tile.rotationDegrees, glm::vec3(1.0f), false);
        }
    }

    // 3. Columnas de Basalto, Chimeneas de Fumarolas y Braseros en las Rectas
    int flameCount = 0;
    for (size_t i = 0; i < straightTiles.size(); ++i) {
        const auto* tile = straightTiles[i];
        bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
        float sideSign = (i % 2 == 0) ? 1.0f : -1.0f;
        glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);

        if (i % 2 == 0) {
            float bDist = 3.6f + (i % 3) * 0.7f;
            glm::vec3 bPos = tile->worldPos + sideDir * bDist;
            float bRot = static_cast<float>((i * 51) % 360);
            spawner.addProp("basaltColumn", bPos + glm::vec3(0.0f, 1.60f, 0.0f), bRot, glm::vec3(1.0f), true);
            spawner.addProp("volcanoRock", bPos + glm::vec3(0.4f, 0.0f, 0.3f), bRot + 30.0f, glm::vec3(0.9f), true);
        } else {
            float cDist = 3.8f + (i % 2) * 0.6f;
            glm::vec3 cPos = tile->worldPos - sideDir * cDist;
            float cRot = static_cast<float>((i * 37) % 360);
            spawner.addProp("lavaChimney", cPos + glm::vec3(0.0f, 1.10f, 0.0f), cRot, glm::vec3(1.0f), true);
            spawner.addProp("lavaVent", cPos + glm::vec3(0.2f, 0.05f, 0.2f), cRot, glm::vec3(0.9f), false);
        }

        if (i % 5 == 1 && flameCount < 6) {
            float brazierDist = 2.65f;
            glm::vec3 bPos = tile->worldPos + sideDir * brazierDist;
            spawner.addProp("fireBrazier", bPos + glm::vec3(0.0f, 0.70f, 0.0f), 0.0f, glm::vec3(1.0f), true);

            float rotDeg = static_cast<float>((i * 53) % 360);
            spawner.addProp("volcanoFlame", bPos + glm::vec3(0.0f, 1.25f, 0.0f), rotDeg, glm::vec3(1.0f), false);
            spawner.addProp("volcanoFlameCore", bPos + glm::vec3(0.0f, 1.28f, 0.0f), rotDeg, glm::vec3(0.9f), false);

            spawner.addLampPosition(bPos + glm::vec3(0.0f, 1.65f, 0.0f));
            flameCount++;
        }
    }

    std::cout << "[OK] [PropSpawner] Total de elementos generados: "
              << spawner.getProps().size() << " (Arco del Dragon, " << flameCount
              << " braseros de fuego, columnas de basalto y chimeneas volcanicas)." << std::endl;
}

} // namespace world
