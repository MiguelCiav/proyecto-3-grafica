#include "renderer/MeshGL.hpp"
#include <glad/glad.h>
#include <utility>

namespace renderer {

MeshGL::MeshGL() = default;

MeshGL::~MeshGL() {
    destroy();
}

MeshGL::MeshGL(MeshGL&& other) noexcept
    : m_vao(other.m_vao)
    , m_vbo(other.m_vbo)
    , m_ebo(other.m_ebo)
    , m_indexCount(other.m_indexCount)
{
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
    other.m_indexCount = 0;
}

MeshGL& MeshGL::operator=(MeshGL&& other) noexcept {
    if (this != &other) {
        destroy();
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_ebo = other.m_ebo;
        m_indexCount = other.m_indexCount;

        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_ebo = 0;
        other.m_indexCount = 0;
    }
    return *this;
}

void MeshGL::destroy() {
    if (m_ebo != 0) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    m_indexCount = 0;
}

void MeshGL::setup(const geometry::SubMesh& submesh) {
    destroy();

    if (submesh.vertices.empty() || submesh.indices.empty()) {
        return;
    }

    m_indexCount = static_cast<uint32_t>(submesh.indices.size());

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    // Subir Vértices al VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 submesh.vertices.size() * sizeof(geometry::Vertex),
                 submesh.vertices.data(),
                 GL_STATIC_DRAW);

    // Subir Índices al EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 submesh.indices.size() * sizeof(uint32_t),
                 submesh.indices.data(),
                 GL_STATIC_DRAW);

    // Atributo 0: Posición (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(geometry::Vertex),
                          reinterpret_cast<void*>(offsetof(geometry::Vertex, position)));

    // Atributo 1: Normal (vec3)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(geometry::Vertex),
                          reinterpret_cast<void*>(offsetof(geometry::Vertex, normal)));

    // Atributo 2: Coordenadas UV (vec2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(geometry::Vertex),
                          reinterpret_cast<void*>(offsetof(geometry::Vertex, texCoords)));

    // Atributo 3: Color de vértice difuso / material (vec4)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
                          sizeof(geometry::Vertex),
                          reinterpret_cast<void*>(offsetof(geometry::Vertex, color)));

    glBindVertexArray(0);
}

void MeshGL::draw() const {
    if (m_vao != 0 && m_indexCount > 0) {
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}

} // namespace renderer
