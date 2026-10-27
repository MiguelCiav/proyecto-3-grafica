#pragma once

#include "audio/SoundEvent.hpp"
#include <algorithm>
#include <string>

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
     * @brief Inicia o cambia la música de fondo de la carrera.
     * @param musicPath Ruta del archivo de audio de la pista.
     * @param volume Nivel de volumen [0.0f, 1.0f].
     * @param loop Si debe reproducirse en bucle continuo.
     */
    virtual void playRaceMusic(const std::string& musicPath, float volume = 0.55f, bool loop = true) = 0;

    /**
     * @brief Pausa la música de carrera actual sin reiniciar el cursor.
     */
    virtual void pauseRaceMusic() = 0;

    /**
     * @brief Reanuda la música de carrera pausada.
     */
    virtual void resumeRaceMusic() = 0;

    /**
     * @brief Detiene y descarga la música de carrera activa.
     */
    virtual void stopRaceMusic() = 0;

    /**
     * @brief Indica si la música de carrera está reproduciéndose actualmente.
     */
    [[nodiscard]] virtual bool isRaceMusicPlaying() const = 0;

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

    void playRaceMusic(const std::string& /*musicPath*/, float /*volume*/ = 0.55f, bool /*loop*/ = true) override {
        m_raceMusicPlaying = true;
    }

    void pauseRaceMusic() override {
        m_raceMusicPlaying = false;
    }

    void resumeRaceMusic() override {
        m_raceMusicPlaying = true;
    }

    void stopRaceMusic() override {
        m_raceMusicPlaying = false;
    }

    [[nodiscard]] bool isRaceMusicPlaying() const override {
        return m_raceMusicPlaying;
    }

    void stopAllGameplaySounds() override {
        m_raceMusicPlaying = false;
    }

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
    bool m_raceMusicPlaying{false};
};

} // namespace audio
