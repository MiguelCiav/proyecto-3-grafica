#pragma once

#include "geometry/GeometryData.hpp"
#include <glm/glm.hpp>
#include <string>

namespace geometry {

/**
 * @brief Generador profundo de primitivas y mallas procedurales 3D en CPU (Zero Blender).
 * Proporciona un conjunto completo de geometrías testeables sin requerir contexto OpenGL.
 */
class ProceduralGeometry {
public:
    // Primitivas volumétricas estándar
    static SubMesh createBox(float hx, float hy, float hz, const std::string& name = "Procedural_Box");
    static SubMesh createSphere(float radius, int rings = 6, int sectors = 10, const std::string& name = "Procedural_Sphere");

    // Formas orgánicas y elementos de utilería temática
    static SubMesh createStar(float outerRadius, float innerRadius, float depth, const std::string& name = "Procedural_Star");
    static SubMesh createFacetedDome(float radius, float height, int rings = 3, int segments = 8, const std::string& name = "Procedural_Faceted_Dome");
    static SubMesh createBrazier(float baseRadius, float bowlRadius, float totalHeight, const std::string& name = "Procedural_Fire_Brazier");
    static SubMesh createFlame(float radius, float height, bool isCore = false);
    static SubMesh createClamShell(float radius = 0.52f, float openAngleDeg = 68.0f, const std::string& name = "Procedural_Scallop_Clam");
    static SubMesh createBubbleColumn(const std::string& name = "Procedural_Bubble_Column");
};

} // namespace geometry
