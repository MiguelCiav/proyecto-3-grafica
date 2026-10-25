#pragma once

#include "world/TrackGrid.hpp"
#include "renderer/Shader.hpp"
#include "renderer/MeshGL.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

namespace world {

struct PropInstance {
    std::string type;
    glm::vec3 position{0.0f};
    float rotationDegrees{0.0f};
    glm::vec3 scale{1.0f};
    glm::mat4 modelMatrix{1.0f};
    glm::vec3 aabbMin{0.0f};
    glm::vec3 aabbMax{0.0f};
};

class PropSpawner {
public:
    PropSpawner();
    ~PropSpawner();

    // Carga los modelos 3D de los obstáculos y decoraciones
    bool init(const std::string& obstaclesDir = "assets/models/obstacles");

    // Genera automáticamente barreras y conos alrededor del circuito
    void generateForGrid(const TrackGrid& grid, bool isNight = false, bool isDesert = false, bool isSpace = false);

    // Dibuja todos los props en pantalla
    void render(const renderer::Shader& shader) const;

    const std::vector<PropInstance>& getProps() const { return m_props; }
    size_t getPropCount() const { return m_props.size(); }
    const std::vector<glm::vec3>& getLampPositions() const { return m_lampPositions; }

    void addProp(const std::string& type, const glm::vec3& pos, float rotDeg = 0.0f, const glm::vec3& scale = glm::vec3(1.0f));
    void clear() { m_props.clear(); m_lampPositions.clear(); }

private:
    std::vector<PropInstance> m_props;
    std::vector<glm::vec3> m_lampPositions;
    std::unordered_map<std::string, renderer::MeshGL> m_propMeshes;
    bool m_initialized{false};
};

} // namespace world
