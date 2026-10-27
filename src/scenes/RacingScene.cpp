#include "scenes/RacingScene.hpp"
#include "core/Input.hpp"
#include "core/Window.hpp"
#include "core/SceneManager.hpp"
#include "physics/CollisionSolver.hpp"
#include "vfx/KartMotionState.hpp"
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>

namespace scenes {

RacingScene::RacingScene(core::GameContext& ctx) : m_ctx(ctx) {}

void RacingScene::resetRace() {
    if (m_ctx.kartPhysics) {
        m_ctx.kartPhysics->setPosition(m_ctx.kartInitialPos);
        m_ctx.kartPhysics->setYaw(m_ctx.kartInitialYaw);
        m_ctx.kartPhysics->resetSpeed();
    }
    if (m_ctx.wheelAnimator) m_ctx.wheelAnimator->reset();
    if (m_ctx.chaseCamera && m_ctx.kartPhysics) m_ctx.chaseCamera->reset(m_ctx.kartInitialPos, m_ctx.kartPhysics->getForwardVector());
    m_lapsCompleted = 0;
    m_currentLapTime = m_totalRaceTime = m_lapNoticeTimer = 0.0f;
    if (m_ctx.skidmarkManager) m_ctx.skidmarkManager->clear();
}

void RacingScene::onEnter() {
    m_cursorLocked = m_isPaused = false;
    core::Input::setCursorVisible(true);
    m_ctx.cameraMode = core::CameraMode::CHASE;
    if (!m_uiRenderer.isInitialized()) m_uiRenderer.init();
    if (!m_pauseMenu.isInitialized()) m_pauseMenu.init(&m_uiRenderer);
    m_pauseMenu.close();
    m_pauseMenu.setSelectedOption(0);
    if (m_ctx.litShader && m_ctx.boosterShader) m_sceneRenderer.init(m_ctx.litShader, m_ctx.boosterShader, m_ctx.paletteTexture);
    resetRace();
    bool isAquatic = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex >= 0 &&
                      static_cast<size_t>(m_ctx.selectedTrackIndex) < m_ctx.tracks.size()) &&
                     (m_ctx.tracks[m_ctx.selectedTrackIndex].theme == world::TrackTheme::AQUATIC_REEF);
    if (m_ctx.kartEmitter) m_ctx.kartEmitter->setUnderwater(isAquatic);
}

void RacingScene::onExit() {
    m_cursorLocked = m_isPaused = false;
    m_pauseMenu.close();
    core::Input::setCursorVisible(true);
}

