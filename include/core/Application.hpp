#pragma once

#include "core/Window.hpp"
#include "core/Time.hpp"
#include "camera/Camera.hpp"
#include "renderer/Shader.hpp"
#include "renderer/Texture.hpp"
#include "renderer/MeshGL.hpp"
#include "geometry/GeometryData.hpp"
#include <memory>
#include <string>
#include <vector>

namespace core {

struct AppConfig {
    std::string title{"Prototipo Mario Kart - CGI (UCV)"};
    int width{1280};
    int height{720};
    bool vsync{true};
};

struct RenderItem {
    std::string name;
    renderer::MeshGL mesh;
    glm::mat4 transform{1.0f};
    glm::vec4 color{1.0f};
};

class Application {
public:
    static Application& getInstance();

    explicit Application(const AppConfig& config = AppConfig());
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool init();
    void run();
    void close();

    Window& getWindow() { return *m_window; }
    const Time& getTime() const { return m_time; }
    camera::Camera& getCamera() { return m_camera; }

private:
    void processInput();
    void update(float deltaTime);
    void render();

    AppConfig m_config;
    std::unique_ptr<Window> m_window;
    Time m_time;
    camera::Camera m_camera;
    bool m_running{false};
    bool m_cursorLocked{false};

    // Recursos de renderizado (Fase 3 - Issue #12)
    renderer::Shader m_basicShader;
    renderer::Texture m_paletteTexture;
    std::vector<RenderItem> m_renderItems;
};

} // namespace core
