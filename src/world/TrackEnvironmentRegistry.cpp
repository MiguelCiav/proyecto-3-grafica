#include "world/TrackEnvironmentRegistry.hpp"

namespace world {

std::vector<DriverProfile> TrackEnvironmentRegistry::createDefaultDrivers() {
    return {
        {
            "driver-oobi",
            "Oobi (Verde)",
            "Personaje clasico, amistoso y equilibrado",
            "assets/models/karts/kart-oobi.obj",
            {}, glm::vec4(0.2f, 0.85f, 0.3f, 1.0f), false
        },
        {
            "driver-oodi",
            "Oodi (Azul)",
            "Piloto veloz con instinto aerodinamico",
            "assets/models/karts/kart-oodi.obj",
            {}, glm::vec4(0.2f, 0.5f, 0.95f, 1.0f), false
        },
        {
            "driver-ooli",
            "Ooli (Naranja)",
            "Especialista en terrenos rusticos y rally",
            "assets/models/karts/kart-ooli.obj",
            {}, glm::vec4(0.95f, 0.55f, 0.15f, 1.0f), false
        },
        {
            "driver-oopi",
            "Oopi (Rojo)",
            "Espiritu competitivo y aceleracion feroz",
            "assets/models/karts/kart-oopi.obj",
            {}, glm::vec4(0.9f, 0.2f, 0.2f, 1.0f), false
        },
        {
            "driver-oozi",
            "Oozi (Negro)",
            "Piloto sigiloso y maestro de curvas cerradas",
            "assets/models/karts/kart-oozi.obj",
            {}, glm::vec4(0.25f, 0.25f, 0.3f, 1.0f), false
        }
    };
}

std::vector<VehicleProfile> TrackEnvironmentRegistry::createDefaultVehicles() {
    return {
        // --- Karts Oficiales Kenney ---
        {
            "kart-oobi",
            "Kart Clasico",
            "Kart",
            "Chasis ligero estandar, agil y facil de maniobrar",
            "assets/models/karts/kart-oobi.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            22.0f, 18.0f, 100.0f, 0.45f, {}, false
        },
        {
            "kart-oodi",
            "Kart Turbo",
            "Kart",
            "Aleron aerodinamico de alta velocidad punta",
            "assets/models/karts/kart-oodi.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            25.5f, 16.0f, 85.0f, 0.38f, {}, false
        },
        {
            "kart-ooli",
            "Kart Rally",
            "Kart",
            "Traccion total y suspension reforzada para curvas",
            "assets/models/karts/kart-ooli.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            21.0f, 20.0f, 105.0f, 0.65f, {}, false
        },
        {
            "kart-oopi",
            "Kart Furia",
            "Kart",
            "Aceleracion explosiva y turbo de salida rapida",
            "assets/models/karts/kart-oopi.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            23.5f, 22.0f, 95.0f, 0.40f, {}, false
        },
        {
            "kart-oozi",
            "Kart Nocturno",
            "Kart",
            "Maximo angulo de giro y derrape ultratecnico",
            "assets/models/karts/kart-oozi.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            22.5f, 19.0f, 120.0f, 0.44f, {}, false
        },
        // --- Vehiculos Adicionales Kenney Car-Kit ---
        {
            "race",
            "Formula Race F1",
            "Carreras",
            "Monoplaza de competicion ultraligero y aerodinamico",
            "assets/models/vehicles/race.obj",
            glm::vec3(0.0f, -0.04f, -0.12f),
            27.0f, 21.0f, 110.0f, 0.35f, {}, false
        },
        {
            "race-future",
            "Formula Future",
            "Carreras",
            "Prototipo futurista de vanguardia y maxima velocidad",
            "assets/models/vehicles/race-future.obj",
            glm::vec3(0.0f, -0.04f, -0.18f),
            28.5f, 23.0f, 105.0f, 0.32f, {}, false
        },
        {
            "police",
            "Patrulla Policia",
            "Especial",
            "Vehiculo de persecucion con gran estabilidad y sirena",
            "assets/models/vehicles/police.obj",
            glm::vec3(0.0f, -0.15f, 0.02f),
            23.0f, 19.0f, 90.0f, 0.45f, {}, false
        },
        {
            "taxi",
            "Taxi Deportivo",
            "Ciudad",
            "Iconico taxi urbano preparado para carreras callejeras",
            "assets/models/vehicles/taxi.obj",
            glm::vec3(0.0f, -0.15f, 0.02f),
            23.5f, 17.5f, 95.0f, 0.42f, {}, false
        },
        {
            "sedan-sports",
            "Sedan Sports",
            "Ciudad",
            "Sedan con aleron deportivo y agarre equilibrado",
            "assets/models/vehicles/sedan-sports.obj",
            glm::vec3(0.0f, -0.15f, 0.02f),
            24.5f, 18.5f, 92.0f, 0.40f, {}, false
        },
        {
            "suv",
            "Camioneta SUV 4x4",
            "Especial",
            "Traccion integral 4x4 con rueda de repuesto",
            "assets/models/vehicles/suv.obj",
            glm::vec3(0.0f, -0.15f, 0.02f),
            21.5f, 19.5f, 88.0f, 0.70f, {}, false
        }
    };
}

std::vector<TrackProfile> TrackEnvironmentRegistry::createDefaultTracks() {
    return {
        {
            "track-grand-circuit",
            "Gran Circuito Internacional UCV (Dia)",
            "Gran Prix ★★★",
            "Mega circuito profesional de 112 tramos diurno con multiples chicanas, horquillas y arco monumental.",
            "assets/levels/track_grand_circuit.txt",
            28, 25,
            false, // isNight
            TrackTheme::DAY_GRASSLAND
        },
        {
            "track-night-neon",
            "Autodromo Nocturno UCV (Neon GP)",
            "Nocturno ★★★",
            "Espectacular circuito nocturno de 90 tramos iluminado con farolas, arcos de neon y balizas luminosas.",
            "assets/levels/track_night_circuit.txt",
            26, 22,
            true, // isNight
            TrackTheme::NIGHT_NEON
        },
        {
            "track-desert",
            "Valle del Desierto UCV (Dakar Canyon Wedge)",
            "Dakar ★★★",
            "Rally asimetrico en forma de cuña por cañones de arenisca, escalones de dunas y rectas de alta velocidad.",
            "assets/levels/track_desert_dunes.txt",
            26, 24,
            false, // isNight
            TrackTheme::DESERT_DUNES
        },
        {
            "track-cosmic",
            "Circuito Espacial UCV (Starfighter Orbital Wings)",
            "Extremo ★★★",
            "Nave orbital con alas solares exteriores, pasarela hiperespacial y vortices estelares suspendidos.",
            "assets/levels/track_cosmic_space.txt",
            26, 26,
            true, // isNight
            TrackTheme::COSMIC_SPACE
        },
        {
            "track-aquatic",
            "Arrecife Abisal UCV (Nautilus Reef & Kelp Atoll)",
            "Acuatico ★★★",
            "Atolon oceanico natural con amplia laguna sur, ensenada submarina y serpentina por bosques de algas kelp.",
            "assets/levels/track_aquatic_reef.txt",
            26, 26,
            false, // isNight
            TrackTheme::AQUATIC_REEF
        },
        {
            "track-volcano",
            "Caldera del Volcan UCV (Inferno Magma Crater)",
            "Infernal ★★★",
            "Circuito de alta adrenalina al borde de un crater de lava con chimeneas de humo, columnas de basalto y braseros.",
            "assets/levels/track_volcano_caldera.txt",
            26, 26,
            false, // isNight
            TrackTheme::VOLCANO_CALDERA
        }
    };
}

BiomeType TrackEnvironmentRegistry::getBiomeForTrack(const TrackProfile& track) {
    switch (track.theme) {
        case TrackTheme::DAY_GRASSLAND:
            return track.isNight ? BiomeType::GRASS_NIGHT : BiomeType::GRASS_DAY;
        case TrackTheme::NIGHT_NEON:
            return BiomeType::GRASS_NIGHT;
        case TrackTheme::DESERT_DUNES:
            return BiomeType::DESERT;
        case TrackTheme::COSMIC_SPACE:
            return BiomeType::SPACE;
        case TrackTheme::AQUATIC_REEF:
            return BiomeType::AQUATIC;
        case TrackTheme::VOLCANO_CALDERA:
            return BiomeType::VOLCANO;
    }
    return BiomeType::GRASS_DAY;
}

glm::vec4 TrackEnvironmentRegistry::getGroundColorForTrack(const TrackProfile& track) {
    switch (track.theme) {
        case TrackTheme::VOLCANO_CALDERA:
            return glm::vec4(0.09f, 0.08f, 0.09f, 1.0f); // Basalto volcanico y ceniza oscura
        case TrackTheme::COSMIC_SPACE:
            return glm::vec4(0.012f, 0.010f, 0.024f, 1.0f); // Vacio cosmico abisal
        case TrackTheme::AQUATIC_REEF:
            return glm::vec4(0.04f, 0.16f, 0.22f, 1.0f); // Fondo marino / lecho abisal turquesa
        case TrackTheme::DESERT_DUNES:
            return glm::vec4(0.89f, 0.74f, 0.48f, 1.0f); // Arena dorada cálida del Sahara
        default:
            if (track.isNight) {
                return glm::vec4(0.08f, 0.12f, 0.10f, 1.0f); // Asfalto / césped nocturno
            }
            return glm::vec4(0.28f, 0.62f, 0.28f, 1.0f);     // Césped verde clásico
    }
}

} // namespace world
