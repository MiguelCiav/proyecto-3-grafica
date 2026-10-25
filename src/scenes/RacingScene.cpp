#include "scenes/RacingScene.hpp"
#include "core/Input.hpp"
#include "core/Window.hpp"
#include "core/SceneManager.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace scenes {

RacingScene::RacingScene(core::GameContext& ctx)
    : m_ctx(ctx) {
}

void RacingScene::onEnter() {
    m_cursorLocked = false;
    core::Input::setCursorVisible(true);
    m_ctx.cameraMode = core::CameraMode::CHASE;

    if (m_ctx.kartPhysics) {
        m_ctx.kartPhysics->setPosition(m_ctx.kartInitialPos);
        m_ctx.kartPhysics->setYaw(m_ctx.kartInitialYaw);
        m_ctx.kartPhysics->resetSpeed();
    }
    if (m_ctx.wheelAnimator) {
        m_ctx.wheelAnimator->reset();
    }
    if (m_ctx.chaseCamera && m_ctx.kartPhysics) {
        m_ctx.chaseCamera->reset(m_ctx.kartInitialPos, m_ctx.kartPhysics->getForwardVector());
    }

    bool isAquatic = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex >= 0 &&
                      static_cast<size_t>(m_ctx.selectedTrackIndex) < m_ctx.tracks.size())
                      ? (m_ctx.tracks[m_ctx.selectedTrackIndex].theme == world::TrackTheme::AQUATIC_REEF) : false;
    if (m_ctx.kartEmitter) {
        m_ctx.kartEmitter->setUnderwater(isAquatic);
    }

    std::string trackName = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex < static_cast<int>(m_ctx.tracks.size()))
                                ? m_ctx.tracks[m_ctx.selectedTrackIndex].name : "Circuito";
    std::cout << "\n========================================================" << std::endl;
    std::cout << "  >>> [CARRERA INICIADA] Pista: " << trackName << " <<<" << std::endl;
    std::cout << "  [W/A/S/D] Conducir | [C] Camara FPS / Chase | [R] Reiniciar" << std::endl;
    std::cout << "  [1/2/3] Modelos Sombreado | [L] Faros Delanteros" << std::endl;
    std::cout << "  [P] Farolas de Pista ON/OFF | [N] Modo Noche / Dia" << std::endl;
    std::cout << "  [M] Showroom / Pistas | [ESC] Menu" << std::endl;
    std::cout << "========================================================\n" << std::endl;
}

void RacingScene::onExit() {
    m_cursorLocked = false;
    core::Input::setCursorVisible(true);
}

