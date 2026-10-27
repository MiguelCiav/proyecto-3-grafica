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
            11.2f, 9.5f, 220.0f, 0.28f, {}, false
        },
        {
            "kart-oodi",
            "Kart Turbo",
            "Kart",
            "Aleron aerodinamico de alta velocidad punta",
            "assets/models/karts/kart-oodi.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            12.0f, 9.0f, 210.0f, 0.24f, {}, false
        },
        {
            "kart-ooli",
            "Kart Rally",
            "Kart",
            "Traccion total y suspension reforzada para curvas",
            "assets/models/karts/kart-ooli.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            10.8f, 10.5f, 235.0f, 0.35f, {}, false
        },
        {
            "kart-oopi",
            "Kart Furia",
            "Kart",
            "Aceleracion explosiva y turbo de salida rapida",
            "assets/models/karts/kart-oopi.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            11.6f, 11.2f, 220.0f, 0.25f, {}, false
        },
        {
            "kart-oozi",
            "Kart Nocturno",
            "Kart",
            "Maximo angulo de giro y derrape ultratecnico",
            "assets/models/karts/kart-oozi.obj",
            glm::vec3(0.0f, 0.0f, 0.0f),
            11.4f, 10.0f, 255.0f, 0.27f, {}, false
        },
        // --- Vehiculos Adicionales Kenney Car-Kit ---
        {
            "race",
            "Formula Race F1",
            "Carreras",
            "Monoplaza de competicion ultraligero y aerodinamico",
            "assets/models/vehicles/race.obj",
            glm::vec3(0.0f, -0.04f, -0.12f),
            12.5f, 11.0f, 240.0f, 0.22f, {}, false
        },
        {
            "race-future",
            "Formula Future",
            "Carreras",
            "Prototipo futurista de vanguardia y maxima velocidad",
            "assets/models/vehicles/race-future.obj",
            glm::vec3(0.0f, -0.04f, -0.18f),
            12.8f, 11.5f, 235.0f, 0.22f, {}, false
        },
        {
            "police",
            "Patrulla Policia",
            "Especial",
            "Vehiculo de persecucion con gran estabilidad y sirena",
            "assets/models/vehicles/police.obj",
            glm::vec3(0.0f, -0.15f, 0.02f),
            11.5f, 10.0f, 215.0f, 0.28f, {}, false
        },
        {
            "taxi",
            "Taxi Deportivo",
            "Ciudad",
            "Iconico taxi urbano preparado para carreras callejeras",
            "assets/models/vehicles/taxi.obj",
            glm::vec3(0.0f, -0.15f, 0.02f),
            11.6f, 9.5f, 220.0f, 0.26f, {}, false
        },
        {
            "sedan-sports",
            "Sedan Sports",
            "Ciudad",
            "Sedan con aleron deportivo y agarre equilibrado",
            "assets/models/vehicles/sedan-sports.obj",
            glm::vec3(0.0f, -0.15f, 0.02f),
            11.8f, 10.0f, 215.0f, 0.25f, {}, false
        },
        {
            "suv",
            "Camioneta SUV 4x4",
            "Especial",
            "Traccion integral 4x4 con rueda de repuesto",
            "assets/models/vehicles/suv.obj",
            glm::vec3(0.0f, -0.15f, 0.02f),
            10.5f, 10.2f, 210.0f, 0.35f, {}, false
        }
    };
}

std::vector<TrackProfile> TrackEnvironmentRegistry::createDefaultTracks() {
    return {
        {
            "track-grand-circuit",
            "Tierra de Nadie International GP",
            "Gran Prix ★★★",
            "El asfalto más ilustre de la Ciudad Universitaria, con parada técnica obligatoria en la Plaza Cubierta.",
            "assets/levels/track_grand_circuit.txt",
            28, 25,
            false, // isNight
            TrackTheme::DAY_GRASSLAND,
            "assets/sfx/Tierra_de_Nadie_International_Gp.mp3"
        },
        {
            "track-night-neon",
            "Cota Mil Midnight Circuit",
            "Nocturno ★★★",
            "La recta nocturna por excelencia: velocidad pura, sin postes de luz y esquivando desniveles a ciegas.",
            "assets/levels/track_night_circuit.txt",
            26, 22,
            true, // isNight
            TrackTheme::NIGHT_NEON,
            "assets/sfx/Cota_Mil_Midnight_Circuit.mp3"
        },
        {
            "track-desert",
            "Médanos Grand Prix",
            "Dakar ★★★",
            "Mucho derrape en arena, cero tracción y brisa con chivo al fondo.",
            "assets/levels/track_desert_dunes.txt",
            26, 24,
            false, // isNight
            TrackTheme::DESERT_DUNES,
            "assets/sfx/Medanos_grand_prix.mp3"
        },
        {
            "track-cosmic",
            "Humboldt Cosmic Raceway",
            "Extremo ★★★",
            "Inspirado en el Planetario Humboldt: correr entre galaxias sin salir de Caracas.",
            "assets/levels/track_cosmic_space.txt",
            26, 26,
            true, // isNight
            TrackTheme::COSMIC_SPACE,
            "assets/sfx/Humboldt_Cosmic_Raceway.mp3"
        },
        {
            "track-aquatic",
            "Los Roques Reef Route",
            "Acuático ★★★",
            "Fondo marino de aguas cristalinas, pero con curvas cerradas entre corales y peñeros.",
            "assets/levels/track_aquatic_reef.txt",
            26, 26,
            false, // isNight
            TrackTheme::AQUATIC_REEF,
            "assets/sfx/Los_Roques_Reef_Route.mp3"
        },
        {
            "track-volcano",
            "El Volcán Hillclimb",
            "Infernal ★★★",
            "Alusión directa al cerro El Volcán en El Hatillo: subida empinada, asfalto caliente y frenos al límite.",
            "assets/levels/track_volcano_caldera.txt",
            26, 26,
            false, // isNight
            TrackTheme::VOLCANO_CALDERA,
            "assets/sfx/El_volcan_hillclimb.mp3"
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
