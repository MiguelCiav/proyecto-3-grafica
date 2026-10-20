#pragma once

#include <string>

namespace renderer {

class Texture {
public:
    Texture();
    ~Texture();

    // Eliminar copia
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Permitir movimiento
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    // Carga la textura desde archivo y la sube a la GPU con Mipmaps
    bool loadFromFile(const std::string& filePath, bool flipY = true);
    void destroy();

    // Enlaza la textura en una unidad de textura de OpenGL (GL_TEXTURE0 + slot)
    void bind(unsigned int slot = 0) const;
    void unbind() const;

    unsigned int getID() const { return m_textureID; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getChannels() const { return m_channels; }
    bool isLoaded() const { return m_textureID != 0; }
    const std::string& getPath() const { return m_path; }

private:
    unsigned int m_textureID{0};
    int m_width{0};
    int m_height{0};
    int m_channels{0};
    std::string m_path;
};

} // namespace renderer
