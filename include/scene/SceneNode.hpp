#pragma once

#include "scene/Transform.hpp"
#include <vector>
#include <memory>
#include <string>

namespace renderer {
    class MeshGL;
    class Shader;
}

namespace scene {

/**
 * @brief Nodo jerarquico para grafos de escena 3D.
 */
class SceneNode {
public:
    explicit SceneNode(std::string name = "SceneNode");
    ~SceneNode() = default;

    // Jerarquia de nodos
    SceneNode* addChild(std::unique_ptr<SceneNode> child);
    SceneNode* createChild(const std::string& childName);
    std::unique_ptr<SceneNode> removeChild(SceneNode* child);

    [[nodiscard]] SceneNode* getParent() const { return m_parent; }
    [[nodiscard]] const std::vector<std::unique_ptr<SceneNode>>& getChildren() const { return m_children; }
    [[nodiscard]] const std::string& getName() const { return m_name; }

    // Transformaciones
    [[nodiscard]] Transform& getTransform() { return m_localTransform; }
    [[nodiscard]] const Transform& getTransform() const { return m_localTransform; }
    [[nodiscard]] const glm::mat4& getWorldMatrix() const { return m_worldMatrix; }

    /**
     * @brief Propaga recursivamente la matriz de mundo a todos los descendientes.
     * M_world = M_parent * M_local
     */
    void updateTransform(const glm::mat4& parentWorldMatrix = glm::mat4(1.0f));

    // Malla y renderizado
    void setMesh(renderer::MeshGL* mesh) { m_mesh = mesh; }
    [[nodiscard]] renderer::MeshGL* getMesh() const { return m_mesh; }

    void setColor(const glm::vec4& color) { m_color = color; }
    [[nodiscard]] const glm::vec4& getColor() const { return m_color; }

    /**
     * @brief Dibuja recursivamente este nodo y sus hijos con el shader dado.
     */
    void draw(const renderer::Shader& shader) const;

private:
    std::string m_name;
    Transform m_localTransform;
    glm::mat4 m_worldMatrix{1.0f};

    SceneNode* m_parent{nullptr};
    std::vector<std::unique_ptr<SceneNode>> m_children;

    renderer::MeshGL* m_mesh{nullptr};
    glm::vec4 m_color{1.0f};
};

} // namespace scene
