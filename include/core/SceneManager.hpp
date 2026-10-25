#pragma once

#include <memory>
#include <unordered_map>
#include "core/IScene.hpp"

namespace core {

enum class GameState {
    MAIN_MENU,       // Pantalla de inicio con camara orbital cinematografica
    VEHICLE_SELECT,  // Showroom 3D con seleccion de corredor, vehiculo y pista de frente
    RACING           // Modo de juego de carreras activo
};

/**
 * @brief Administrador del grafo de escenas y transiciones de estados del juego.
 */
class SceneManager {
public:
    SceneManager() = default;
    ~SceneManager() = default;

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    void registerScene(GameState state, std::shared_ptr<IScene> scene);
    void changeScene(GameState nextState);

    [[nodiscard]] GameState getCurrentState() const { return m_currentState; }
    [[nodiscard]] IScene* getCurrentScene() const { return m_currentScene.get(); }

    void handleInput(float deltaTime);
    void update(float deltaTime);
    void render(int windowWidth, int windowHeight);

private:
    GameState m_currentState{GameState::MAIN_MENU};
    std::shared_ptr<IScene> m_currentScene{nullptr};
    std::unordered_map<int, std::shared_ptr<IScene>> m_scenes;
};

} // namespace core
