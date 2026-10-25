#include "lighting/LightManager.hpp"
#include "renderer/Shader.hpp"
#include <iostream>
#include <sstream>
#include <cassert>
#include <glm/geometric.hpp>

namespace lighting {

LightManager::LightManager() {
    init();
}

void LightManager::init() {
    // -------------------------------------------------------------
    // 1. Luz Direccional (Sol del circuito)
    // -------------------------------------------------------------
    m_dirLight.direction = glm::normalize(glm::vec3(-0.35f, -0.85f, -0.40f));
    m_dirLight.ambient   = glm::vec3(0.35f, 0.35f, 0.38f);
    m_dirLight.diffuse   = glm::vec3(0.88f, 0.86f, 0.82f);
    m_dirLight.specular  = glm::vec3(0.50f, 0.50f, 0.50f);
    m_dirLight.enabled   = true;

    // -------------------------------------------------------------
    // 2. Múltiples Luces Puntuales (Postes de luz / farolas del circuito)
    // -------------------------------------------------------------
    clearPointLights();

    // Farola 1: Recta de salida / meta
    PointLight light1;
    light1.position  = glm::vec3(0.0f, 4.5f, 0.0f);
    light1.ambient   = glm::vec3(0.06f, 0.05f, 0.03f);
    light1.diffuse   = glm::vec3(1.00f, 0.85f, 0.45f); // Luz ámbar cálida
    light1.specular  = glm::vec3(1.00f, 0.90f, 0.60f);
    light1.constant  = 1.0f;
    light1.linear    = 0.07f;
    light1.quadratic = 0.017f;
    light1.enabled   = true;
    addPointLight(light1);

    // Farola 2: Curva cerrada norte
    PointLight light2;
    light2.position  = glm::vec3(30.0f, 4.5f, 15.0f);
    light2.ambient   = glm::vec3(0.06f, 0.05f, 0.03f);
    light2.diffuse   = glm::vec3(1.00f, 0.80f, 0.40f);
    light2.specular  = glm::vec3(1.00f, 0.90f, 0.60f);
    light2.constant  = 1.0f;
    light2.linear    = 0.07f;
    light2.quadratic = 0.017f;
    light2.enabled   = true;
    addPointLight(light2);

    // Farola 3: Curva este
    PointLight light3;
    light3.position  = glm::vec3(45.0f, 4.5f, 45.0f);
    light3.ambient   = glm::vec3(0.06f, 0.05f, 0.03f);
    light3.diffuse   = glm::vec3(1.00f, 0.85f, 0.50f);
    light3.specular  = glm::vec3(1.00f, 0.90f, 0.60f);
    light3.constant  = 1.0f;
    light3.linear    = 0.07f;
    light3.quadratic = 0.017f;
    light3.enabled   = true;
    addPointLight(light3);

    // Farola 4: Recta posterior / túnel
    PointLight light4;
    light4.position  = glm::vec3(15.0f, 4.5f, 45.0f);
    light4.ambient   = glm::vec3(0.06f, 0.05f, 0.03f);
    light4.diffuse   = glm::vec3(0.95f, 0.82f, 0.42f);
    light4.specular  = glm::vec3(1.00f, 0.90f, 0.60f);
    light4.constant  = 1.0f;
    light4.linear    = 0.07f;
    light4.quadratic = 0.017f;
    light4.enabled   = true;
    addPointLight(light4);

    // -------------------------------------------------------------
    // 3. Luz Focal / Spotlight (Faros delanteros del kart)
    // -------------------------------------------------------------
    m_spotLight.position   = glm::vec3(0.0f, 0.45f, 0.0f);
    m_spotLight.direction  = glm::vec3(0.0f, -0.06f, 1.0f);
    m_spotLight.ambient    = glm::vec3(0.0f);
    m_spotLight.diffuse    = glm::vec3(1.60f, 1.55f, 1.45f); // Luz xenón brillante de faro
    m_spotLight.specular   = glm::vec3(1.20f, 1.20f, 1.20f);
    m_spotLight.cutOff     = 0.9135f;  // cos(24.0 deg) - Cono más amplio que baña la calzada
    m_spotLight.outerCutOff = 0.8480f; // cos(32.0 deg) - Penumbra suave envolvente
    m_spotLight.constant   = 1.0f;
    m_spotLight.linear     = 0.035f;
    m_spotLight.quadratic  = 0.0050f;
    m_spotLight.enabled    = true;

    // Propiedades de material base
    m_defaultMaterial.ambient   = glm::vec3(1.0f);
    m_defaultMaterial.diffuse   = glm::vec3(1.0f);
    m_defaultMaterial.specular  = glm::vec3(0.5f);
    m_defaultMaterial.shininess = 32.0f;

    m_currentModel = LightingModel::BLINN_PHONG;
    m_pointLightsEnabled = true;
}

void LightManager::addPointLight(const PointLight& light) {
    if (m_pointLights.size() < MAX_POINT_LIGHTS) {
        m_pointLights.push_back(light);
    } else {
        std::cerr << "[LightManager] Limite maximo de fuentes puntuales alcanzado (" << MAX_POINT_LIGHTS << ")." << std::endl;
    }
}

void LightManager::clearPointLights() {
    m_pointLights.clear();
}

PointLight& LightManager::getPointLight(size_t index) {
    assert(index < m_pointLights.size());
    return m_pointLights[index];
}

const PointLight& LightManager::getPointLight(size_t index) const {
    assert(index < m_pointLights.size());
    return m_pointLights[index];
}

void LightManager::togglePointLights() {
    m_pointLightsEnabled = !m_pointLightsEnabled;
    for (auto& pt : m_pointLights) {
        pt.enabled = m_pointLightsEnabled;
    }
}

void LightManager::updateKartHeadlights(const glm::vec3& kartPos, const glm::vec3& forwardDir) {
    glm::vec3 dir = forwardDir;
    if (glm::length(dir) < 0.0001f) {
        dir = glm::vec3(0.0f, 0.0f, 1.0f);
    } else {
        dir = glm::normalize(dir);
    }

    // Los faros emergen del morro delantero del kart a 0.45m del suelo
    m_spotLight.position = kartPos + dir * 0.75f + glm::vec3(0.0f, 0.45f, 0.0f);

    // Haz proyectado hacia adelante y con un ligero declive hacia la pista (-0.06 en Y)
    m_spotLight.direction = glm::normalize(dir + glm::vec3(0.0f, -0.06f, 0.0f));
}

void LightManager::setLightingModel(LightingModel model) {
    m_currentModel = model;
    std::cout << "[LightManager] Modelo de sombreado conmutado a: " << getCurrentModelName() << std::endl;
}

void LightManager::cycleNextModel() {
    int next = (static_cast<int>(m_currentModel) + 1) % 3;
    setLightingModel(static_cast<LightingModel>(next));
}

void LightManager::applyToShader(const renderer::Shader& shader, const glm::vec3& viewPos) const {
    if (!shader.isValid()) return;

    // 0 = Flat, 1 = Lambert, 2 = Blinn-Phong
    shader.setInt("u_LightingModel", static_cast<int>(m_currentModel));
    shader.setVec3("u_ViewPos", viewPos);

    // -------------------------------------------------------------
    // 1. Luz Direccional (Sol)
    // -------------------------------------------------------------
    shader.setBool("u_DirLight.enabled", m_dirLight.enabled);
    shader.setVec3("u_DirLight.direction", m_dirLight.direction);
    shader.setVec3("u_DirLight.ambient", m_dirLight.ambient);
    shader.setVec3("u_DirLight.diffuse", m_dirLight.diffuse);
    shader.setVec3("u_DirLight.specular", m_dirLight.specular);

    // Compatibilidad adicional con formato sin prefijo 'u_'
    shader.setBool("dirLight.enabled", m_dirLight.enabled);
    shader.setVec3("dirLight.direction", m_dirLight.direction);
    shader.setVec3("dirLight.ambient", m_dirLight.ambient);
    shader.setVec3("dirLight.diffuse", m_dirLight.diffuse);
    shader.setVec3("dirLight.specular", m_dirLight.specular);

    // -------------------------------------------------------------
    // 2. Múltiples Luces Puntuales (Farolas de circuito con atenuación calculada)
    // -------------------------------------------------------------
    int numPoints = static_cast<int>(std::min(m_pointLights.size(), MAX_POINT_LIGHTS));
    shader.setInt("u_NumPointLights", numPoints);
    shader.setInt("numPointLights", numPoints);

    for (int i = 0; i < numPoints; ++i) {
        const auto& pt = m_pointLights[i];
        std::string idxStr = std::to_string(i);

        // Formato oficial: pointLights[i].*
        std::string pfx = "pointLights[" + idxStr + "].";
        shader.setBool(pfx + "enabled", pt.enabled);
        shader.setVec3(pfx + "position", pt.position);
        shader.setVec3(pfx + "ambient", pt.ambient);
        shader.setVec3(pfx + "diffuse", pt.diffuse);
        shader.setVec3(pfx + "specular", pt.specular);
        shader.setFloat(pfx + "constant", pt.constant);
        shader.setFloat(pfx + "linear", pt.linear);
        shader.setFloat(pfx + "quadratic", pt.quadratic);

        // Formato con prefijo 'u_': u_PointLights[i].*
        std::string uPfx = "u_PointLights[" + idxStr + "].";
        shader.setBool(uPfx + "enabled", pt.enabled);
        shader.setVec3(uPfx + "position", pt.position);
        shader.setVec3(uPfx + "ambient", pt.ambient);
        shader.setVec3(uPfx + "diffuse", pt.diffuse);
        shader.setVec3(uPfx + "specular", pt.specular);
        shader.setFloat(uPfx + "constant", pt.constant);
        shader.setFloat(uPfx + "linear", pt.linear);
        shader.setFloat(uPfx + "quadratic", pt.quadratic);
    }

    // -------------------------------------------------------------
    // 3. Luz Focal / Spotlight (Faros del kart)
    // -------------------------------------------------------------
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

    // Compatibilidad adicional sin prefijo 'u_'
    shader.setBool("spotLight.enabled", m_spotLight.enabled);
    shader.setVec3("spotLight.position", m_spotLight.position);
    shader.setVec3("spotLight.direction", m_spotLight.direction);
    shader.setVec3("spotLight.ambient", m_spotLight.ambient);
    shader.setVec3("spotLight.diffuse", m_spotLight.diffuse);
    shader.setVec3("spotLight.specular", m_spotLight.specular);
    shader.setFloat("spotLight.cutOff", m_spotLight.cutOff);
    shader.setFloat("spotLight.outerCutOff", m_spotLight.outerCutOff);
    shader.setFloat("spotLight.constant", m_spotLight.constant);
    shader.setFloat("spotLight.linear", m_spotLight.linear);
    shader.setFloat("spotLight.quadratic", m_spotLight.quadratic);

    // -------------------------------------------------------------
    // Material
    // -------------------------------------------------------------
    shader.setFloat("u_Material.shininess", m_defaultMaterial.shininess);
    shader.setFloat("material.shininess", m_defaultMaterial.shininess);
}

} // namespace lighting