void RacingScene::handleInput(float deltaTime) {
    // Tecla M: Volver al Showroom
    if (core::Input::isKeyJustPressed(GLFW_KEY_M)) {
        if (m_ctx.sceneManager) {
            m_ctx.sceneManager->changeScene(core::GameState::VEHICLE_SELECT);
        }
        return;
    }

    // ESC: Liberar cursor en modo FPS o volver al menu principal
    if (core::Input::isKeyJustPressed(GLFW_KEY_ESCAPE)) {
        if (m_ctx.cameraMode == core::CameraMode::FPS && m_cursorLocked) {
            m_cursorLocked = false;
            core::Input::setCursorVisible(true);
        } else {
            if (m_ctx.sceneManager) {
                m_ctx.sceneManager->changeScene(core::GameState::MAIN_MENU);
            }
            return;
        }
    }

    // Conmutador de camara (Tecla C)
    if (core::Input::isKeyJustPressed(GLFW_KEY_C)) {
        if (m_ctx.cameraMode == core::CameraMode::CHASE) {
            m_ctx.cameraMode = core::CameraMode::FPS;
            m_cursorLocked = true;
            core::Input::setCursorVisible(false);
            if (m_ctx.fpsCamera && m_ctx.chaseCamera && m_ctx.kartPhysics) {
                m_ctx.fpsCamera->setPosition(m_ctx.chaseCamera->getPosition());
                m_ctx.fpsCamera->lookAtTarget(m_ctx.kartPhysics->getPosition() + glm::vec3(0.0f, 1.0f, 0.0f));
            }
            std::cout << "[Camara] Modo: INSPECCION LIBRE FPS (Restriccion de Colision ON)" << std::endl;
        } else {
            m_ctx.cameraMode = core::CameraMode::CHASE;
            m_cursorLocked = false;
            core::Input::setCursorVisible(true);
            if (m_ctx.chaseCamera && m_ctx.kartPhysics) {
                m_ctx.chaseCamera->reset(m_ctx.kartPhysics->getPosition(), m_ctx.kartPhysics->getForwardVector());
            }
            std::cout << "[Camara] Modo: CONDUCCION TERCERA PERSONA (Chase Cam)" << std::endl;
        }
    }

    // Reinicio de kart (Tecla R)
    if (core::Input::isKeyJustPressed(GLFW_KEY_R)) {
        if (m_ctx.kartPhysics) {
            m_ctx.kartPhysics->setPosition(m_ctx.kartInitialPos);
            m_ctx.kartPhysics->setYaw(m_ctx.kartInitialYaw);
            m_ctx.kartPhysics->resetSpeed();
        }
        if (m_ctx.wheelAnimator) {
            m_ctx.wheelAnimator->reset();
        }
        if (m_ctx.chaseCamera && m_ctx.kartPhysics) {
            m_ctx.chaseCamera->reset(m_ctx.kartInitialPos, m_ctx.kartPhysics->getForwardVector());
        }
        m_lapsCompleted = 0;
        m_currentLapTime = 0.0f;
        m_totalRaceTime = 0.0f;
        m_lapNoticeTimer = 0.0f;
        if (m_ctx.skidmarkManager) {
            m_ctx.skidmarkManager->clear();
        }
        std::string trackName = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex < static_cast<int>(m_ctx.tracks.size()))
                                    ? m_ctx.tracks[m_ctx.selectedTrackIndex].name : "Circuito";
        std::cout << "[Kart] Posicion y cronometros reiniciados a la linea de salida de " << trackName << "." << std::endl;
    }

    // Controlador de Iluminacion y Faros
    if (m_ctx.lightingController) {
        m_ctx.lightingController->handleInput();
    }

    // En modo FPS: alternar captura de mouse con clic o TAB
    if (m_ctx.cameraMode == core::CameraMode::FPS && m_ctx.fpsCamera && m_ctx.collisionSystem) {
        if (core::Input::isKeyJustPressed(GLFW_KEY_TAB) || (!m_cursorLocked && core::Input::isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT))) {
            m_cursorLocked = !m_cursorLocked;
            core::Input::setCursorVisible(!m_cursorLocked);
        }

        if (m_cursorLocked) {
            if (core::Input::isKeyPressed(GLFW_KEY_W)) m_ctx.fpsCamera->processKeyboardWithCollision(camera::CameraMovement::FORWARD, deltaTime, *m_ctx.collisionSystem);
            if (core::Input::isKeyPressed(GLFW_KEY_S)) m_ctx.fpsCamera->processKeyboardWithCollision(camera::CameraMovement::BACKWARD, deltaTime, *m_ctx.collisionSystem);
            if (core::Input::isKeyPressed(GLFW_KEY_A)) m_ctx.fpsCamera->processKeyboardWithCollision(camera::CameraMovement::LEFT, deltaTime, *m_ctx.collisionSystem);
            if (core::Input::isKeyPressed(GLFW_KEY_D)) m_ctx.fpsCamera->processKeyboardWithCollision(camera::CameraMovement::RIGHT, deltaTime, *m_ctx.collisionSystem);
            if (core::Input::isKeyPressed(GLFW_KEY_SPACE) || core::Input::isKeyPressed(GLFW_KEY_E))
                m_ctx.fpsCamera->processKeyboardWithCollision(camera::CameraMovement::UP, deltaTime, *m_ctx.collisionSystem);
            if (core::Input::isKeyPressed(GLFW_KEY_LEFT_SHIFT) || core::Input::isKeyPressed(GLFW_KEY_Q))
                m_ctx.fpsCamera->processKeyboardWithCollision(camera::CameraMovement::DOWN, deltaTime, *m_ctx.collisionSystem);

            float dx = core::Input::getMouseDeltaX();
            float dy = core::Input::getMouseDeltaY();
            if (dx != 0.0f || dy != 0.0f) {
                m_ctx.fpsCamera->processMouseMovement(dx, dy);
            }

            float scroll = core::Input::getScrollDeltaY();
            if (scroll != 0.0f) {
                m_ctx.fpsCamera->processMouseScroll(scroll);
            }
        }
    }
}

