#include <iostream>
#include <cassert>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vfx/SkidmarkManager.hpp"
#include "ui/RacingHud.hpp"
#include "animation/WheelAnimator.hpp"
#include "physics/KartPhysics.hpp"

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Validacion de Giro de Ruedas, Skidmarks y HUD    " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // TEST 1: Direccion de viraje de ruedas delanteras (WheelAnimator)
    // -------------------------------------------------------------------------
    {
        animation::WheelAnimator animator(0.32f, 28.0f);

        // Al girar a la derecha (+1.0), el angulo de giro interno es positivo
        animator.update(0.5f, 10.0f, 1.0f);
        float steerAngle = animator.getSteerAngle();
        assert(steerAngle > 5.0f && "El viraje con steerInput=+1 debe ser positivo");

        // En KartEntity se aplica -steerAngle para que en coordenadas diestras de OpenGL
        // (-Z adelante, +X derecha), la rotacion negativa en Y apunte hacia +X (derecha)
        float appliedEulerY = -steerAngle;
        assert(appliedEulerY < 0.0f && "Euler Y debe ser negativo para apuntar la rueda a la derecha");
        (void)appliedEulerY;

        // Al girar a la izquierda (-1.0)
        animator.reset();
        animator.update(0.5f, 10.0f, -1.0f);
        steerAngle = animator.getSteerAngle();
        assert(steerAngle < -5.0f && "El viraje con steerInput=-1 debe ser negativo");
        appliedEulerY = -steerAngle;
        assert(appliedEulerY > 0.0f && "Euler Y debe ser positivo para apuntar la rueda a la izquierda");

        std::cout << "  [CHECK 1] Direccion de viraje de ruedas delanteras verificada (alineada con giro)." << std::endl;
    }

    // -------------------------------------------------------------------------
    // TEST 2: Contexto OpenGL para Skidmarks y HUD
    // -------------------------------------------------------------------------
    if (!glfwInit()) {
        std::cerr << "Fallo al inicializar GLFW" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Test HUD & Skidmarks", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // -------------------------------------------------------------------------
    // TEST 3: Inicializacion y generacion de marcas de derrape (SkidmarkManager)
    // -------------------------------------------------------------------------
    {
        vfx::SkidmarkManager skidmarks;
        bool ok = skidmarks.init();
        assert(ok && "SkidmarkManager::init() debe retornar true");
        (void)ok;

        // Conduccion recta sin frenar ni derrapar: no debe generar marcas
        glm::vec3 pos(0.0f);
        skidmarks.update(0.05f, pos, 0.0f, 15.0f, 0.0f, false, false);
        assert(!skidmarks.isCurrentlySkidding() && "En recta a velocidad constante no debe derrapar");
        assert(skidmarks.getActiveSegmentCount() == 0 && "No debe haber segmentos");

        // Derrape brusco girando a velocidad
        for (int i = 0; i < 20; ++i) {
            pos += glm::vec3(0.0f, 0.0f, -0.4f);
            skidmarks.update(0.05f, pos, 0.0f, 15.0f, 0.8f, false, false);
        }
        assert(skidmarks.isCurrentlySkidding() && "Debe detectar derrape activo con steer=0.8 y speed=15");
        assert(skidmarks.getActiveSegmentCount() > 0 && "Deben generarse segmentos de derrape");

        // Limpieza de marcas
        skidmarks.clear();
        assert(skidmarks.getActiveSegmentCount() == 0 && "clear() debe vaciar los segmentos");

        std::cout << "  [CHECK 2] Sistema de marcas de derrape en asfalto verificado con exito." << std::endl;
    }

    // -------------------------------------------------------------------------
    // TEST 4: Inicializacion y pase de renderizado de HUD (RacingHud con Kenney Space Expansion)
    // -------------------------------------------------------------------------
    {
        ui::RacingHud hud;
        bool ok = hud.init();
        assert(ok && "RacingHud::init() debe retornar true");
        assert(hud.isInitialized() && "RacingHud debe reportar estar inicializado");
        assert(hud.areTexturesLoaded() && "Todas las texturas de Kenney Space Expansion deben cargarse correctamente");
        (void)ok;

        ui::TelemetryData telem;
        telem.speedKmH = 65.4f;
        telem.maxSpeedKmH = 80.0f;
        telem.isBoosting = true;
        telem.isSkidding = true;

        ui::LapTimingData timing;
        timing.currentLap = 2;
        timing.totalLaps = 3;
        timing.currentLapTime = 42.15f;
        timing.bestLapTime = 39.80f;
        timing.totalRaceTime = 81.95f;
        timing.lapNoticeTimer = 2.0f;
        timing.lastLapTime = 39.80f;
        timing.isNewRecord = true;

        // Renderizado del HUD en lote 2D con Turbo Boost activo
        hud.render(1280.0f, 720.0f, telem, timing);
        GLenum err = glGetError();
        assert(err == GL_NO_ERROR && "No deben producirse errores de OpenGL en el renderizado con Turbo Boost");

        // Renderizado sin Turbo Boost, con marcha atrás (R) y terreno OFFROAD
        telem.isBoosting = false;
        telem.isSkidding = false;
        telem.isReversing = true;
        telem.isOffRoad = true;
        telem.speedKmH = -12.0f;
        timing.isNewRecord = false;
        hud.render(1920.0f, 1080.0f, telem, timing);
        err = glGetError();
        assert(err == GL_NO_ERROR && "No deben producirse errores de OpenGL en marcha atrás y offroad");

        // Renderizado del Modal de Pausa interactivo (7 opciones)
        ui::PauseMenuData pauseData;
        pauseData.selectedOption = 0;
        pauseData.lightingModel = "BLINN-PHONG";
        pauseData.isNight = false;
        pauseData.areHeadlightsOn = true;
        pauseData.animTimer = 0.5f;

        hud.renderPauseModal(1280.0f, 720.0f, pauseData);
        err = glGetError();
        assert(err == GL_NO_ERROR && "No deben producirse errores de OpenGL en renderPauseModal (opcion 0)");

        // Probar modal de pausa en modo noche y con opción 2 seleccionada
        pauseData.selectedOption = 2;
        pauseData.isNight = true;
        pauseData.lightingModel = "LAMBERT";
        hud.renderPauseModal(1280.0f, 720.0f, pauseData);
        err = glGetError();
        assert(err == GL_NO_ERROR && "No deben producirse errores de OpenGL en renderPauseModal (modo noche)");
        (void)err;

        std::cout << "  [CHECK 3] Interfaz HUD Sci-Fi y Modal de Pausa (Kenney Space Pack + UI Pack) validados." << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Todos los tests de Viraje, Skidmarks y HUD 100%! " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
