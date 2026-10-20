#pragma once

#include "core/Window.hpp"
#include "core/Time.hpp"
#include <memory>
#include <string>

namespace core {

struct AppConfig {
    std::string title{"Prototipo Mario Kart - CGI (UCV)"};
    int width{1280};
    int height{720};
    bool vsync{true};
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

private:
    void processInput();
    void update(float deltaTime);
    void render();

    AppConfig m_config;
    std::unique_ptr<Window> m_window;
    Time m_time;
    bool m_running{false};
};

} // namespace core
