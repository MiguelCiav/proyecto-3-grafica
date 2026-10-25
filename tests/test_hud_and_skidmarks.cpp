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

    // -------------------------------------------------------------------------
    // TEST 4: Validacion de Dinamica de Vehiculo, Derrape, Wall Sliding y Cesped
    // -------------------------------------------------------------------------
    {
        Engine::Physics::KartPhysics kart;

        // 1. Validar colisionadores reducidos al 70% (0.70x)
        auto sphere = kart.getBoundingSphere();
        assert(std::abs(sphere.radius - 0.34f) < 0.001f && "Radio de esfera debe ser 0.34f");
        auto box = kart.getBoundingBox();
        assert(std::abs(box.halfExtents.x - 0.38f) < 0.001f && "Half-extent X de caja debe ser 0.38f");
        assert(std::abs(box.halfExtents.y - 0.28f) < 0.001f && "Half-extent Y de caja debe ser 0.28f");
        assert(std::abs(box.halfExtents.z - 0.56f) < 0.001f && "Half-extent Z de caja debe ser 0.56f");
        (void)sphere;
        (void)box;

        // 2. Mecanica de Derrape (Power Slide): steer > 0.1 y drift=true
        kart.setPosition(glm::vec3(0.0f));
        kart.setYaw(0.0f);
        // Acelerar un poco para tener velocidad
        kart.update(1.0f, true, false, 0.0f, false);
        float speedBefore = kart.getSpeed();
        assert(speedBefore > 5.0f);
        (void)speedBefore;

        // Sin drift: steer a tope (1.0f) genera scrubbing y m_isDrifting es false
        kart.update(0.1f, true, false, 1.0f, false);
        assert(!kart.isDrifting());
        float yawNoDrift = kart.getYaw();
        (void)yawNoDrift;

        // Con drift: bandera m_isDrifting = true y mayor tasa de viraje (1.35x)
        Engine::Physics::KartPhysics kartDrift;
        kartDrift.setPosition(glm::vec3(0.0f));
        kartDrift.setYaw(0.0f);
        kartDrift.update(1.0f, true, false, 0.0f, false);
        kartDrift.update(0.1f, true, false, 1.0f, true);
        assert(kartDrift.isDrifting());
        // El giro debe ser mayor con drift que sin drift
        assert(std::abs(kartDrift.getYaw()) > std::abs(yawNoDrift));

        // 3. Deslizamiento rasante en muros (Wall Sliding)
        // Impacto lateral rasante (dot ~ 0): el kart debe conservar el 85% de su velocidad
        Engine::Physics::KartPhysics kartWall;
        kartWall.setPosition(glm::vec3(0.0f));
        kartWall.setYaw(0.0f); // forward = (0, 0, 1)
        kartWall.update(1.0f, true, false, 0.0f);
        float speedBeforeImpact = kartWall.getSpeed();
        // Separación mínima perpendicular al avance: mtv = (0.5f, 0.0f, 0.0f), dot(forward, mtv) = 0.0f
        kartWall.onCollision(glm::vec3(0.5f, 0.0f, 0.0f), 0.2f);
        assert(std::abs(kartWall.getSpeed() - speedBeforeImpact * 0.85f) < 0.01f && "Wall sliding debe preservar el 85% de velocidad");
        (void)speedBeforeImpact;

        // Choque frontal directo (dot = -1.0f): debe rebotar
        Engine::Physics::KartPhysics kartHeadOn;
        kartHeadOn.setPosition(glm::vec3(0.0f));
        kartHeadOn.setYaw(0.0f); // forward = (0, 0, 1)
        kartHeadOn.update(1.0f, true, false, 0.0f);
        float headOnSpeed = kartHeadOn.getSpeed();
        kartHeadOn.onCollision(glm::vec3(0.0f, 0.0f, -0.5f), 0.25f);
        assert(kartHeadOn.getSpeed() < 0.0f && std::abs(kartHeadOn.getSpeed() - (-headOnSpeed * 0.25f)) < 0.01f);
        (void)headOnSpeed;

        // 4. Penalización firme en césped
        Engine::Physics::KartPhysics kartGrass;
        kartGrass.setPosition(glm::vec3(0.0f));
        // Acelerar en asfalto a alta velocidad (~14 m/s)
        for (int i = 0; i < 40; ++i) {
            kartGrass.update(0.05f, true, false, 0.0f);
        }
        assert(kartGrass.getSpeed() > 12.0f);
        // Salir al césped (fricción 0.28)
        kartGrass.setSurfaceFriction(0.28f);
        // En un solo paso de 0.1s debe haber frenado firmemente (-28 m/s * 0.1s = -2.8 m/s)
        float preGrassSpeed = kartGrass.getSpeed();
        kartGrass.update(0.1f, true, false, 0.0f);
        assert(kartGrass.getSpeed() <= preGrassSpeed - 2.5f && "El kart debe frenar con fuerza al salir al cesped");
        (void)preGrassSpeed;

        // 5. Inmunidad al césped con Boost
        Engine::Physics::KartPhysics kartBoostGrass;
        kartBoostGrass.setPosition(glm::vec3(0.0f));
        kartBoostGrass.setSurfaceFriction(0.28f);
        kartBoostGrass.applyBoost(1.2f, 1.28f, 3.5f);
        assert(kartBoostGrass.isBoosting());
        kartBoostGrass.update(0.1f, true, false, 0.0f);
        // La velocidad en boost no debe ser limitada a m_maxSpeed * 0.28f
        assert(kartBoostGrass.getSpeed() > kartBoostGrass.m_maxSpeed * 0.5f);

        std::cout << "  [CHECK 4] Escala 70%, Derrape, Wall-Sliding, Penalizacion de cesped e Inmunidad Turbo validados." << std::endl;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Todos los tests de Viraje, Skidmarks y HUD 100%! " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
