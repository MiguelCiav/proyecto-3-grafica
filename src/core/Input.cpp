#include "core/Input.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

namespace core {

void Input::init(GLFWwindow* window) {
    s_window = window;

    s_keys.fill(false);
    s_keysJustPressed.fill(false);
    s_mouseButtons.fill(false);
    s_mouseButtonsJustPressed.fill(false);

    s_mouseX = 0.0f;
    s_mouseY = 0.0f;
    s_lastMouseX = 0.0f;
    s_lastMouseY = 0.0f;
    s_mouseDeltaX = 0.0f;
    s_mouseDeltaY = 0.0f;
    s_scrollDeltaY = 0.0f;
    s_firstMouse = true;
    s_cursorVisible = true;

    if (!s_window) return;

    glfwSetKeyCallback(s_window, keyCallback);
    glfwSetCursorPosCallback(s_window, cursorPosCallback);
    glfwSetMouseButtonCallback(s_window, mouseButtonCallback);
    glfwSetScrollCallback(s_window, scrollCallback);

    std::cout << "[OK] [Input] Gestor de entrada (teclado/mouse) vinculado a la ventana." << std::endl;
}

void Input::update() {
    // Resetear deltas de movimiento y pulsaciones de un solo fotograma
    s_keysJustPressed.fill(false);
    s_mouseButtonsJustPressed.fill(false);
    s_mouseDeltaX = 0.0f;
    s_mouseDeltaY = 0.0f;
    s_scrollDeltaY = 0.0f;
}

bool Input::isKeyPressed(int keyCode) {
    if (keyCode >= 0 && static_cast<size_t>(keyCode) < MAX_KEYS) {
        return s_keys[keyCode];
    }
    return false;
}

bool Input::isKeyJustPressed(int keyCode) {
    if (keyCode >= 0 && static_cast<size_t>(keyCode) < MAX_KEYS) {
        return s_keysJustPressed[keyCode];
    }
    return false;
}

bool Input::isMouseButtonPressed(int button) {
    if (button >= 0 && static_cast<size_t>(button) < MAX_BUTTONS) {
        return s_mouseButtons[button];
    }
    return false;
}

bool Input::isMouseButtonJustPressed(int button) {
    if (button >= 0 && static_cast<size_t>(button) < MAX_BUTTONS) {
        return s_mouseButtonsJustPressed[button];
    }
    return false;
}

float Input::getMouseX() { return s_mouseX; }
float Input::getMouseY() { return s_mouseY; }
float Input::getMouseDeltaX() { return s_mouseDeltaX; }
float Input::getMouseDeltaY() { return s_mouseDeltaY; }
float Input::getScrollDeltaY() { return s_scrollDeltaY; }

void Input::setCursorVisible(bool visible) {
    s_cursorVisible = visible;
    if (s_window) {
        glfwSetInputMode(s_window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
}

bool Input::isCursorVisible() {
    return s_cursorVisible;
}

void Input::keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key >= 0 && static_cast<size_t>(key) < MAX_KEYS) {
        if (action == GLFW_PRESS) {
            s_keys[key] = true;
            s_keysJustPressed[key] = true;
        } else if (action == GLFW_RELEASE) {
            s_keys[key] = false;
        }
    }
}

void Input::cursorPosCallback(GLFWwindow* /*window*/, double xpos, double ypos) {
    float x = static_cast<float>(xpos);
    float y = static_cast<float>(ypos);

    if (s_firstMouse) {
        s_lastMouseX = x;
        s_lastMouseY = y;
        s_firstMouse = false;
    }

    s_mouseDeltaX += (x - s_lastMouseX);
    s_mouseDeltaY += (s_lastMouseY - y); // Invertir Y para coordenadas estándar (arriba positivo)

    s_lastMouseX = x;
    s_lastMouseY = y;
    s_mouseX = x;
    s_mouseY = y;
}

void Input::mouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
    if (button >= 0 && static_cast<size_t>(button) < MAX_BUTTONS) {
        if (action == GLFW_PRESS) {
            s_mouseButtons[button] = true;
            s_mouseButtonsJustPressed[button] = true;
        } else if (action == GLFW_RELEASE) {
            s_mouseButtons[button] = false;
        }
    }
}

void Input::scrollCallback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset) {
    s_scrollDeltaY += static_cast<float>(yoffset);
}

} // namespace core
