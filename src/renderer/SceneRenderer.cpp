#include "renderer/SceneRenderer.hpp"
#include "renderer/Shader.hpp"
#include "renderer/Texture.hpp"
#include "renderer/Skybox.hpp"
#include "renderer/TrackRenderer.hpp"
#include "renderer/MeshGL.hpp"
#include "world/TrackGrid.hpp"
#include "world/PropSpawner.hpp"
#include "world/BoosterPad.hpp"
#include "world/TrackEnvironmentRegistry.hpp"
#include "vehicle/KartEntity.hpp"
#include "vfx/KartParticleEmitter.hpp"
#include "vfx/SkidmarkManager.hpp"
#include "lighting/LightManager.hpp"
#include <glad/glad.h>
#include <iostream>

namespace renderer {

SceneRenderer::SceneRenderer() = default;
SceneRenderer::~SceneRenderer() = default;

SceneRenderer::SceneRenderer(SceneRenderer&& other) noexcept
    : m_ownedLitShader(std::move(other.m_ownedLitShader))
    , m_ownedBoosterShader(std::move(other.m_ownedBoosterShader))
    , m_litShader(other.m_litShader == &other.m_ownedLitShader ? &m_ownedLitShader : other.m_litShader)
    , m_boosterShader(other.m_boosterShader == &other.m_ownedBoosterShader ? &m_ownedBoosterShader : other.m_boosterShader)
    , m_paletteTexture(other.m_paletteTexture)
    , m_initialized(other.m_initialized) {
    other.m_litShader = nullptr;
    other.m_boosterShader = nullptr;
    other.m_paletteTexture = nullptr;
    other.m_initialized = false;
}

SceneRenderer& SceneRenderer::operator=(SceneRenderer&& other) noexcept {
    if (this != &other) {
        m_ownedLitShader = std::move(other.m_ownedLitShader);
        m_ownedBoosterShader = std::move(other.m_ownedBoosterShader);
        m_litShader = (other.m_litShader == &other.m_ownedLitShader) ? &m_ownedLitShader : other.m_litShader;
        m_boosterShader = (other.m_boosterShader == &other.m_ownedBoosterShader) ? &m_ownedBoosterShader : other.m_boosterShader;
        m_paletteTexture = other.m_paletteTexture;
        m_initialized = other.m_initialized;

        other.m_litShader = nullptr;
        other.m_boosterShader = nullptr;
        other.m_paletteTexture = nullptr;
        other.m_initialized = false;
    }
    return *this;
}

bool SceneRenderer::init(const std::string& shadersDir) {
    std::string litVert = shadersDir + "/lit.vert";
    std::string litFrag = shadersDir + "/lit.frag";
    if (!m_ownedLitShader.loadFromFiles(litVert, litFrag)) {
        std::cerr << "[ERROR] [SceneRenderer] Fallo al compilar litShader desde "
                  << litVert << " y " << litFrag << std::endl;
        return false;
    }
    m_litShader = &m_ownedLitShader;

    std::string boosterVert = shadersDir + "/booster_procedural.vert";
    std::string boosterFrag = shadersDir + "/booster_procedural.frag";
    if (!m_ownedBoosterShader.loadFromFiles(boosterVert, boosterFrag)) {
        std::cerr << "[ERROR] [SceneRenderer] Fallo al compilar boosterShader desde "
                  << boosterVert << " y " << boosterFrag << std::endl;
        return false;
    }
    m_boosterShader = &m_ownedBoosterShader;

    m_initialized = true;
    std::cout << "[OK] [SceneRenderer] Pipeline de renderizado inicializado exitosamente (" << shadersDir << ")." << std::endl;
    return true;
}

bool SceneRenderer::init(Shader* litShader, Shader* boosterShader, Texture* paletteTexture) {
    if (!litShader || !boosterShader) {
        std::cerr << "[ERROR] [SceneRenderer] Se proporcionaron punteros nulos de shaders a init()" << std::endl;
        return false;
    }
    m_litShader = litShader;
    m_boosterShader = boosterShader;
    m_paletteTexture = paletteTexture;
    m_initialized = true;
    return true;
}

glm::vec4 SceneRenderer::calculateClearColor(world::TrackTheme theme, bool isNight) {
    switch (theme) {
        case world::TrackTheme::VOLCANO_CALDERA:
            return isNight ? glm::vec4(0.04f, 0.02f, 0.02f, 1.0f)
                           : glm::vec4(0.16f, 0.07f, 0.04f, 1.0f);
        case world::TrackTheme::COSMIC_SPACE:
            return glm::vec4(0.006f, 0.004f, 0.012f, 1.0f);
        case world::TrackTheme::AQUATIC_REEF:
            return isNight ? glm::vec4(0.01f, 0.04f, 0.07f, 1.0f)
                           : glm::vec4(0.03f, 0.20f, 0.30f, 1.0f);
        case world::TrackTheme::DESERT_DUNES:
            return isNight ? glm::vec4(0.02f, 0.03f, 0.06f, 1.0f)
                           : glm::vec4(0.45f, 0.72f, 0.96f, 1.0f);
        case world::TrackTheme::NIGHT_NEON:
            return glm::vec4(0.03f, 0.04f, 0.08f, 1.0f);
        case world::TrackTheme::DAY_GRASSLAND:
        default:
            return isNight ? glm::vec4(0.03f, 0.04f, 0.08f, 1.0f)
                           : glm::vec4(0.42f, 0.70f, 0.95f, 1.0f);
    }
}

void SceneRenderer::beginFrame(float screenWidth, float screenHeight, const glm::vec4& clearColor) {
    if (screenWidth > 0.0f && screenHeight > 0.0f) {
        glViewport(0, 0, static_cast<GLsizei>(screenWidth), static_cast<GLsizei>(screenHeight));
    }
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void SceneRenderer::beginFrame(world::TrackTheme theme, bool isNight, float screenWidth, float screenHeight) {
    beginFrame(screenWidth, screenHeight, calculateClearColor(theme, isNight));
}

void SceneRenderer::renderScene(const RenderSceneDesc& desc, const lighting::LightManager& lightManager) {
    renderScene(desc, &lightManager);
}

void SceneRenderer::renderScene(const RenderSceneDesc& desc, const lighting::LightManager* lightManager) {
    // -----------------------------------------------------------------
    // Pase 1: Geometría Opaca con m_litShader
    // -----------------------------------------------------------------
    if (m_litShader && m_litShader->isValid()) {
        m_litShader->use();

        if (lightManager) {
            lightManager->applyToShader(*m_litShader, desc.cameraPosition);
        }

        m_litShader->setMat4("u_View", desc.viewMatrix);
        m_litShader->setMat4("u_Projection", desc.projectionMatrix);
        m_litShader->setInt("u_Texture", 0);

        const Texture* activeTex = desc.paletteTexture ? desc.paletteTexture : m_paletteTexture;
        bool hasTexture = (activeTex && activeTex->isLoaded());
        m_litShader->setBool("u_UseTexture", hasTexture);
        if (hasTexture) {
            activeTex->bind(0);
        }

        // 1.1 Renderizar pista modular y plano de terreno
        if (desc.renderGround && desc.trackRenderer && desc.trackGrid) {
            desc.trackRenderer->render(*desc.trackGrid, *m_litShader);
        }

        // 1.2 Renderizar obstáculos y decoraciones
        if (desc.renderProps && desc.propSpawner) {
            m_litShader->setBool("u_UseTexture", true);
            desc.propSpawner->render(*m_litShader);
        }

        // 1.3 Renderizar pedestal (Showroom / expositor si está presente)
        if (desc.pedestalMesh) {
            m_litShader->setBool("u_UseTexture", false);
            m_litShader->setBool("u_UseVertexColor", false);
            m_litShader->setMat4("u_Model", desc.pedestalTransform);
            m_litShader->setVec4("u_Color", desc.pedestalColor);
            desc.pedestalMesh->draw();
        }

        // 1.4 Renderizar entidad jerárquica del kart
        if (desc.renderKart && desc.kartEntity) {
            m_litShader->setBool("u_UseTexture", true);
            m_litShader->setBool("u_UseVertexColor", false);
            m_litShader->setVec4("u_Color", glm::vec4(1.0f));
            desc.kartEntity->draw(*m_litShader);
        }
    }

    // -----------------------------------------------------------------
    // Pase 2: Boosters procedurales con m_boosterShader
    // -----------------------------------------------------------------
    if (desc.renderBoosters && desc.boosterPads && !desc.boosterPads->empty() &&
        m_boosterShader && m_boosterShader->isValid()) {
        m_boosterShader->use();
        m_boosterShader->setMat4("u_View", desc.viewMatrix);
        m_boosterShader->setMat4("u_Projection", desc.projectionMatrix);
        m_boosterShader->setVec3("u_BaseColor", glm::vec3(0.08f, 0.08f, 0.10f));
        m_boosterShader->setVec3("u_EnergyColor", glm::vec3(1.0f, 0.55f, 0.0f));
        m_boosterShader->setVec3("u_CoreColor", glm::vec3(1.0f, 1.0f, 0.85f));
        m_boosterShader->setFloat("u_EmissionIntensity", 3.0f);
        m_boosterShader->setFloat("u_Speed", 3.5f);
        m_boosterShader->setFloat("u_Repetitions", 3.0f);
        m_boosterShader->setFloat("u_ArrowThickness", 0.22f);
        m_boosterShader->setFloat("u_PulseSpeed", 5.0f);

        for (const auto& pad : *desc.boosterPads) {
            pad.draw(*m_boosterShader, desc.elapsedTime);
        }
    }

    // -----------------------------------------------------------------
    // Pase 3: Efectos translúcidos con blending (Skidmarks y Partículas)
    // -----------------------------------------------------------------
    if (desc.renderSkidmarks && desc.skidmarkManager) {
        desc.skidmarkManager->render(desc.viewMatrix, desc.projectionMatrix);
    }

    if (desc.renderParticles && desc.kartEmitter) {
        const_cast<vfx::KartParticleEmitter*>(desc.kartEmitter)->render(desc.viewMatrix, desc.projectionMatrix);
    }

    // -----------------------------------------------------------------
    // Pase 4: Skybox cúbico inmersivo (glDepthFunc LEQUAL)
    // -----------------------------------------------------------------
    if (desc.renderSkybox && desc.skybox && desc.skybox->isLoaded()) {
        glDepthFunc(GL_LEQUAL);
        desc.skybox->draw(desc.viewMatrix, desc.projectionMatrix);
        glDepthFunc(GL_LESS);
    }
}

void SceneRenderer::endFrame() {
    glUseProgram(0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace renderer
