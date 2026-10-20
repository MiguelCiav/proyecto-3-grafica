#include "scene/SceneNode.hpp"
#include "renderer/MeshGL.hpp"
#include "renderer/Shader.hpp"
#include <algorithm>

namespace scene {

SceneNode::SceneNode(std::string name)
    : m_name(std::move(name)) {}

SceneNode* SceneNode::addChild(std::unique_ptr<SceneNode> child) {
    if (!child) return nullptr;
    child->m_parent = this;
    SceneNode* rawPtr = child.get();
    m_children.push_back(std::move(child));
    return rawPtr;
}

SceneNode* SceneNode::createChild(const std::string& childName) {
    auto child = std::make_unique<SceneNode>(childName);
    return addChild(std::move(child));
}

std::unique_ptr<SceneNode> SceneNode::removeChild(SceneNode* child) {
    auto it = std::find_if(m_children.begin(), m_children.end(),
        [child](const std::unique_ptr<SceneNode>& node) {
            return node.get() == child;
        });

    if (it != m_children.end()) {
        std::unique_ptr<SceneNode> removed = std::move(*it);
        removed->m_parent = nullptr;
        m_children.erase(it);
        return removed;
    }
    return nullptr;
}

void SceneNode::updateTransform(const glm::mat4& parentWorldMatrix) {
    m_worldMatrix = parentWorldMatrix * m_localTransform.getLocalMatrix();

    for (const auto& child : m_children) {
        child->updateTransform(m_worldMatrix);
    }
}

void SceneNode::draw(const renderer::Shader& shader) const {
    if (m_mesh) {
        shader.setMat4("u_Model", m_worldMatrix);
        shader.setVec4("u_Color", m_color);
        m_mesh->draw();
    }

    for (const auto& child : m_children) {
        child->draw(shader);
    }
}

} // namespace scene
