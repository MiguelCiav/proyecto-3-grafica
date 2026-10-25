#include "renderer/Skybox.hpp"
#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>

namespace renderer {

namespace {

const float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

} // namespace

Skybox::Skybox() = default;

Skybox::~Skybox() {
    destroy();
}

Skybox::Skybox(Skybox&& other) noexcept
    : m_textureID(other.m_textureID)
    , m_vao(other.m_vao)
    , m_vbo(other.m_vbo)
    , m_shader(std::move(other.m_shader))
{
    other.m_textureID = 0;
    other.m_vao = 0;
    other.m_vbo = 0;
}

Skybox& Skybox::operator=(Skybox&& other) noexcept {
    if (this != &other) {
        destroy();
        m_textureID = other.m_textureID;
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_shader = std::move(other.m_shader);

        other.m_textureID = 0;
        other.m_vao = 0;
        other.m_vbo = 0;
    }
    return *this;
}

bool Skybox::loadFromFiles(const std::vector<std::string>& faces) {
    if (!loadCubemap(faces)) {
        return false;
    }
    setupMesh();
    return true;
}

bool Skybox::loadFromDirectory(const std::string& directoryPath, const std::string& extension) {
    std::string prefix = directoryPath;
    if (!prefix.empty() && prefix.back() != '/' && prefix.back() != '\\') {
        prefix += "/";
    }

    std::vector<std::string> faces = {
        prefix + "right" + extension,
        prefix + "left" + extension,
        prefix + "top" + extension,
        prefix + "bottom" + extension,
        prefix + "front" + extension,
        prefix + "back" + extension
    };

    return loadFromFiles(faces);
}

bool Skybox::loadShader(const std::string& vertPath, const std::string& fragPath) {
    m_shader = std::make_unique<Shader>();
    if (!m_shader->loadFromFiles(vertPath, fragPath)) {
        std::cerr << "Skybox failed to load shader: " << vertPath << ", " << fragPath << std::endl;
        m_shader.reset();
        return false;
    }
    return true;
}

void Skybox::setupMesh() {
    if (m_vao != 0) return;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool Skybox::loadCubemap(const std::vector<std::string>& faces) {
    if (faces.size() != 6) {
        std::cerr << "Skybox error: Exactly 6 faces are required for cubemap (right, left, top, bottom, front, back)." << std::endl;
        return false;
    }

    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }

    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

    stbi_set_flip_vertically_on_load(false);

    for (unsigned int i = 0; i < faces.size(); ++i) {
        int width = 0;
        int height = 0;
        int nrChannels = 0;
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = GL_RGB;
            if (nrChannels == 1) {
                format = GL_RED;
            } else if (nrChannels == 4) {
                format = GL_RGBA;
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cerr << "Skybox failed to load face image: " << faces[i] << std::endl;
            stbi_image_free(data);
            destroy();
            return false;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return true;
}

void Skybox::draw(const glm::mat4& view, const glm::mat4& projection) const {
    if (!isLoaded()) return;
    if (m_shader && m_shader->isValid()) {
        draw(view, projection, *m_shader);
    }
}

void Skybox::draw(const glm::mat4& view, const glm::mat4& projection, const Shader& shader) const {
    if (!isLoaded() || !shader.isValid()) return;

    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

    shader.use();
    shader.setMat4("u_View", view);
    shader.setMat4("u_Projection", projection);
    shader.setInt("u_Skybox", 0);
    shader.setVec3("u_SkyTint", m_tint);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
}

void Skybox::destroy() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
    m_shader.reset();
}

} // namespace renderer
