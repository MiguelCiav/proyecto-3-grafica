#include "renderer/Shader.hpp"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

namespace renderer {

Shader::Shader() = default;

Shader::~Shader() {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
        m_programID = 0;
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_programID(other.m_programID)
    , m_uniformLocations(std::move(other.m_uniformLocations))
{
    other.m_programID = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (m_programID != 0) {
            glDeleteProgram(m_programID);
        }
        m_programID = other.m_programID;
        m_uniformLocations = std::move(other.m_uniformLocations);
        other.m_programID = 0;
    }
    return *this;
}

std::string Shader::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[ERROR] [Shader] No se pudo abrir el archivo de shader: " << filePath << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source) {
    if (source.empty()) return 0;

    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::string typeStr = (type == GL_VERTEX_SHADER) ? "VERTEX" :
                              (type == GL_FRAGMENT_SHADER) ? "FRAGMENT" : "GEOMETRY";
        std::cerr << "[ERROR] [Shader] Error de compilacion en shader " << typeStr << ":\n"
                  << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::loadFromFiles(const std::string& vertPath, const std::string& fragPath, const std::string& geomPath) {
    std::string vertCode = readFile(vertPath);
    std::string fragCode = readFile(fragPath);
    std::string geomCode = geomPath.empty() ? "" : readFile(geomPath);

    if (vertCode.empty() || fragCode.empty()) {
        return false;
    }

    return loadFromSource(vertCode, fragCode, geomCode);
}

bool Shader::loadFromSource(const std::string& vertSource, const std::string& fragSource, const std::string& geomSource) {
    unsigned int vertShader = compileShader(GL_VERTEX_SHADER, vertSource);
    unsigned int fragShader = compileShader(GL_FRAGMENT_SHADER, fragSource);
    unsigned int geomShader = 0;

    if (!vertShader || !fragShader) {
        if (vertShader) glDeleteShader(vertShader);
        if (fragShader) glDeleteShader(fragShader);
        return false;
    }

    if (!geomSource.empty()) {
        geomShader = compileShader(GL_GEOMETRY_SHADER, geomSource);
        if (!geomShader) {
            glDeleteShader(vertShader);
            glDeleteShader(fragShader);
            return false;
        }
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    if (geomShader) {
        glAttachShader(program, geomShader);
    }
    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "[ERROR] [Shader] Error de enlace de programa shader:\n"
                  << infoLog << std::endl;
        glDeleteProgram(program);
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        if (geomShader) glDeleteShader(geomShader);
        return false;
    }

    // Los shaders individuales ya quedaron enlazados en el ejecutable binario de GPU
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    if (geomShader) glDeleteShader(geomShader);

    if (m_programID != 0) {
        glDeleteProgram(m_programID);
    }
    m_programID = program;
    m_uniformLocations.clear();

    return true;
}

void Shader::use() const {
    if (m_programID != 0) {
        glUseProgram(m_programID);
    }
}

void Shader::unbind() const {
    glUseProgram(0);
}

int Shader::getUniformLocation(const std::string& name) const {
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end()) {
        return it->second;
    }

    int location = glGetUniformLocation(m_programID, name.c_str());
    m_uniformLocations[name] = location;
    return location;
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setMat3(const std::string& name, const glm::mat3& value) const {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

} // namespace renderer