void RacingScene::update(float deltaTime) {
    m_elapsedTime += deltaTime;
    if (m_finishLineCooldown > 0.0f) {
        m_finishLineCooldown -= deltaTime;
    }

    // 1. Entrada de conduccion del kart
    bool accel = false;
    bool brake = false;
    float steer = 0.0f;

    if (m_ctx.cameraMode == core::CameraMode::CHASE) {
        accel = core::Input::isKeyPressed(GLFW_KEY_W) || core::Input::isKeyPressed(GLFW_KEY_UP);
        brake = core::Input::isKeyPressed(GLFW_KEY_S) || core::Input::isKeyPressed(GLFW_KEY_DOWN);
        if (core::Input::isKeyPressed(GLFW_KEY_A) || core::Input::isKeyPressed(GLFW_KEY_LEFT))  steer -= 1.0f;
        if (core::Input::isKeyPressed(GLFW_KEY_D) || core::Input::isKeyPressed(GLFW_KEY_RIGHT)) steer += 1.0f;
    } else {
        accel = core::Input::isKeyPressed(GLFW_KEY_UP);
        brake = core::Input::isKeyPressed(GLFW_KEY_DOWN);
        if (core::Input::isKeyPressed(GLFW_KEY_LEFT))  steer -= 1.0f;
        if (core::Input::isKeyPressed(GLFW_KEY_RIGHT)) steer += 1.0f;
    }

    if (!m_ctx.kartPhysics) return;

    // 2. Evaluacion de superficie (Cesped vs Asfalto)
    glm::vec3 kartPos = m_ctx.kartPhysics->getPosition();
    if (m_ctx.trackGrid) {
        const auto* currentTile = m_ctx.trackGrid->getTileAtWorldPos(kartPos.x, kartPos.z);
        if (currentTile == nullptr || currentTile->type == world::TileType::EMPTY) {
            float offRoad = (!m_ctx.vehicles.empty() && m_ctx.selectedVehicleIndex < static_cast<int>(m_ctx.vehicles.size()))
                                ? m_ctx.vehicles[m_ctx.selectedVehicleIndex].offRoadFriction : 0.45f;
            m_ctx.kartPhysics->setSurfaceFriction(offRoad);
        } else {
            m_ctx.kartPhysics->setSurfaceFriction(1.0f);
        }
    }

    // 0. Actualizar cronometros de carrera y notificacion de vuelta
    m_currentLapTime += deltaTime;
    m_totalRaceTime += deltaTime;
    if (m_lapNoticeTimer > 0.0f) {
        m_lapNoticeTimer -= deltaTime;
    }

    // 3. Actualizar modelo de dinamica de vehiculo
    m_ctx.kartPhysics->update(deltaTime, accel, brake, steer);

    // 4. Resolucion de colisiones del kart contra barreras/conos con esfera suave
    if (m_ctx.collisionSystem) {
        glm::vec3 mtv(0.0f);
        if (m_ctx.collisionSystem->checkCollision(m_ctx.kartPhysics->getBoundingSphere(), mtv)) {
            m_ctx.kartPhysics->onCollision(mtv, 0.25f);
        }
    }

    // 5. Evaluacion de zonas de boost y meta
    if (m_ctx.boosterPads) {
        for (const auto& pad : *m_ctx.boosterPads) {
            pad.onKartContact(*m_ctx.kartPhysics);
        }
    }

    if (m_ctx.zoneManager) {
        const world::SpecialZone* hitZone = m_ctx.zoneManager->checkTrigger(m_ctx.kartPhysics->getPosition());
        if (hitZone != nullptr) {
            if (hitZone->type == world::ZoneType::FINISH_LINE && m_finishLineCooldown <= 0.0f && m_ctx.kartPhysics->getSpeed() > 1.5f) {
                if (m_currentLapTime > 3.0f) {
                    m_lapsCompleted++;
                    m_finishLineCooldown = 5.0f;
                    m_lastLapTime = m_currentLapTime;
                    if (m_bestLapTime <= 0.0f || m_currentLapTime < m_bestLapTime) {
                        m_bestLapTime = m_currentLapTime;
                        m_isNewRecord = true;
                    } else {
                        m_isNewRecord = false;
                    }
                    m_currentLapTime = 0.0f;
                    m_lapNoticeTimer = 3.5f;

                    std::string trackName = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex < static_cast<int>(m_ctx.tracks.size()))
                                                ? m_ctx.tracks[m_ctx.selectedTrackIndex].name : "Circuito";
                    std::cout << ">>> [META!] Vuelta " << m_lapsCompleted << " completada en "
                              << trackName << "! Tiempo: " << m_lastLapTime << " s <<<" << std::endl;
                }
            }
        }
    }

    // 6. Actualizar cinematica de ruedas y Grafo de Escena
    if (m_ctx.wheelAnimator) {
        m_ctx.wheelAnimator->update(deltaTime, m_ctx.kartPhysics->getSpeed(), steer);
    }
    if (m_ctx.kartEntity && m_ctx.wheelAnimator) {
        m_ctx.kartEntity->update(m_ctx.kartPhysics->getPosition(), m_ctx.kartPhysics->getYaw(), *m_ctx.wheelAnimator);
    }

    if (m_ctx.kartEmitter) {
        m_ctx.kartEmitter->update(deltaTime, *m_ctx.kartPhysics, accel);
    }

    if (m_ctx.propSpawner) {
        m_ctx.propSpawner->update(deltaTime);
    }

    // Actualizar marcas de derrape en el asfalto y terreno
    if (m_ctx.skidmarkManager && m_ctx.kartEntity) {
        bool isBraking = brake && (m_ctx.kartPhysics->getSpeed() > 1.5f);
        glm::vec3 leftTireContact = m_ctx.kartEntity->getWheelBLWorldPos();
        glm::vec3 rightTireContact = m_ctx.kartEntity->getWheelBRWorldPos();

        // Adaptar la cota vertical según la superficie: el asfalto está en Y=0.0f (+0.012f elevacion),
        // mientras que el plano de terreno (cesped, arena, magma) se encuentra en Y=-0.05f (-0.042f elevacion).
        float leftY = 0.012f;
        float rightY = 0.012f;
        if (m_ctx.trackGrid) {
            const auto* leftTile = m_ctx.trackGrid->getTileAtWorldPos(leftTireContact.x, leftTireContact.z);
            if (leftTile == nullptr || leftTile->type == world::TileType::EMPTY) {
                leftY = -0.042f;
            }
            const auto* rightTile = m_ctx.trackGrid->getTileAtWorldPos(rightTireContact.x, rightTireContact.z);
            if (rightTile == nullptr || rightTile->type == world::TileType::EMPTY) {
                rightY = -0.042f;
            }
        }
        leftTireContact.y = leftY;
        rightTireContact.y = rightY;

        m_ctx.skidmarkManager->updateWithWheelPositions(deltaTime, leftTireContact, rightTireContact,
                                                       m_ctx.kartPhysics->getSpeed(), steer, isBraking, m_ctx.kartPhysics->isBoosting());
    }

    if (m_ctx.lightingController) {
        m_ctx.lightingController->update(m_ctx.kartPhysics->getPosition(), m_ctx.kartPhysics->getForwardVector(), deltaTime);
    }

    // 7. Actualizar camaras
    if (m_ctx.chaseCamera) {
        m_ctx.chaseCamera->update(deltaTime, m_ctx.kartPhysics->getPosition(), m_ctx.kartPhysics->getForwardVector());
    }
    if (m_ctx.cameraMode == core::CameraMode::FPS && m_ctx.fpsCamera && m_ctx.collisionSystem) {
        m_ctx.fpsCamera->resolveCollision(*m_ctx.collisionSystem);
    }

    // 8. Actualizar telemetría en el título de la ventana
    m_titleTimer += deltaTime;
    if (m_titleTimer >= 0.20f && m_ctx.window && m_ctx.kartPhysics) {
        m_titleTimer = 0.0f;
        std::stringstream ss;
        ss << "Prototipo Mario Kart - CGI (UCV) | ";
        if (m_ctx.cameraMode == core::CameraMode::CHASE) {
            std::string vName = (!m_ctx.vehicles.empty() && m_ctx.selectedVehicleIndex < static_cast<int>(m_ctx.vehicles.size()))
                                    ? m_ctx.vehicles[m_ctx.selectedVehicleIndex].name : "Kart";
            std::string tName = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex < static_cast<int>(m_ctx.tracks.size()))
                                    ? m_ctx.tracks[m_ctx.selectedTrackIndex].name : "Pista";
            ss << "[" << vName << " @ " << tName << "] ";
            ss << "| Vel: " << std::fixed << std::setprecision(1) << (m_ctx.kartPhysics->getSpeed() * 3.6f) << " km/h";
            if (m_ctx.kartPhysics->isBoosting()) ss << " [TURBO!]";
            if (m_ctx.kartPhysics->getSurfaceFriction() < 0.9f) ss << " [OFF-ROAD]";
            ss << " | Vueltas: " << m_lapsCompleted;
        } else if (m_ctx.fpsCamera) {
            ss << "[Modo Inspeccion FPS (Colisiones ON)] | Pos: ("
               << std::setprecision(1) << m_ctx.fpsCamera->getPosition().x << ", "
               << m_ctx.fpsCamera->getPosition().y << ", "
               << m_ctx.fpsCamera->getPosition().z << ")";
        }
        if (m_ctx.lightingController) {
            ss << " | [" << m_ctx.lightingController->getLightingModelName()
               << " | Faros: " << (m_ctx.lightingController->areHeadlightsOn() ? "ON" : "OFF") << "]";
        }
        ss << " | [1/2/3: Luz] [L: Faros] [M: Menu] [C: Camara] [R: Reset] [ESC: Salir]";
        m_ctx.window->setTitle(ss.str());
    }
}

