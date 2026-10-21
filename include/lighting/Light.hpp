#pragma once

#include <glm/glm.hpp>
#include <string>

namespace lighting {

/**
 * @brief Modelos de iluminación
 * Se permite alternar en tiempo real entre ellos mediante un controlador de escena.
 */
enum class LightingModel : int {
    FLAT = 0,         // Sombreado plano facetado (normal constante por polígono)
    LAMBERT = 1,      // Sombreado difuso puro (coseno del ángulo de incidencia N · L)
    BLINN_PHONG = 2   // Sombreado completo (Ambiente + Difuso + Especular con Half-vector)
};

/**
 * @brief Devuelve el nombre amigable del modelo de iluminación.
 */
inline const char* getLightingModelName(LightingModel model) {
    switch (model) {
        case LightingModel::FLAT:        return "Flat Shading (Plano Facetado)";
        case LightingModel::LAMBERT:     return "Lambert Shading (Difuso Puro)";
        case LightingModel::BLINN_PHONG: return "Blinn-Phong Shading (Completo Especular)";
        default:                         return "Desconocido";
    }
}

/**
 * @brief 1. Fuente de Iluminación Direccional (Simula el Sol del escenario).
 * Rayos paralelos sin atenuación por distancia.
 */
struct DirectionalLight {
    glm::vec3 direction{-0.35f, -0.85f, -0.4f}; // Vector apuntando hacia abajo y hacia adelante
    glm::vec3 ambient{0.32f, 0.32f, 0.36f};     // Iluminación ambiental base diurna
    glm::vec3 diffuse{0.88f, 0.86f, 0.82f};     // Tono cálido solar
    glm::vec3 specular{0.50f, 0.50f, 0.50f};    // Brillo reflejado
    bool enabled{true};
};

/**
 * @brief 2. Fuente de Iluminación Puntual (Simula una farola o poste de luz en la pista).
 * Emisión omnidireccional atenuada cuadráticamente con la distancia.
 */
struct PointLight {
    glm::vec3 position{0.0f, 4.5f, 0.0f};       // Ubicación en el mundo 3D
    glm::vec3 ambient{0.05f, 0.04f, 0.02f};     // Aporte ambiental local
    glm::vec3 diffuse{1.0f, 0.82f, 0.40f};      // Tono ámbar/dorado de farola de pista
    glm::vec3 specular{1.0f, 0.90f, 0.60f};     // Destello brillante
    
    // Coeficientes de atenuación: F_att = 1.0 / (c + l*d + q*d^2)
    float constant{1.0f};
    float linear{0.07f};
    float quadratic{0.017f};
    bool enabled{true};
};

/**
 * @brief 3. Fuente de Iluminación Focal (Spot Light - Faros delanteros del kart).
 * Cono de luz proyectado en la dirección frontal del vehículo con penumbra suave.
 */
struct SpotLight {
    glm::vec3 position{0.0f, 0.5f, 0.0f};       // Posición montada en el chasis
    glm::vec3 direction{0.0f, -0.1f, 1.0f};     // Vector frontal levemente inclinado a la pista
    glm::vec3 ambient{0.0f, 0.0f, 0.0f};        // No aporta ambiental fuera del cono
    glm::vec3 diffuse{1.0f, 0.98f, 0.90f};      // Luz xenón blanca brillante
    glm::vec3 specular{1.0f, 1.0f, 1.0f};       // Reflejo especular directo
    
    // Cosenos de los ángulos de apertura interior y exterior para transición suave
    float cutOff{0.9659f};       // ~15 grados (cos(15°))
    float outerCutOff{0.9396f};  // ~20 grados (cos(20°))
    
    float constant{1.0f};
    float linear{0.045f};
    float quadratic{0.0075f};
    bool enabled{true};
};

/**
 * @brief Propiedades del material de la superficie para interacción con la luz.
 */
struct Material {
    glm::vec3 ambient{1.0f};
    glm::vec3 diffuse{1.0f};
    glm::vec3 specular{0.5f};
    float shininess{32.0f};
};

} // namespace lighting
