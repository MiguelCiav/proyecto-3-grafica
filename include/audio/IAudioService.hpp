#pragma once

#include "audio/SoundEvent.hpp"
#include <algorithm>

namespace audio {

/**
 * @brief Interfaz abstracta del servicio de reproducción de audio.
 */
class IAudioService {
public:
    virtual ~IAudioService() = default;

    /**
     * @brief Reproduce un efecto de sonido discreto (UI o colisión/boost).
     * @param event Identificador del evento sonoro.
     * @param volume Volumen relativo [0.0f, 1.0f].
     * @param pitch Tono / velocidad de reproducción (> 0.0f, por defecto 1.0f).
     */
    virtual void playSound(SoundEvent event, float volume = 1.0f, float pitch = 1.0f) = 0;

    /**
     * @brief Activa, modula (RPM/pitch y volumen según velocidad y turbo) o detiene el loop del motor.
     */
    virtual void setEngineSoundState(bool active, float speedRatio = 0.0f, bool isBoosting = false) = 0;

    /**
     * @brief Activa o corta instantáneamente el sonido de derrape de neumáticos (WHEEL_DRIFT.mp3).
     */
    virtual void setDriftSoundState(bool active, float volume = 1.0f) = 0;

    /**
     * @brief Activa o desactiva la música de fondo del menú principal y showroom.
     * @param active Si es true reproduce/reanuda la música, si es false la detiene.
     * @param volume Volumen relativo de la pista.
     */
    virtual void setMenuMusicActive(bool active, float volume = 0.55f) = 0;

    /**
     * @brief Indica si la música de menú está sonando actualmente.
     */
    [[nodiscard]] virtual bool isMenuMusicPlaying() const = 0;

    /**
     * @brief Detiene todos los sonidos continuos de gameplay (motor y derrape).
     */
    virtual void stopAllGameplaySounds() = 0;

    /**
     * @brief Ajusta el volumen maestro del motor de audio [0.0f, 1.0f].
     */
    virtual void setMasterVolume(float volume) = 0;

    /**
     * @brief Obtiene el volumen maestro actual [0.0f, 1.0f].
     */
    [[nodiscard]] virtual float getMasterVolume() const = 0;

    /**
     * @brief Silencia o reactiva toda la salida de audio.
     */
    virtual void setMuted(bool muted) = 0;

    /**
     * @brief Indica si el sistema de audio está silenciado.
     */
    [[nodiscard]] virtual bool isMuted() const = 0;
};

/**
 * @brief Implementación Null Object de IAudioService para entornos o contextos
 *        donde no se ha inyectado un motor de audio activo.
 */
class NullAudioService final : public IAudioService {
public:
    void playSound(SoundEvent /*event*/, float /*volume*/ = 1.0f, float /*pitch*/ = 1.0f) override {}

    void setEngineSoundState(bool /*active*/, float /*speedRatio*/ = 0.0f, bool /*isBoosting*/ = false) override {}

    void setDriftSoundState(bool /*active*/, float /*volume*/ = 1.0f) override {}

    void setMenuMusicActive(bool active, float /*volume*/ = 0.55f) override {
        m_menuMusicPlaying = active;
    }

    [[nodiscard]] bool isMenuMusicPlaying() const override {
        return m_menuMusicPlaying;
    }

    void stopAllGameplaySounds() override {}

    void setMasterVolume(float volume) override {
        m_volume = std::clamp(volume, 0.0f, 1.0f);
    }

    [[nodiscard]] float getMasterVolume() const override {
        return m_volume;
    }

    void setMuted(bool muted) override {
        m_muted = muted;
    }

    [[nodiscard]] bool isMuted() const override {
        return m_muted;
    }

private:
    float m_volume{1.0f};
    bool m_muted{false};
    bool m_menuMusicPlaying{false};
};

} // namespace audio
