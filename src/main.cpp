#include "core/Application.hpp"
#include <iostream>

int main() {
    try {
        core::AppConfig config;
        config.title = "Prototipo Mario Kart - CGI (UCV)";
        config.width = 1280;
        config.height = 720;
        config.vsync = true;

        core::Application app(config);
        if (!app.init()) {
            std::cerr << "[FATAL] Fallo al inicializar la aplicación." << std::endl;
            return -1;
        }

        app.run();
    } catch (const std::exception& e) {
        std::cerr << "[EXCEPTION] " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "[UNKNOWN EXCEPTION] Error inesperado en el ciclo de ejecución." << std::endl;
        return -1;
    }

    return 0;
}
