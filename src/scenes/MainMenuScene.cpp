#include "scenes/MainMenuScene.hpp"
#include "core/Input.hpp"
#include "core/Window.hpp"
#include "core/SceneManager.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace scenes {

MainMenuScene::MainMenuScene(core::GameContext& ctx)
    : m_ctx(ctx) {
}

void MainMenuScene::onEnter() {
    core::Input::setCursorVisible(true);

    if (!m_uiRenderer.isInitialized()) {
        m_uiRenderer.init();
    }
    m_menuUI.init(&m_uiRenderer, &m_ctx);
}

void MainMenuScene::onExit() {
}

void MainMenuScene::handleInput(float deltaTime) {
    m_menuUI.handleInput(deltaTime);
}

void MainMenuScene::update(float deltaTime) {
    m_menuOrbitAngle += deltaTime * 8.0f;
    m_elapsedTime += deltaTime;

    if (m_ctx.propSpawner) {
        m_ctx.propSpawner->update(deltaTime);
    }

    m_menuUI.update(deltaTime);

    m_titleTimer += deltaTime;
    if (m_titleTimer >= 0.25f && m_ctx.window) {
        m_titleTimer = 0.0f;
        m_ctx.window->setTitle("Prototipo Mario Kart - CGI (UCV) | MENU PRINCIPAL | [W/S/FLECHAS] Navegar | [ENTER] Seleccionar");
    }
}

void MainMenuScene::render(int windowWidth, int windowHeight) {
    bool isNight = m_ctx.lightingController ? m_ctx.lightingController->isNight() : false;
    bool isSpace = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex >= 0 &&
                    static_cast<size_t>(m_ctx.selectedTrackIndex) < m_ctx.tracks.size())
                    ? (m_ctx.tracks[m_ctx.selectedTrackIndex].theme == world::TrackTheme::COSMIC_SPACE) : false;
    bool isDesert = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex >= 0 &&
                     static_cast<size_t>(m_ctx.selectedTrackIndex) < m_ctx.tracks.size())
                     ? (m_ctx.tracks[m_ctx.selectedTrackIndex].theme == world::TrackTheme::DESERT_DUNES) : false;
    bool isAquatic = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex >= 0 &&
                      static_cast<size_t>(m_ctx.selectedTrackIndex) < m_ctx.tracks.size())
                      ? (m_ctx.tracks[m_ctx.selectedTrackIndex].theme == world::TrackTheme::AQUATIC_REEF) : false;
    bool isVolcano = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex >= 0 &&
                      static_cast<size_t>(m_ctx.selectedTrackIndex) < m_ctx.tracks.size())
                      ? (m_ctx.tracks[m_ctx.selectedTrackIndex].theme == world::TrackTheme::VOLCANO_CALDERA) : false;

    if (isVolcano) {
        if (isNight) glClearColor(0.04f, 0.02f, 0.02f, 1.0f);
        else         glClearColor(0.16f, 0.07f, 0.04f, 1.0f);
    } else if (isSpace) {
        glClearColor(0.006f, 0.004f, 0.012f, 1.0f);
    } else if (isAquatic) {
        if (isNight) glClearColor(0.01f, 0.04f, 0.07f, 1.0f);
        else         glClearColor(0.03f, 0.20f, 0.30f, 1.0f);
    } else if (isDesert) {
        if (isNight) glClearColor(0.02f, 0.03f, 0.06f, 1.0f);
        else         glClearColor(0.50f, 0.72f, 0.95f, 1.0f);
    } else {
        if (isNight) glClearColor(0.04f, 0.05f, 0.09f, 1.0f);
        else         glClearColor(0.40f, 0.68f, 0.95f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderer::Shader& shader = (m_ctx.litShader && m_ctx.litShader->isValid()) ? *m_ctx.litShader : *m_ctx.litShader;
    if (!shader.isValid()) return;

    shader.use();

    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight > 0 ? windowHeight : 1);
    float rad = glm::radians(m_menuOrbitAngle);
    float cols = m_ctx.trackGrid ? static_cast<float>(m_ctx.trackGrid->getCols()) : 12.0f;
    float orbitDist = (cols * world::TrackGrid::TILE_SIZE) * 0.65f;
    glm::vec3 camPos(std::sin(rad) * orbitDist, 15.0f, std::cos(rad) * orbitDist);
    glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(55.0f), aspect, 0.1f, 1000.0f);

    if (m_ctx.lightManager) {
        m_ctx.lightManager->applyToShader(shader, camPos);
    }

    shader.setMat4("u_View", view);
    shader.setMat4("u_Projection", proj);
    shader.setInt("u_Texture", 0);

    if (m_ctx.paletteTexture && m_ctx.paletteTexture->isLoaded()) {
        m_ctx.paletteTexture->bind(0);
    }

    if (m_ctx.trackRenderer && m_ctx.trackGrid) {
        m_ctx.trackRenderer->render(*m_ctx.trackGrid, shader);
    }
    if (m_ctx.propSpawner) {
        m_ctx.propSpawner->render(shader);
    }

    if (m_ctx.kartEntity) {
        shader.setBool("u_UseTexture", true);
        shader.setVec4("u_Color", glm::vec4(1.0f));
        m_ctx.kartEntity->draw(shader);
    }

    if (m_ctx.boosterShader && m_ctx.boosterShader->isValid() && m_ctx.boosterPads) {
        m_ctx.boosterShader->use();
        m_ctx.boosterShader->setMat4("u_View", view);
        m_ctx.boosterShader->setMat4("u_Projection", proj);
        m_ctx.boosterShader->setVec3("u_BaseColor", glm::vec3(0.08f, 0.08f, 0.10f));
        m_ctx.boosterShader->setVec3("u_EnergyColor", glm::vec3(1.0f, 0.55f, 0.0f));
        m_ctx.boosterShader->setVec3("u_CoreColor", glm::vec3(1.0f, 1.0f, 0.85f));
        m_ctx.boosterShader->setFloat("u_EmissionIntensity", 3.0f);
        m_ctx.boosterShader->setFloat("u_Speed", 3.5f);
        m_ctx.boosterShader->setFloat("u_Repetitions", 3.0f);
        m_ctx.boosterShader->setFloat("u_ArrowThickness", 0.22f);
        m_ctx.boosterShader->setFloat("u_PulseSpeed", 5.0f);

        for (const auto& pad : *m_ctx.boosterPads) {
            pad.draw(*m_ctx.boosterShader, m_elapsedTime);
        }
    }

    if (m_ctx.kartEmitter) {
        m_ctx.kartEmitter->render(view, proj);
    }

    const auto& skybox = m_ctx.getCurrentSkybox();
    if (skybox.isLoaded()) {
        skybox.draw(view, proj);
    }

    // -------------------------------------------------------------
    // Capa de Interfaz Grafica 2D (Kenney UI Pack)
    // -------------------------------------------------------------
    m_menuUI.render(static_cast<float>(windowWidth), static_cast<float>(windowHeight));
}

} // namespace scenes
