#pragma once

namespace core {

class Time {
public:
    Time();

    // Actualiza contadores y calcula deltaTime y FPS
    void update();

    // Getters en segundos
    float getDeltaTime() const { return m_deltaTime; }
    float getElapsedTime() const { return m_currentTime; }
    
    // Métricas de rendimiento
    float getFPS() const { return m_fps; }
    float getFrameTimeMs() const { return m_frameTimeMs; }

    // Reinicia contadores
    void reset();

private:
    float m_currentTime{0.0f};
    float m_lastTime{0.0f};
    float m_deltaTime{0.0f};

    // Cálculo estable de FPS promediado por segundo
    float m_fpsTimer{0.0f};
    unsigned int m_frameCount{0};
    float m_fps{0.0f};
    float m_frameTimeMs{0.0f};
};

} // namespace core