void RacingScene::handleInput(float deltaTime) {
    if (core::Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
        if (!m_isPaused && m_ctx.cameraMode == core::CameraMode::FPS && m_cursorLocked) {
            m_cursorLocked = false;
            core::Input::setCursorVisible(true);
        }
        setPaused(!m_isPaused);
        return;
    }

    if (m_isPaused) {
        if (!m_pauseMenu.isInitialized()) {
            if (!m_uiRenderer.isInitialized()) m_uiRenderer.init();
            m_pauseMenu.init(&m_uiRenderer);
        }
        if (!m_pauseMenu.isOpen()) m_pauseMenu.open();
        float sw = m_ctx.window ? static_cast<float>(m_ctx.window->getWidth()) : 1280.0f;
        float sh = m_ctx.window ? static_cast<float>(m_ctx.window->getHeight()) : 720.0f;
        m_pauseMenu.handleInput(deltaTime, m_ctx.lightingController, m_ctx.lightManager, sw, sh);

        switch (m_pauseMenu.consumeAction()) {
            case ui::PauseAction::RESUME: m_isPaused = false; break;
            case ui::PauseAction::RESTART: resetRace(); m_isPaused = false; break;
            case ui::PauseAction::GO_TO_MAIN_MENU:
                m_isPaused = false;
                if (m_ctx.sceneManager) m_ctx.sceneManager->changeScene(core::GameState::MAIN_MENU);
                break;
            case ui::PauseAction::QUIT_GAME:
                if (m_ctx.window) glfwSetWindowShouldClose(m_ctx.window->getNativeWindow(), GLFW_TRUE);
                else if (GLFWwindow* win = glfwGetCurrentContext()) glfwSetWindowShouldClose(win, GLFW_TRUE);
                break;
            default: break;
        }
        return;
    }

    if (core::Input::isKeyJustPressed(GLFW_KEY_M)) {
        if (m_ctx.sceneManager) m_ctx.sceneManager->changeScene(core::GameState::VEHICLE_SELECT);
        return;
    }
    if (core::Input::isKeyJustPressed(GLFW_KEY_C)) {
        if (m_ctx.cameraMode == core::CameraMode::CHASE) {
            m_ctx.cameraMode = core::CameraMode::FPS;
            m_cursorLocked = true;
            core::Input::setCursorVisible(false);
            if (m_ctx.fpsCamera && m_ctx.chaseCamera && m_ctx.kartPhysics) {
                m_ctx.fpsCamera->setPosition(m_ctx.chaseCamera->getPosition());
                m_ctx.fpsCamera->lookAtTarget(m_ctx.kartPhysics->getPosition() + glm::vec3(0.0f, 1.0f, 0.0f));
            }
        } else {
            m_ctx.cameraMode = core::CameraMode::CHASE;
            m_cursorLocked = false;
            core::Input::setCursorVisible(true);
            if (m_ctx.chaseCamera && m_ctx.kartPhysics) m_ctx.chaseCamera->reset(m_ctx.kartPhysics->getPosition(), m_ctx.kartPhysics->getForwardVector());
        }
    }
    if (core::Input::isKeyJustPressed(GLFW_KEY_R)) resetRace();
    if (m_ctx.lightingController) m_ctx.lightingController->handleInput();

    if (m_ctx.cameraMode == core::CameraMode::FPS && m_ctx.fpsCamera && m_ctx.collisionSystem) {
        if (core::Input::isKeyJustPressed(GLFW_KEY_TAB) || (!m_cursorLocked && core::Input::isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT))) {
            m_cursorLocked = !m_cursorLocked;
            core::Input::setCursorVisible(!m_cursorLocked);
        }
        if (m_cursorLocked) {
            auto& cam = *m_ctx.fpsCamera;
            const auto& col = *m_ctx.collisionSystem;
            if (core::Input::isKeyPressed(GLFW_KEY_W)) cam.processKeyboardWithCollision(camera::CameraMovement::FORWARD, deltaTime, col);
            if (core::Input::isKeyPressed(GLFW_KEY_S)) cam.processKeyboardWithCollision(camera::CameraMovement::BACKWARD, deltaTime, col);
            if (core::Input::isKeyPressed(GLFW_KEY_A)) cam.processKeyboardWithCollision(camera::CameraMovement::LEFT, deltaTime, col);
            if (core::Input::isKeyPressed(GLFW_KEY_D)) cam.processKeyboardWithCollision(camera::CameraMovement::RIGHT, deltaTime, col);
            if (core::Input::isKeyPressed(GLFW_KEY_SPACE) || core::Input::isKeyPressed(GLFW_KEY_E)) cam.processKeyboardWithCollision(camera::CameraMovement::UP, deltaTime, col);
            if (core::Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT) || core::Input::isKeyPressed(GLFW_KEY_Q)) cam.processKeyboardWithCollision(camera::CameraMovement::DOWN, deltaTime, col);
            float dx = core::Input::getMouseDeltaX(), dy = core::Input::getMouseDeltaY();
            if (dx != 0.0f || dy != 0.0f) cam.processMouseMovement(dx, dy);
            float scroll = core::Input::getScrollDeltaY();
            if (scroll != 0.0f) cam.processMouseScroll(scroll);
        }
    }
}

