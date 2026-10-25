#pragma once

#include <string>
#include <glm/glm.hpp>

namespace world {

/**
 * @brief Tipos de bioma fuertemente tipados para pistas y distribución de props.
 * Reemplaza los flags booleanos sueltos y evita estados de biomas contradictorios o superpuestos.
 */
enum class BiomeType {
    GRASS_DAY,
    GRASS_NIGHT,
    DESERT,
    SPACE,
    AQUATIC,
    VOLCANO
};

} // namespace world
