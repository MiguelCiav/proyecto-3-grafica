#pragma once

#include <string>
#include <functional>

// Forward declaration para no exponer cabeceras crudas de GLFW en includes públicos
struct GLFWwindow;

namespace core {

struct WindowProps {
    std::string title;
    int width;
    int height;
    bool vsync;
    bool resizable;

    WindowProps(const std::string& t = "Prototipo Mario Kart - CGI (UCV)",
                int w = 1280,
                int h = 720,
                bool v = true,
                bool r = true)
        : title(t), width(w), height(h), vsync(v), resizable(r) {}
};

class Window {
public:
    using ResizeCallback = std::function<void(int width, int height)>;

    explicit Window(const WindowProps& props = WindowProps());
    ~Window();

    // Eliminar copia para garantizar propiedad RAII estricta del contexto
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Permitir semántica de movimiento
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    // Inicialización del contexto y ventana
    bool init();
    void shutdown();

    // Control del ciclo de ventana
    bool shouldClose() const;
    void setShouldClose(bool close);
    void pollEvents() const;
    void swapBuffers() const;

    // Limpieza de pantalla con OpenGL
    void clear(float r = 0.1f, float g = 0.1f, float b = 0.12f, float a = 1.0f) const;

    // Configuración dinámica
    void setVSync(bool enabled);
    bool isVSync() const { return m_props.vsync; }
    void setTitle(const std::string& title);

    // Getters
    int getWidth() const { return m_props.width; }
    int getHeight() const { return m_props.height; }
    float getAspectRatio() const {
        return (m_props.height > 0) ? static_cast<float>(m_props.width) / static_cast<float>(m_props.height) : 1.0f;
    }
    GLFWwindow* getNativeWindow() const { return m_window; }
    bool isInitialized() const { return m_initialized; }

    // Callback de redimensionamiento
    void setResizeCallback(ResizeCallback callback) { m_resizeCallback = callback; }

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    WindowProps m_props;
    GLFWwindow* m_window{nullptr};
    ResizeCallback m_resizeCallback;
    bool m_initialized{false};
};

} // namespace core