void RacingScene::update(float deltaTime) {
    if (m_isPaused) { m_pauseMenu.update(deltaTime); return; }
    m_elapsedTime += deltaTime;
    if (m_finishLineCooldown > 0.0f) m_finishLineCooldown -= deltaTime;
    if (!m_ctx.kartPhysics) return;

    bool isChase = (m_ctx.cameraMode == core::CameraMode::CHASE);
    bool accel = core::Input::isKeyPressed(GLFW_KEY_UP) || (isChase && core::Input::isKeyPressed(GLFW_KEY_W));
    bool brake = core::Input::isKeyPressed(GLFW_KEY_DOWN) || (isChase && core::Input::isKeyPressed(GLFW_KEY_S));
    bool drift = core::Input::isKeyPressed(GLFW_KEY_RIGHT_SHIFT) ||
                 (isChase && (core::Input::isKeyPressed(GLFW_KEY_SPACE) || core::Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT)));
    float steer = 0.0f;
    if (core::Input::isKeyPressed(GLFW_KEY_LEFT) || (isChase && core::Input::isKeyPressed(GLFW_KEY_A)))  steer -= 1.0f;
    if (core::Input::isKeyPressed(GLFW_KEY_RIGHT) || (isChase && core::Input::isKeyPressed(GLFW_KEY_D))) steer += 1.0f;

    glm::vec3 kartPos = m_ctx.kartPhysics->getPosition();
    if (m_ctx.trackGrid) {
        const auto* tile = m_ctx.trackGrid->getTileAtWorldPos(kartPos.x, kartPos.z);
        bool offRoadTile = (tile == nullptr || tile->type == world::TileType::EMPTY);
        float offRoad = (!m_ctx.vehicles.empty() && m_ctx.selectedVehicleIndex < static_cast<int>(m_ctx.vehicles.size()))
                            ? m_ctx.vehicles[m_ctx.selectedVehicleIndex].offRoadFriction : 0.28f;
        m_ctx.kartPhysics->setSurfaceFriction(offRoadTile ? offRoad : 1.0f);
    }

    m_currentLapTime += deltaTime;
    m_totalRaceTime += deltaTime;
    if (m_lapNoticeTimer > 0.0f) m_lapNoticeTimer -= deltaTime;

    m_ctx.kartPhysics->update(deltaTime, accel, brake, steer, drift);
    if (m_ctx.collisionSystem) {
        const auto hit = physics::CollisionSolver::solveSphereAgainstSystem(m_ctx.kartPhysics->getBoundingSphere(), *m_ctx.collisionSystem);
        if (hit.collided) m_ctx.kartPhysics->onCollision(hit.mtv, 0.25f);
    }
    if (m_ctx.boosterPads) {
        for (const auto& pad : *m_ctx.boosterPads) pad.onKartContact(*m_ctx.kartPhysics);
    }
    if (m_ctx.zoneManager) {
        const world::SpecialZone* hitZone = m_ctx.zoneManager->checkTrigger(m_ctx.kartPhysics->getPosition());
        if (hitZone && hitZone->type == world::ZoneType::FINISH_LINE && m_finishLineCooldown <= 0.0f &&
            m_ctx.kartPhysics->getSpeed() > 1.5f && m_currentLapTime > 3.0f) {
            m_lapsCompleted++;
            m_finishLineCooldown = 5.0f;
            m_lastLapTime = m_currentLapTime;
            m_isNewRecord = (m_bestLapTime <= 0.0f || m_currentLapTime < m_bestLapTime);
            if (m_isNewRecord) m_bestLapTime = m_currentLapTime;
            m_currentLapTime = 0.0f;
            m_lapNoticeTimer = 3.5f;
        }
    }

    if (m_ctx.wheelAnimator) m_ctx.wheelAnimator->update(deltaTime, m_ctx.kartPhysics->getSpeed(), steer);
    if (m_ctx.kartEntity && m_ctx.wheelAnimator) m_ctx.kartEntity->update(m_ctx.kartPhysics->getPosition(), m_ctx.kartPhysics->getYaw(), *m_ctx.wheelAnimator);
    if (m_ctx.kartEmitter) {
        vfx::KartMotionState motion{m_ctx.kartPhysics->getPosition(), m_ctx.kartPhysics->getYaw(),
                                    m_ctx.kartPhysics->getSpeed(), accel, m_ctx.kartPhysics->isBoosting()};
        m_ctx.kartEmitter->update(deltaTime, motion);
    }
    if (m_ctx.propSpawner) m_ctx.propSpawner->update(deltaTime);

    if (m_ctx.skidmarkManager && m_ctx.kartEntity) {
        bool isDriftSkid = m_ctx.kartPhysics->isDrifting() || (brake && m_ctx.kartPhysics->getSpeed() > 1.5f);
        glm::vec3 leftTire = m_ctx.kartEntity->getWheelBLWorldPos();
        glm::vec3 rightTire = m_ctx.kartEntity->getWheelBRWorldPos();
        auto surfaceY = [&](const glm::vec3& pos) {
            const auto* tile = m_ctx.trackGrid ? m_ctx.trackGrid->getTileAtWorldPos(pos.x, pos.z) : nullptr;
            return (!m_ctx.trackGrid || (tile && tile->type != world::TileType::EMPTY)) ? 0.012f : -0.042f;
        };
        leftTire.y = surfaceY(leftTire);
        rightTire.y = surfaceY(rightTire);
        m_ctx.skidmarkManager->updateWithWheelPositions(deltaTime, leftTire, rightTire,
            m_ctx.kartPhysics->getSpeed(), steer, isDriftSkid, m_ctx.kartPhysics->isBoosting());
    }

    if (m_ctx.lightingController) m_ctx.lightingController->update(m_ctx.kartPhysics->getPosition(), m_ctx.kartPhysics->getForwardVector(), deltaTime);
    if (m_ctx.chaseCamera) m_ctx.chaseCamera->update(deltaTime, m_ctx.kartPhysics->getPosition(), m_ctx.kartPhysics->getForwardVector());
    if (m_ctx.cameraMode == core::CameraMode::FPS && m_ctx.fpsCamera && m_ctx.collisionSystem) m_ctx.fpsCamera->resolveCollision(*m_ctx.collisionSystem);
}

