#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace renderer {

class Shader {
public:
    Shader();
    ~Shader();

    // Prohibir copia para evitar duplicación de ID de GPU
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Permitir movimiento
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    // Carga y compilación desde archivos o código fuente directo
    bool loadFromFiles(const std::string& vertPath, const std::string& fragPath, const std::string& geomPath = "");
    bool loadFromSource(const std::string& vertSource, const std::string& fragSource, const std::string& geomSource = "");

    void use() const;
    void bind() const { use(); }
    void unbind() const;

    unsigned int getID() const { return m_programID; }
    bool isValid() const { return m_programID != 0; }

    // Setters de uniformes con caché de ubicaciones
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

private:
    unsigned int compileShader(unsigned int type, const std::string& source);
    int getUniformLocation(const std::string& name) const;
    static std::string readFile(const std::string& filePath);

    unsigned int m_programID{0};
    mutable std::unordered_map<std::string, int> m_uniformLocations;
};

} // namespace renderer
