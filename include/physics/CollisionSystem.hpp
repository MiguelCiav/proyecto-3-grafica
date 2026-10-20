#pragma once

#include "physics/Collider.hpp"
#include <vector>

namespace Engine::Physics {

/**
 * @brief Sistema de registro de obstaculos y resolucion de colisiones del mundo.
 */
class CollisionSystem {
public:
    CollisionSystem() = default;
    ~CollisionSystem() = default;

    void addStaticBox(const BoundingBox& box);
    void clear();

    [[nodiscard]] const std::vector<BoundingBox>& getStaticBoxes() const { return m_staticBoxes; }

    /**
     * @brief Consulta si una esfera colisiona con cualquier obstaculo estatico.
     * @param sphere Esfera envolvente a probar.
     * @param outMTV Suma o vector de penetracion maximo para corregir la posicion.
     * @return true si existe al menos una colision.
     */
    bool checkCollision(const BoundingSphere& sphere, glm::vec3& outMTV) const;

    /**
     * @brief Consulta colision de una caja AABB contra los obstaculos estaticos.
     */
    bool checkCollision(const BoundingBox& box, glm::vec3& outMTV) const;

    /**
     * @brief Resuelve iterativamente la posicion de una esfera evitando que penetre colisionadores.
     * @param currentPos Posicion proyectada.
     * @param radius Radio de la esfera envolvente.
     * @param maxIterations Limite de iteraciones para esquinas.
     * @return Posicion final corregida.
     */
    glm::vec3 resolveSpherePosition(const glm::vec3& currentPos, float radius, int maxIterations = 3) const;

private:
    std::vector<BoundingBox> m_staticBoxes;
};

} // namespace Engine::Physics
