#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "renderer/Shader.hpp"

namespace renderer {

class Skybox {
public:
    Skybox();
    ~Skybox();

    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;

    Skybox(Skybox&& other) noexcept;
    Skybox& operator=(Skybox&& other) noexcept;

    bool loadFromFiles(const std::vector<std::string>& faces);
    bool loadFromDirectory(const std::string& directoryPath, const std::string& extension = ".png");
    bool loadShader(const std::string& vertPath, const std::string& fragPath);

    void draw(const glm::mat4& view, const glm::mat4& projection) const;
    void draw(const glm::mat4& view, const glm::mat4& projection, const Shader& shader) const;

    void destroy();

    void setTint(const glm::vec3& tint) { m_tint = tint; }
    const glm::vec3& getTint() const { return m_tint; }

    unsigned int getTextureID() const { return m_textureID; }
    unsigned int getVAO() const { return m_vao; }
    bool isLoaded() const { return m_textureID != 0 && m_vao != 0; }
    Shader* getShader() const { return m_shader.get(); }

private:
    void setupMesh();
    bool loadCubemap(const std::vector<std::string>& faces);

    unsigned int m_textureID{0};
    unsigned int m_vao{0};
    unsigned int m_vbo{0};
    glm::vec3 m_tint{1.0f};
    std::unique_ptr<Shader> m_shader;
};

} // namespace renderer
