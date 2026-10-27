#pragma once

#include <glm/glm.hpp>

namespace core {

enum class CameraMode {
    CHASE,
    FPS
};

enum class SelectionCategory {
    DRIVER,
    VEHICLE,
    TRACK
};

/**
 * @brief Estado puro de sesion y seleccion activa del jugador (Corredor, Vehiculo, Pista y Camara).
 */
struct SessionState {
    int selectedDriverIndex{0};
    int selectedVehicleIndex{0};
    int selectedTrackIndex{0};
    SelectionCategory selectionCategory{SelectionCategory::DRIVER};
    CameraMode cameraMode{CameraMode::CHASE};

    glm::vec3 kartInitialPos{0.0f};
    float kartInitialYaw{90.0f};

    /**
     * @brief Normaliza un indice dentro del rango [0, count - 1] con aritmetica modular circular (wrap-around).
     */
    [[nodiscard]] static constexpr int wrapIndex(int index, int count) noexcept {
        if (count <= 0) {
            return 0;
        }
        return (index % count + count) % count;
    }

    void setDriverIndex(int index, int totalDrivers) noexcept {
        selectedDriverIndex = wrapIndex(index, totalDrivers);
    }

    void setVehicleIndex(int index, int totalVehicles) noexcept {
        selectedVehicleIndex = wrapIndex(index, totalVehicles);
    }

    void setTrackIndex(int index, int totalTracks) noexcept {
        selectedTrackIndex = wrapIndex(index, totalTracks);
    }

    void cycleCategory(int delta = 1) noexcept {
        const int current = static_cast<int>(selectionCategory);
        selectionCategory = static_cast<SelectionCategory>(wrapIndex(current + delta, 3));
    }

    void toggleCameraMode() noexcept {
        cameraMode = (cameraMode == CameraMode::CHASE) ? CameraMode::FPS : CameraMode::CHASE;
    }
};

} // namespace core
