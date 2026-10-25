#pragma once

namespace core {

/**
 * @brief Interfaz base para las escenas del juego (MainMenu, Showroom, Racing).
 * Diseñado bajo el principio de modulos profundos para desacoplar el ciclo de vida
 * de entrada, actualizacion y pase de renderizado especifico de cada estado.
 */
class IScene {
public:
    virtual ~IScene() = default;

    virtual void onEnter() {}
    virtual void onExit() {}

    virtual void handleInput(float deltaTime) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(int windowWidth, int windowHeight) = 0;
};

} // namespace core
