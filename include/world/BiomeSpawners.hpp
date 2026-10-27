#pragma once

#include "world/TrackGrid.hpp"
#include "world/Biome.hpp"
#include <memory>

namespace world {

class PropSpawner;

class IBiomePropSpawner {
public:
    virtual ~IBiomePropSpawner() = default;
    virtual void spawnProps(const TrackGrid& grid, PropSpawner& spawner) = 0;
};

class GrasslandPropSpawner : public IBiomePropSpawner {
public:
    explicit GrasslandPropSpawner(bool isNight = false);
    void spawnProps(const TrackGrid& grid, PropSpawner& spawner) override;

    bool isNight() const { return m_isNight; }

private:
    bool m_isNight{false};
};

class DesertPropSpawner : public IBiomePropSpawner {
public:
    DesertPropSpawner() = default;
    void spawnProps(const TrackGrid& grid, PropSpawner& spawner) override;
};

class SpacePropSpawner : public IBiomePropSpawner {
public:
    SpacePropSpawner() = default;
    void spawnProps(const TrackGrid& grid, PropSpawner& spawner) override;
};

class AquaticPropSpawner : public IBiomePropSpawner {
public:
    AquaticPropSpawner() = default;
    void spawnProps(const TrackGrid& grid, PropSpawner& spawner) override;
};

class VolcanoPropSpawner : public IBiomePropSpawner {
public:
    VolcanoPropSpawner() = default;
    void spawnProps(const TrackGrid& grid, PropSpawner& spawner) override;
};

class BiomePropSpawnerFactory {
public:
    static std::unique_ptr<IBiomePropSpawner> create(BiomeType biome);
};

} // namespace world
