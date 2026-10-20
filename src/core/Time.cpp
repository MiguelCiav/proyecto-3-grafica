#include "core/Time.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>

namespace core {

Time::Time() {
    reset();
}

void Time::reset() {
    m_currentTime = static_cast<float>(glfwGetTime());
    m_lastTime = m_currentTime;
    m_deltaTime = 0.0f;
    m_fpsTimer = 0.0f;
    m_frameCount = 0;
    m_fps = 60.0f;
    m_frameTimeMs = 16.66f;
}

void Time::update() {
    m_currentTime = static_cast<float>(glfwGetTime());
    m_deltaTime = m_currentTime - m_lastTime;
    m_lastTime = m_currentTime;

    // Clamping defensivo de deltaTime para evitar saltos bruscos en física/cámara
    // tras pausas, breakpoints o arrastre de ventana (máximo 100ms = 10 FPS mínimos)
    m_deltaTime = std::min(m_deltaTime, 0.1f);

    // Actualizar contador de FPS
    m_frameCount++;
    m_fpsTimer += m_deltaTime;

    if (m_fpsTimer >= 1.0f) {
        m_fps = static_cast<float>(m_frameCount) / m_fpsTimer;
        m_frameTimeMs = (m_fps > 0.0f) ? (1000.0f / m_fps) : 0.0f;
        m_frameCount = 0;
        m_fpsTimer = 0.0f;
    }
}

} // namespace core
