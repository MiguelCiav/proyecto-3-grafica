#include "core/Window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace core {

Window::Window(const WindowProps& props)
    : m_props(props)
{
}

Window::~Window() {
    shutdown();
}

Window::Window(Window&& other) noexcept
    : m_props(std::move(other.m_props))
    , m_window(other.m_window)
    , m_resizeCallback(std::move(other.m_resizeCallback))
    , m_initialized(other.m_initialized)
{
    other.m_window = nullptr;
    other.m_initialized = false;
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        shutdown();
        m_props = std::move(other.m_props);
        m_window = other.m_window;
        m_resizeCallback = std::move(other.m_resizeCallback);
        m_initialized = other.m_initialized;

        other.m_window = nullptr;
        other.m_initialized = false;
    }
    return *this;
}

bool Window::init() {
    if (m_initialized) {
        return true;
    }

    if (!glfwInit()) {
        std::cerr << "[ERROR] [Window] No se pudo inicializar GLFW." << std::endl;
        return false;
    }

    // Configurar hints de OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, m_props.resizable ? GLFW_TRUE : GLFW_FALSE);

    m_window = glfwCreateWindow(m_props.width, m_props.height, m_props.title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "[ERROR] [Window] Fallo al crear la ventana GLFW." << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

    // Inicializar GLAD con el loader de GLFW
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "[ERROR] [Window] Fallo al inicializar GLAD." << std::endl;
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        glfwTerminate();
        return false;
    }

    // Configuración inicial de OpenGL
    int fbWidth = 0, fbHeight = 0;
    glfwGetFramebufferSize(m_window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    setVSync(m_props.vsync);

    std::cout << "[OK] [Window] Contexto OpenGL 3.3 inicializado exitosamente:" << std::endl;
    std::cout << "     - Fabricante: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "     - Renderer:   " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "     - Version:    " << glGetString(GL_VERSION) << std::endl;
    std::cout << "     - Resolucion: " << fbWidth << "x" << fbHeight << std::endl;

    m_initialized = true;
    return true;
}

void Window::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    if (m_initialized) {
        glfwTerminate();
        m_initialized = false;
    }
}

bool Window::shouldClose() const {
    return m_window ? glfwWindowShouldClose(m_window) : true;
}

void Window::setShouldClose(bool close) {
    if (m_window) {
        glfwSetWindowShouldClose(m_window, close ? GLFW_TRUE : GLFW_FALSE);
    }
}

void Window::pollEvents() const {
    glfwPollEvents();
}

void Window::swapBuffers() const {
    if (m_window) {
        glfwSwapBuffers(m_window);
    }
}

void Window::clear(float r, float g, float b, float a) const {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::setVSync(bool enabled) {
    m_props.vsync = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

void Window::setTitle(const std::string& title) {
    m_props.title = title;
    if (m_window) {
        glfwSetWindowTitle(m_window, title.c_str());
    }
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->m_props.width = width;
        self->m_props.height = height;
        glViewport(0, 0, width, height);

        if (self->m_resizeCallback) {
            self->m_resizeCallback(width, height);
        }
    }
}

} // namespace core
