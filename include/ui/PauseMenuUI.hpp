#pragma once

#include "renderer/Texture.hpp"
#include <glm/glm.hpp>

namespace lighting {
class LightingController;
class LightManager;
}

namespace ui {

class UIRenderer;

/**
 * @brief Acciones resultantes de la interaccion del usuario con el menu de pausa.
 */
enum class PauseAction {
    NONE = 0,
    RESUME,
    RESTART,
    GO_TO_MAIN_MENU,
    QUIT_GAME
};

/**
 * @brief Componente modular de interfaz de usuario para el menu de pausa de la carrera.
 *
 * Encapsula la presentacion visual y logica de navegacion del modal de pausa:
 * - Navegacion por teclado vertical (W / S / Flechas Arriba/Abajo) con wrap-around ciclico.
 * - Atajos numericos directos (1 a 7) tanto en fila superior como teclado numerico.
 * - Modificacion de opciones horizontales (A / D / Flechas Izquierda/Derecha).
 * - Confirmacion por ENTER o ESPACIO.
 * - Hit-testing de raton en tiempo real (hover y clic sobre las opciones).
 * - Conmutacion interactiva de modelos de sombreado (Flat, Lambert, Blinn-Phong).
 * - Alternancia del ciclo dia/noche y faros spotlight del kart.
 * - Notificacion de acciones desacopladas (Continuar, Reiniciar, Volver al Menu, Salir).
 */
class PauseMenuUI {
public:
    PauseMenuUI();
    ~PauseMenuUI() = default;

    // Deshabilitar copia para garantizar gestion RAII segura de texturas en GPU
    PauseMenuUI(const PauseMenuUI&) = delete;
    PauseMenuUI& operator=(const PauseMenuUI&) = delete;

    // Permitir movimiento
    PauseMenuUI(PauseMenuUI&&) noexcept = default;
    PauseMenuUI& operator=(PauseMenuUI&&) noexcept = default;

    /**
     * @brief Inicializa el componente de pausa cargando las texturas de Kenney UI Pack.
     * @param renderer Puntero al renderizador 2D por lotes del subsistema UI.
     * @return true si la inicializacion fue exitosa.
     */
    bool init(UIRenderer* renderer);

    /**
     * @brief Abre el menu de pausa y hace visible el cursor del raton.
     */
    void open();

    /**
     * @brief Cierra el menu de pausa.
     */
    void close();

    /**
     * @brief Indica si el menu de pausa se encuentra actualmente abierto.
     */
    [[nodiscard]] bool isOpen() const { return m_isOpen; }

    /**
     * @brief Actualiza temporizadores y animaciones internas del modal.
     * @param deltaTime Tiempo transcurrido desde el ultimo frame en segundos.
     */
    void update(float deltaTime);

    /**
     * @brief Procesa la entrada de teclado y raton cuando el menu esta desplegado.
     * @param deltaTime Tiempo transcurrido en segundos para animaciones internas.
     * @param lightingCtrl Puntero al controlador de iluminacion (dia/noche, faros).
     * @param lightMgr Puntero al gestor de iluminacion (modelos de sombreado).
     */
    void handleInput(float deltaTime,
                     lighting::LightingController* lightingCtrl,
                     lighting::LightManager* lightMgr);

    /**
     * @brief Sobrecarga de handleInput especificando dimensiones de pantalla para hit-testing.
     */
    void handleInput(float deltaTime,
                     lighting::LightingController* lightingCtrl,
                     lighting::LightManager* lightMgr,
                     float screenWidth,
                     float screenHeight);

    /**
     * @brief Devuelve la accion pendiente seleccionada y la reinicia a NONE.
     * @return Accion a ejecutar por la escena que posee el componente.
     */
    [[nodiscard]] PauseAction consumeAction();

    /**
     * @brief Consulta la accion pendiente actual sin reiniciarla.
     */
    [[nodiscard]] PauseAction getPendingAction() const { return m_pendingAction; }

    /**
     * @brief Renderiza el fondo oscurecido, marco modal 9-slice, botones y controles interactivos.
     * @param screenWidth Ancho actual del viewport / pantalla.
     * @param screenHeight Alto actual del viewport / pantalla.
     * @param lightingCtrl Puntero al controlador de iluminacion para consultar estados.
     * @param lightMgr Puntero al gestor de iluminacion para consultar modelos.
     */
    void render(float screenWidth,
                float screenHeight,
                const lighting::LightingController* lightingCtrl,
                const lighting::LightManager* lightMgr);

    /**
     * @brief Obtiene el indice de la opcion actualmente seleccionada (0 a 6).
     */
    [[nodiscard]] int getSelectedOption() const { return m_selectedOption; }

    /**
     * @brief Establece manualmente el indice de la opcion seleccionada (0 a 6).
     */
    void setSelectedOption(int option);

    /**
     * @brief Obtiene el temporizador de animacion acumulado.
     */
    [[nodiscard]] float getAnimTimer() const { return m_animTimer; }

    /**
     * @brief Establece el temporizador de animacion acumulado.
     */
    void setAnimTimer(float timer) { m_animTimer = timer; }

    /**
     * @brief Verifica si el componente ha sido inicializado.
     */
    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    /**
     * @brief Verifica si todas las texturas requeridas fueron cargadas exitosamente en GPU.
     */
    [[nodiscard]] bool areTexturesLoaded() const;

private:
    UIRenderer* m_renderer{nullptr};

    // Texturas de Kenney UI Pack y Space Expansion
    renderer::Texture m_texPauseModalFrame;
    renderer::Texture m_texPauseBtnIdle;
    renderer::Texture m_texPauseBtnHover;
    renderer::Texture m_texArrowPointer;
    renderer::Texture m_texCheckOn;
    renderer::Texture m_texCheckOff;

    PauseAction m_pendingAction{PauseAction::NONE};
    int m_selectedOption{0};
    bool m_isOpen{false};
    float m_animTimer{0.0f};
    bool m_initialized{false};

    float m_screenWidth{1280.0f};
    float m_screenHeight{720.0f};
};

} // namespace ui
