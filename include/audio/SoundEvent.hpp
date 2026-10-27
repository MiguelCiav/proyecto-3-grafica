#pragma once

#include <cstdint>

namespace audio {

/**
 * @brief Eventos de sonido discretos de interfaz de usuario (SFX).
 */
enum class SoundEvent : uint8_t {
    UI_NAVIGATE = 0, ///< Desplazamiento del cursor / hover entre opciones (tap-a.ogg)
    UI_CONFIRM,      ///< Confirmar acción / iniciar carrera / abrir modal (click-a.ogg)
    UI_BACK,         ///< Volver / cancelar / cerrar modal con ESC (click-b.ogg)
    UI_SWITCH,       ///< Cambiar ajuste o rotar ítem en carrusel (switch-a.ogg)
    UI_TAB,          ///< Cambiar pestaña activa en Showroom (switch-b.ogg)
    UI_PAUSE,        ///< Abrir menú de pausa en carrera (tap-b.ogg)
    COUNT
};

} // namespace audio