void RacingScene::render(int windowWidth, int windowHeight) {
    if (!m_sceneRenderer.isInitialized() && m_ctx.litShader && m_ctx.boosterShader) {
        m_sceneRenderer.init(m_ctx.litShader, m_ctx.boosterShader, m_ctx.paletteTexture);
    }
    const float w = static_cast<float>(windowWidth);
    const float h = static_cast<float>(windowHeight > 0 ? windowHeight : 1);
    const bool isNight = m_ctx.lightingController ? m_ctx.lightingController->isNight() : false;
    const world::TrackTheme theme = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex >= 0 &&
                                     static_cast<size_t>(m_ctx.selectedTrackIndex) < m_ctx.tracks.size())
                                     ? m_ctx.tracks[m_ctx.selectedTrackIndex].theme : world::TrackTheme::DAY_GRASSLAND;

    m_sceneRenderer.beginFrame(w, h, renderer::SceneRenderer::calculateClearColor(theme, isNight));
    const auto& activeCam = m_ctx.getActiveCamera();
    renderer::RenderSceneDesc desc{
        activeCam.getViewMatrix(), activeCam.getProjectionMatrix(w / h), activeCam.getPosition(), m_elapsedTime,
        m_ctx.trackGrid, m_ctx.trackRenderer, m_ctx.propSpawner, m_ctx.kartEntity, m_ctx.boosterPads,
        m_ctx.kartEmitter, m_ctx.skidmarkManager, &m_ctx.getCurrentSkybox(), m_ctx.paletteTexture,
        nullptr, glm::mat4(1.0f), glm::vec4(0.20f, 0.22f, 0.28f, 1.0f), isNight
    };
    if (m_ctx.lightManager) m_sceneRenderer.renderScene(desc, *m_ctx.lightManager);
    else m_sceneRenderer.renderScene(desc);
    m_sceneRenderer.endFrame();

    if (m_ctx.racingHud && m_ctx.kartPhysics) {
        ui::TelemetryData telem{
            std::abs(m_ctx.kartPhysics->getSpeed()) * 3.6f,
            (!m_ctx.vehicles.empty() && m_ctx.selectedVehicleIndex < static_cast<int>(m_ctx.vehicles.size()))
                ? m_ctx.vehicles[m_ctx.selectedVehicleIndex].maxSpeed * 3.6f : 80.0f,
            m_ctx.kartPhysics->isBoosting(), false, (m_ctx.kartPhysics->getSpeed() < -0.1f),
            m_ctx.skidmarkManager ? m_ctx.skidmarkManager->isCurrentlySkidding() : false,
            (m_ctx.kartPhysics->getSurfaceFriction() < 0.9f)
        };
        ui::LapTimingData timing{
            std::min(m_lapsCompleted + 1, m_totalLaps), m_totalLaps, m_currentLapTime, m_bestLapTime,
            m_totalRaceTime, m_lapNoticeTimer, m_lastLapTime, m_isNewRecord
        };
        m_ctx.racingHud->render(w, h, telem, timing);
    }

    if (m_isPaused) {
        if (!m_pauseMenu.isInitialized()) {
            if (!m_uiRenderer.isInitialized()) m_uiRenderer.init();
            m_pauseMenu.init(&m_uiRenderer);
        }
        if (!m_pauseMenu.isOpen()) m_pauseMenu.open();
        m_pauseMenu.render(w, h, m_ctx.lightingController, m_ctx.lightManager);
    }
}

} // namespace scenes
