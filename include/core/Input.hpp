#pragma once

#include <array>
#include <cstddef>

struct GLFWwindow;

namespace core {

class Input {
public:
    // Inicialización y enlace con la ventana de GLFW
    static void init(GLFWwindow* window);

    // Debe llamarse al inicio o final de cada fotograma para limpiar deltas y eventos únicos
    static void update();

    // Consulta de teclado
    static bool isKeyPressed(int keyCode);
    static bool isKeyJustPressed(int keyCode);

    // Consulta de mouse
    static bool isMouseButtonPressed(int button);
    static bool isMouseButtonJustPressed(int button);

    static float getMouseX();
    static float getMouseY();
    static float getMouseDeltaX();
    static float getMouseDeltaY();
    static float getScrollDeltaY();

    // Control del cursor
    static void setCursorVisible(bool visible);
    static bool isCursorVisible();

    // Simulación de entrada para pruebas
    static void simulateKey(int key, int action);

private:
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    static inline GLFWwindow* s_window{nullptr};

    static constexpr size_t MAX_KEYS = 512;
    static constexpr size_t MAX_BUTTONS = 16;

    static inline std::array<bool, MAX_KEYS> s_keys{};
    static inline std::array<bool, MAX_KEYS> s_keysJustPressed{};

    static inline std::array<bool, MAX_BUTTONS> s_mouseButtons{};
    static inline std::array<bool, MAX_BUTTONS> s_mouseButtonsJustPressed{};

    static inline float s_mouseX{0.0f};
    static inline float s_mouseY{0.0f};
    static inline float s_lastMouseX{0.0f};
    static inline float s_lastMouseY{0.0f};
    static inline float s_mouseDeltaX{0.0f};
    static inline float s_mouseDeltaY{0.0f};
    static inline float s_scrollDeltaY{0.0f};
    static inline bool s_firstMouse{true};
    static inline bool s_cursorVisible{true};
};

} // namespace core
