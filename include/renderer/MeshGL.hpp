#pragma once

#include "geometry/GeometryData.hpp"
#include <cstdint>

namespace renderer {

class MeshGL {
public:
    MeshGL();
    ~MeshGL();

    // Eliminar copia
    MeshGL(const MeshGL&) = delete;
    MeshGL& operator=(const MeshGL&) = delete;

    // Permitir movimiento
    MeshGL(MeshGL&& other) noexcept;
    MeshGL& operator=(MeshGL&& other) noexcept;

    // Crea y transfiere los buffers a la GPU
    void setup(const geometry::SubMesh& submesh);
    void destroy();

    // Dibuja la malla enlazando el VAO
    void draw() const;

    unsigned int getVAO() const { return m_vao; }
    uint32_t getIndexCount() const { return m_indexCount; }
    bool isLoaded() const { return m_vao != 0 && m_indexCount > 0; }

private:
    unsigned int m_vao{0};
    unsigned int m_vbo{0};
    unsigned int m_ebo{0};
    uint32_t m_indexCount{0};
};

} // namespace renderer
