#include "lighting/LightingSystem.hpp"
#include "renderer/Shader.hpp"
#include <iostream>
#include <glm/geometric.hpp>

namespace lighting {

LightingSystem::LightingSystem() {
    init();
}

void LightingSystem::init() {
    // 1. Sol diurno del circuito (Direccional)
    m_dirLight.direction = glm::normalize(glm::vec3(-0.35f, -0.85f, -0.40f));
    m_dirLight.ambient   = glm::vec3(0.35f, 0.35f, 0.38f);
    m_dirLight.diffuse   = glm::vec3(0.88f, 0.86f, 0.82f);
    m_dirLight.specular  = glm::vec3(0.50f, 0.50f, 0.50f);
    m_dirLight.enabled   = true;

    // 2. Farola de circuito (Puntual)
    // Posicionada en una curva representativa de la pista
    m_pointLight.position  = glm::vec3(20.0f, 4.5f, 20.0f);
    m_pointLight.ambient   = glm::vec3(0.05f, 0.04f, 0.02f);
    m_pointLight.diffuse   = glm::vec3(1.0f, 0.82f, 0.40f); // Luz cálida dorada
    m_pointLight.specular  = glm::vec3(1.0f, 0.90f, 0.60f);
    m_pointLight.constant  = 1.0f;
    m_pointLight.linear    = 0.07f;
    m_pointLight.quadratic = 0.017f;
    m_pointLight.enabled   = true;

    // 3. Faros frontales del kart (Focal / Spot Light)
    m_spotLight.position   = glm::vec3(0.0f, 0.45f, 0.0f);
    m_spotLight.direction  = glm::vec3(0.0f, -0.06f, 1.0f);
    m_spotLight.ambient    = glm::vec3(0.0f);
    m_spotLight.diffuse    = glm::vec3(1.0f, 0.98f, 0.90f); // Luz xenón brillante
    m_spotLight.specular   = glm::vec3(1.0f, 1.0f, 1.0f);
    m_spotLight.cutOff     = 0.9659f;  // cos(15.0 deg)
    m_spotLight.outerCutOff = 0.9396f; // cos(20.0 deg)
    m_spotLight.constant   = 1.0f;
    m_spotLight.linear     = 0.045f;
    m_spotLight.quadratic  = 0.0075f;
    m_spotLight.enabled    = true;

    // Propiedades de material base
    m_defaultMaterial.ambient   = glm::vec3(1.0f);
    m_defaultMaterial.diffuse   = glm::vec3(1.0f);
    m_defaultMaterial.specular  = glm::vec3(0.5f);
    m_defaultMaterial.shininess = 32.0f;

    m_currentModel = LightingModel::BLINN_PHONG;
}

void LightingSystem::updateKartHeadlights(const glm::vec3& kartPos, const glm::vec3& forwardDir) {
    glm::vec3 dir = forwardDir;
    if (glm::length(dir) < 0.0001f) {
        dir = glm::vec3(0.0f, 0.0f, 1.0f);
    } else {
        dir = glm::normalize(dir);
    }

    // Los faros emergen de la trompa frontal del kart a 0.45m del suelo
    m_spotLight.position = kartPos + dir * 0.75f + glm::vec3(0.0f, 0.45f, 0.0f);

    // Haz proyectado hacia adelante y con un ligero declive hacia la pista
    m_spotLight.direction = glm::normalize(dir + glm::vec3(0.0f, -0.06f, 0.0f));
}

void LightingSystem::setPointLightPosition(const glm::vec3& position) {
    m_pointLight.position = position;
}

void LightingSystem::setLightingModel(LightingModel model) {
    m_currentModel = model;
    std::cout << "[Iluminacion] Modelo de sombreado conmutado a: " << getCurrentModelName() << std::endl;
}

void LightingSystem::cycleNextModel() {
    int next = (static_cast<int>(m_currentModel) + 1) % 3;
    setLightingModel(static_cast<LightingModel>(next));
}

void LightingSystem::applyToShader(const renderer::Shader& shader, const glm::vec3& viewPos) const {
    if (!shader.isValid()) return;

    // 0 = Flat, 1 = Lambert, 2 = Blinn-Phong
    shader.setInt("u_LightingModel", static_cast<int>(m_currentModel));
    shader.setVec3("u_ViewPos", viewPos);

    // 1. Luz Direccional (Sol)
    shader.setBool("u_DirLight.enabled", m_dirLight.enabled);
    shader.setVec3("u_DirLight.direction", m_dirLight.direction);
    shader.setVec3("u_DirLight.ambient", m_dirLight.ambient);
    shader.setVec3("u_DirLight.diffuse", m_dirLight.diffuse);
    shader.setVec3("u_DirLight.specular", m_dirLight.specular);

    // 2. Luz Puntual (Farola de pista)
    shader.setBool("u_PointLight.enabled", m_pointLight.enabled);
    shader.setVec3("u_PointLight.position", m_pointLight.position);
    shader.setVec3("u_PointLight.ambient", m_pointLight.ambient);
    shader.setVec3("u_PointLight.diffuse", m_pointLight.diffuse);
    shader.setVec3("u_PointLight.specular", m_pointLight.specular);
    shader.setFloat("u_PointLight.constant", m_pointLight.constant);
    shader.setFloat("u_PointLight.linear", m_pointLight.linear);
    shader.setFloat("u_PointLight.quadratic", m_pointLight.quadratic);

    // 3. Luz Focal (Faros del kart)
    shader.setBool("u_SpotLight.enabled", m_spotLight.enabled);
    shader.setVec3("u_SpotLight.position", m_spotLight.position);
    shader.setVec3("u_SpotLight.direction", m_spotLight.direction);
    shader.setVec3("u_SpotLight.ambient", m_spotLight.ambient);
    shader.setVec3("u_SpotLight.diffuse", m_spotLight.diffuse);
    shader.setVec3("u_SpotLight.specular", m_spotLight.specular);
    shader.setFloat("u_SpotLight.cutOff", m_spotLight.cutOff);
    shader.setFloat("u_SpotLight.outerCutOff", m_spotLight.outerCutOff);
    shader.setFloat("u_SpotLight.constant", m_spotLight.constant);
    shader.setFloat("u_SpotLight.linear", m_spotLight.linear);
    shader.setFloat("u_SpotLight.quadratic", m_spotLight.quadratic);

    // Material
    shader.setFloat("u_Material.shininess", m_defaultMaterial.shininess);
}

} // namespace lighting
