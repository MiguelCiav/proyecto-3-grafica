#pragma once

#include "scene/SceneNode.hpp"
#include "renderer/MeshGL.hpp"
#include "geometry/KartMeshExtractor.hpp"
#include "animation/WheelAnimator.hpp"
#include <memory>
#include <string>

namespace vehicle {

/**
 * @brief Entidad jerarquica completa del kart articulado con chasis, piloto y 4 ruedas en grafo de escena.
 */
class KartEntity {
public:
    KartEntity();
    ~KartEntity() = default;

    /**
     * @brief Inicializa la jerarquia del grafo de escena a partir del ensamble procesado de Kenney.
     */
    bool setup(const geometry::KartAssembly& assembly);

    /**
     * @brief Actualiza las transformaciones jerarquicas en base a la posicion, orientacion y animador de ruedas.
     */
    void update(const glm::vec3& worldPos, float yawDegrees, const animation::WheelAnimator& animator);

    /**
     * @brief Dibuja recursivamente el grafo de escena completo del kart.
     */
    void draw(const renderer::Shader& shader) const;

    [[nodiscard]] scene::SceneNode* getRootNode() { return m_rootNode.get(); }
    [[nodiscard]] const scene::SceneNode* getRootNode() const { return m_rootNode.get(); }

    [[nodiscard]] const glm::vec3& getPosition() const { return m_rootNode->getTransform().position; }
    [[nodiscard]] float getYaw() const { return m_rootNode->getTransform().rotationEuler.y; }

    [[nodiscard]] glm::vec3 getWheelBLWorldPos() const {
        if (m_wheelBLNode) {
            return glm::vec3(m_wheelBLNode->getWorldMatrix()[3]);
        }
        return getPosition();
    }

    [[nodiscard]] glm::vec3 getWheelBRWorldPos() const {
        if (m_wheelBRNode) {
            return glm::vec3(m_wheelBRNode->getWorldMatrix()[3]);
        }
        return getPosition();
    }

private:
    std::unique_ptr<scene::SceneNode> m_rootNode;

    // Nodos hijos de partes articuladas
    scene::SceneNode* m_driverNode{nullptr};
    scene::SceneNode* m_wheelFLNode{nullptr};
    scene::SceneNode* m_wheelFRNode{nullptr};
    scene::SceneNode* m_wheelBLNode{nullptr};
    scene::SceneNode* m_wheelBRNode{nullptr};

    // Mallas OpenGL asignadas a la GPU
    renderer::MeshGL m_chassisMesh;
    renderer::MeshGL m_driverMesh;
    renderer::MeshGL m_wheelFLMesh;
    renderer::MeshGL m_wheelFRMesh;
    renderer::MeshGL m_wheelBLMesh;
    renderer::MeshGL m_wheelBRMesh;

    bool m_initialized{false};
};

} // namespace vehicle
