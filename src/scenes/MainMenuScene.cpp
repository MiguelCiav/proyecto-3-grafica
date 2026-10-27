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
    m_ctx.getAudio().setMenuMusicActive(true, 0.85f);
    core::Input::setCursorVisible(true);

    m_sceneRenderer.init(m_ctx.litShader, m_ctx.boosterShader, m_ctx.paletteTexture);

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
    world::TrackTheme theme = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex >= 0 &&
                               static_cast<size_t>(m_ctx.selectedTrackIndex) < m_ctx.tracks.size())
                                  ? m_ctx.tracks[m_ctx.selectedTrackIndex].theme
                                  : world::TrackTheme::DAY_GRASSLAND;
    bool isNight = m_ctx.lightingController ? m_ctx.lightingController->isNight() : false;
    glm::vec4 clearColor = renderer::SceneRenderer::calculateClearColor(theme, isNight);

    m_sceneRenderer.beginFrame(static_cast<float>(windowWidth), static_cast<float>(windowHeight), clearColor);

    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight > 0 ? windowHeight : 1);
    float rad = glm::radians(m_menuOrbitAngle);
    float cols = m_ctx.trackGrid ? static_cast<float>(m_ctx.trackGrid->getCols()) : 12.0f;
    float orbitDist = (cols * world::TrackGrid::TILE_SIZE) * 0.65f;
    glm::vec3 camPos(std::sin(rad) * orbitDist, 15.0f, std::cos(rad) * orbitDist);
    glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(55.0f), aspect, 0.1f, 1000.0f);

    renderer::RenderSceneDesc desc;
    desc.viewMatrix = view;
    desc.projectionMatrix = proj;
    desc.cameraPosition = camPos;
    desc.elapsedTime = m_elapsedTime;
    desc.trackGrid = m_ctx.trackGrid;
    desc.trackRenderer = m_ctx.trackRenderer;
    desc.propSpawner = m_ctx.propSpawner;
    desc.kartEntity = m_ctx.kartEntity;
    desc.boosterPads = m_ctx.boosterPads;
    desc.kartEmitter = m_ctx.kartEmitter;
    desc.skybox = &m_ctx.getCurrentSkybox();
    desc.paletteTexture = m_ctx.paletteTexture;
    desc.isNight = isNight;
    desc.renderSkidmarks = false;

    if (m_ctx.lightManager) {
        m_sceneRenderer.renderScene(desc, *m_ctx.lightManager);
    } else {
        m_sceneRenderer.renderScene(desc, nullptr);
    }
    m_sceneRenderer.endFrame();

    // -------------------------------------------------------------
    // Capa de Interfaz Grafica 2D (Kenney UI Pack)
    // -------------------------------------------------------------
    m_menuUI.render(static_cast<float>(windowWidth), static_cast<float>(windowHeight));
}

} // namespace scenes