void RacingScene::render(int windowWidth, int windowHeight) {
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
        else         glClearColor(0.45f, 0.72f, 0.96f, 1.0f);
    } else {
        if (isNight) glClearColor(0.03f, 0.04f, 0.08f, 1.0f);
        else         glClearColor(0.42f, 0.70f, 0.95f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderer::Shader& shader = (m_ctx.litShader && m_ctx.litShader->isValid()) ? *m_ctx.litShader : *m_ctx.litShader;
    if (!shader.isValid()) return;

    shader.use();

    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight > 0 ? windowHeight : 1);
    const auto& activeCam = m_ctx.getActiveCamera();
    glm::mat4 view = activeCam.getViewMatrix();
    glm::mat4 proj = activeCam.getProjectionMatrix(aspect);
    glm::vec3 camPos = activeCam.getPosition();

    if (m_ctx.lightManager) {
        m_ctx.lightManager->applyToShader(shader, camPos);
    }

    shader.setMat4("u_View", view);
    shader.setMat4("u_Projection", proj);
    shader.setInt("u_Texture", 0);
    shader.setBool("u_UseTexture", m_ctx.paletteTexture && m_ctx.paletteTexture->isLoaded());

    if (m_ctx.paletteTexture && m_ctx.paletteTexture->isLoaded()) {
        m_ctx.paletteTexture->bind(0);
    }

    // 1. Renderizar pista modular
    if (m_ctx.trackRenderer && m_ctx.trackGrid) {
        m_ctx.trackRenderer->render(*m_ctx.trackGrid, shader);
    }

    // 2. Renderizar props y obstaculos
    if (m_ctx.propSpawner) {
        shader.setBool("u_UseTexture", true);
        m_ctx.propSpawner->render(shader);
    }

    // 3. Renderizar entidad jerarquica del kart
    if (m_ctx.kartEntity) {
        shader.setBool("u_UseTexture", true);
        shader.setBool("u_UseVertexColor", false);
        shader.setVec4("u_Color", glm::vec4(1.0f));
        m_ctx.kartEntity->draw(shader);
    }

    // 4. Renderizar pads de aceleracion procedural
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

    // 5. Renderizar marcas de derrape en el asfalto
    if (m_ctx.skidmarkManager) {
        m_ctx.skidmarkManager->render(view, proj);
    }

    // 6. Renderizar efectos visuales de particulas del kart
    if (m_ctx.kartEmitter) {
        m_ctx.kartEmitter->render(view, proj);
    }

    // 7. Renderizar Skybox 3D inmersivo
    const auto& skybox = m_ctx.getCurrentSkybox();
    if (skybox.isLoaded()) {
        skybox.draw(view, proj);
    }

    // 8. Renderizar interfaces de usuario HUD
    if (m_ctx.racingHud && m_ctx.kartPhysics) {
        ui::TelemetryData telem;
        telem.speedKmH = std::abs(m_ctx.kartPhysics->getSpeed()) * 3.6f;
        telem.maxSpeedKmH = (!m_ctx.vehicles.empty() && m_ctx.selectedVehicleIndex < static_cast<int>(m_ctx.vehicles.size()))
                                ? m_ctx.vehicles[m_ctx.selectedVehicleIndex].maxSpeed * 3.6f : 80.0f;
        telem.isBoosting = m_ctx.kartPhysics->isBoosting();
        telem.isBraking = false;
        telem.isReversing = (m_ctx.kartPhysics->getSpeed() < -0.1f);
        telem.isSkidding = m_ctx.skidmarkManager ? m_ctx.skidmarkManager->isCurrentlySkidding() : false;
        telem.isOffRoad = (m_ctx.kartPhysics->getSurfaceFriction() < 0.9f);

        ui::LapTimingData timing;
        timing.currentLap = std::min(m_lapsCompleted + 1, m_totalLaps);
        timing.totalLaps = m_totalLaps;
        timing.currentLapTime = m_currentLapTime;
        timing.bestLapTime = m_bestLapTime;
        timing.totalRaceTime = m_totalRaceTime;
        timing.lastLapTime = m_lastLapTime;
        timing.lapNoticeTimer = m_lapNoticeTimer;
        timing.isNewRecord = m_isNewRecord;

        m_ctx.racingHud->render(static_cast<float>(windowWidth), static_cast<float>(windowHeight), telem, timing);
    }
}

} // namespace scenes
