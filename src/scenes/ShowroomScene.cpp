#include "scenes/ShowroomScene.hpp"
#include "core/Input.hpp"
#include "core/Window.hpp"
#include "core/SceneManager.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <sstream>

namespace scenes {

ShowroomScene::ShowroomScene(core::GameContext& ctx)
    : m_ctx(ctx) {
}

ShowroomScene::~ShowroomScene() {
    m_showroomUI.destroy();
}

void ShowroomScene::onEnter() {
    core::Input::setCursorVisible(true);
    m_showcaseAngle = 0.0f; // Vista frontal fija obligatoria
    m_ctx.applySelection();

    if (!m_showroomUI.isInitialized()) {
        m_showroomUI.init(&m_ctx);
    }

    if (m_ctx.wheelAnimator) {
        m_ctx.wheelAnimator->update(0.0f, 0.0f, 0.0f);
    }
    if (m_ctx.kartEntity && m_ctx.wheelAnimator) {
        m_ctx.kartEntity->update(glm::vec3(0.0f, 0.05f, 0.0f), m_showcaseAngle, *m_ctx.wheelAnimator);
    }

    m_ctx.printSelectionHUD();
}

void ShowroomScene::onExit() {
}

void ShowroomScene::handleInput(float deltaTime) {
    m_showroomUI.handleInput(deltaTime);
}

void ShowroomScene::update(float deltaTime) {
    // En el showroom el kart se mantiene FIJO DE FRENTE (sin rotación continua)
    m_showcaseAngle = 0.0f;

    m_showroomUI.update(deltaTime);

    // Kart en el pedestal del showroom mirando de frente. Es imprescindible propagar
    // el grafo de escena cada fotograma: sin esto las ruedas conservan matrices de mundo
    // obsoletas (origen o posición de carrera) y no se renderizan sobre el chasis.
    if (m_ctx.wheelAnimator) {
        m_ctx.wheelAnimator->update(deltaTime, 0.0f, 0.0f);
    }
    if (m_ctx.kartEntity && m_ctx.wheelAnimator) {
        m_ctx.kartEntity->update(glm::vec3(0.0f, 0.05f, 0.0f), m_showcaseAngle, *m_ctx.wheelAnimator);
    }

    if (m_ctx.lightingController) {
        m_ctx.lightingController->update(glm::vec3(0.0f, 0.05f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), deltaTime);
    }

    m_titleTimer += deltaTime;
    if (m_titleTimer >= 0.20f && m_ctx.window) {
        m_titleTimer = 0.0f;
        std::string dName = (!m_ctx.drivers.empty() && m_ctx.selectedDriverIndex < static_cast<int>(m_ctx.drivers.size()))
                                ? m_ctx.drivers[m_ctx.selectedDriverIndex].name : "";
        std::string vName = (!m_ctx.vehicles.empty() && m_ctx.selectedVehicleIndex < static_cast<int>(m_ctx.vehicles.size()))
                                ? m_ctx.vehicles[m_ctx.selectedVehicleIndex].name : "";
        std::string tName = (!m_ctx.tracks.empty() && m_ctx.selectedTrackIndex < static_cast<int>(m_ctx.tracks.size()))
                                ? m_ctx.tracks[m_ctx.selectedTrackIndex].name : "";
        std::stringstream ss;
        ss << "Prototipo Mario Kart - CGI (UCV) | SHOWROOM ["
           << (m_ctx.selectionCategory == core::SelectionCategory::DRIVER ? ">> 1.CORREDOR: " : "1.Piloto: ") << dName << " | "
           << (m_ctx.selectionCategory == core::SelectionCategory::VEHICLE ? ">> 2.VEHICULO: " : "2.Auto: ") << vName << " | "
           << (m_ctx.selectionCategory == core::SelectionCategory::TRACK ? ">> 3.PISTA: " : "3.Pista: ") << tName
           << "] | [TAB] Categoria | [A/D] Cambiar | [ENTER] Correr";
        m_ctx.window->setTitle(ss.str());
    }
}

void ShowroomScene::render(int windowWidth, int windowHeight) {
    // -----------------------------------------------------------------
    // 1. Pase de Renderizado 3D: Pedestal, Iluminación y Kart en tiempo real
    // -----------------------------------------------------------------
    glClearColor(0.12f, 0.15f, 0.24f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if (m_ctx.litShader && m_ctx.litShader->isValid()) {
        renderer::Shader& shader = *m_ctx.litShader;
        shader.use();

        float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight > 0 ? windowHeight : 1);
        // Desplazamiento sutil de la cámara (0.35f) para encuadrar perfectamente el kart
        // a la izquierda del panel lateral de estadísticas
        glm::vec3 camPos(0.35f, 1.35f, 3.4f);
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.35f, 0.40f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        if (m_ctx.lightManager) {
            m_ctx.lightManager->applyToShader(shader, camPos);
        }

        shader.setMat4("u_View", view);
        shader.setMat4("u_Projection", proj);
        shader.setInt("u_Texture", 0);

        if (m_ctx.paletteTexture && m_ctx.paletteTexture->isLoaded()) {
            m_ctx.paletteTexture->bind(0);
        }

        // 1.1 Dibujar el Pedestal bajo el kart
        if (m_ctx.pedestalMesh) {
            shader.setBool("u_UseTexture", false);
            shader.setBool("u_UseVertexColor", false);
            glm::mat4 pedMat = glm::rotate(glm::mat4(1.0f), glm::radians(m_showcaseAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            shader.setMat4("u_Model", pedMat);
            shader.setVec4("u_Color", glm::vec4(0.20f, 0.22f, 0.28f, 1.0f));
            m_ctx.pedestalMesh->draw();
        }

        // 1.2 Dibujar el Kart y Piloto DE FRENTE con sus texturas
        if (m_ctx.kartEntity) {
            shader.setBool("u_UseTexture", true);
            shader.setBool("u_UseVertexColor", false);
            shader.setVec4("u_Color", glm::vec4(1.0f));
            m_ctx.kartEntity->draw(shader);
        }
    }

    // -----------------------------------------------------------------
    // 2. Pase de Renderizado 2D: Interfaz gráfica ShowroomUI
    // -----------------------------------------------------------------
    m_showroomUI.render(static_cast<float>(windowWidth), static_cast<float>(windowHeight));
}

} // namespace scenes
