#pragma once

#include "physics/Collider.hpp"
#include <glm/glm.hpp>

namespace physics {

class CollisionSystem;
class KartPhysics;

/**
 * @brief Estructura estandarizada con los resultados geometricos y fisicos de un contacto.
 */
struct CollisionHit {
    bool collided{false};
    glm::vec3 normal{0.0f};
    glm::vec3 mtv{0.0f}; // Minimum Translation Vector
    float penetration{0.0f};

    // Alias de conveniencia para compatibilidad con SUBAGENTE_4_ANALISIS.md
    bool hasCollision{false};

    [[nodiscard]] bool isCollided() const { return collided; }
    explicit operator bool() const { return collided; }
};

/**
 * @brief Solucionador cinematico y estatico de colisiones.
 * Desacopla la logica de resolucion fisica de los componentes de escena, entidad y camara.
 */
class CollisionSolver {
public:
    /**
     * @brief Consulta y resuelve el impacto de una esfera envolvente contra el sistema de colisiones.
     * @param sphere Esfera a comprobar.
     * @param system Sistema de colisiones del circuito.
     * @return CollisionHit con indicador de impacto, normal, penetracion y MTV.
     */
    static CollisionHit solveSphereAgainstSystem(const BoundingSphere& sphere, const CollisionSystem& system);

    /**
     * @brief Calcula la respuesta cinematica elastica/inelastica frente a una normal de contacto.
     * @param currentVelocity Vector velocidad lineal antes del choque.
     * @param collisionNormal Normal de contacto apuntando fuera del obstaculo.
     * @param restitution Coeficiente de restitucion elastica (0.0 = deslizamiento/inelastico, 1.0 = rebote perfecto).
     * @return Nuevo vector de velocidad tras el contacto.
     */
    static glm::vec3 computeElasticResponse(const glm::vec3& currentVelocity, const glm::vec3& collisionNormal, float restitution = 0.25f);

    /**
     * @brief Resuelve la posicion de una camara libre FPS evitando interpenetracion con obstaculos y acotando Y.
     * @param cameraPos Posicion propuesta de la camara.
     * @param cameraRadius Radio envolvente de la camara.
     * @param system Sistema de colisiones del circuito.
     * @param minY Cota minima vertical (evita atravesar el suelo).
     * @param maxY Cota maxima vertical (limite de altura).
     * @return Posicion corregida fuera de obstaculos y dentro de [minY, maxY].
     */
    static glm::vec3 resolveCameraPosition(const glm::vec3& cameraPos,
                                           float cameraRadius,
                                           const CollisionSystem& system,
                                           float minY = 0.5f,
                                           float maxY = 80.0f);

    /**
     * @brief Resuelve iterativamente la posicion de una esfera evitando interpenetracion.
     * @param system Sistema de colisiones.
     * @param position Posicion inicial.
     * @param radius Radio envolvente.
     * @param maxIterations Numero maximo de iteraciones de relajacion.
     * @return Posicion resuelta.
     */
    static glm::vec3 resolveSpherePosition(const CollisionSystem& system,
                                           const glm::vec3& position,
                                           float radius,
                                           int maxIterations = 3);

    /**
     * @brief Resuelve el impacto de un kart contra un obstaculo estatico (Wall Sliding / Rebote).
     */
    static void resolveKartContact(KartPhysics& kart, const glm::vec3& mtv, float reboundFactor = 0.25f);
};

} // namespace physics

namespace Engine::Physics {
    using CollisionHit = ::physics::CollisionHit;
    using CollisionSolver = ::physics::CollisionSolver;
}
