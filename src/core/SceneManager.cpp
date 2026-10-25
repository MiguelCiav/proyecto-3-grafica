#include "core/SceneManager.hpp"
#include <iostream>

namespace core {

void SceneManager::registerScene(GameState state, std::shared_ptr<IScene> scene) {
    m_scenes[static_cast<int>(state)] = std::move(scene);
}

void SceneManager::changeScene(GameState nextState) {
    auto it = m_scenes.find(static_cast<int>(nextState));
    if (it == m_scenes.end()) {
        std::cerr << "[SceneManager] Error: No se encontro escena para el estado especificado." << std::endl;
        return;
    }

    if (m_currentScene) {
        m_currentScene->onExit();
    }

    m_currentState = nextState;
    m_currentScene = it->second;

    if (m_currentScene) {
        m_currentScene->onEnter();
    }
}

void SceneManager::handleInput(float deltaTime) {
    if (m_currentScene) {
        m_currentScene->handleInput(deltaTime);
    }
}

void SceneManager::update(float deltaTime) {
    if (m_currentScene) {
        m_currentScene->update(deltaTime);
    }
}

void SceneManager::render(int windowWidth, int windowHeight) {
    if (m_currentScene) {
        m_currentScene->render(windowWidth, windowHeight);
    }
}

} // namespace core
