#pragma once

#include "audio/IAudioService.hpp"
#include <cstdint>
#include <memory>

namespace audio {

/**
 * @brief Gestor de audio concreto basado en miniaudio + stb_vorbis.
 *
 * Utiliza el Idiom Pimpl para ocultar por completo las cabeceras de C de miniaudio
 * de las unidades de traducción del motor y la interfaz.
 * Precarga los efectos de sonido de interfaz y gameplay en memoria PCM
 * para reproducción de baja latencia, gestiona los loops modulados de motor
 * y derrape de neumáticos, y soporta fallback automático a ma_backend_null
 * en entornos headless/CI sin hardware de sonido.
 */
class AudioManager final : public IAudioService {
public:
    AudioManager();
    ~AudioManager() override;

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    AudioManager(AudioManager&&) noexcept;
    AudioManager& operator=(AudioManager&&) noexcept;

    /**
     * @brief Inicializa el motor miniaudio y precarga los bancos de SFX y loops de vehículo.
     * @return true si el motor se inicializó (con dispositivo real o backend nulo).
     */
    bool init();

    /**
     * @brief Libera todos los sonidos cargados y apaga el motor de audio de forma ordenada.
     */
    void shutdown();

    // Implementación de IAudioService
    void playSound(SoundEvent event, float volume = 1.0f, float pitch = 1.0f) override;
    void setEngineSoundState(bool active, float speedRatio = 0.0f, bool isBoosting = false) override;
    void setDriftSoundState(bool active, float volume = 1.0f) override;
    void setMenuMusicActive(bool active, float volume = 0.85f) override;
    [[nodiscard]] bool isMenuMusicPlaying() const override;
    void playRaceMusic(const std::string& musicPath, float volume = 0.85f, bool loop = true) override;
    void pauseRaceMusic() override;
    void resumeRaceMusic() override;
    void stopRaceMusic() override;
    [[nodiscard]] bool isRaceMusicPlaying() const override;
    void stopAllGameplaySounds() override;
    void setMasterVolume(float volume) override;
    [[nodiscard]] float getMasterVolume() const override;
    void setMuted(bool muted) override;
    [[nodiscard]] bool isMuted() const override;

    // Consultas de estado y diagnóstico para pruebas automatizadas
    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]] bool isSoundLoaded(SoundEvent event) const noexcept;
    [[nodiscard]] bool areAllSoundsLoaded() const noexcept;
    [[nodiscard]] bool isEngineSoundLoaded() const noexcept;
    [[nodiscard]] bool isDriftSoundLoaded() const noexcept;
    [[nodiscard]] bool isMenuMusicLoaded() const noexcept;
    [[nodiscard]] bool isRaceMusicLoaded() const noexcept;
    [[nodiscard]] bool isEngineSoundPlaying() const noexcept;
    [[nodiscard]] bool isDriftSoundPlaying() const noexcept;
    [[nodiscard]] const std::string& getCurrentRaceMusicPath() const noexcept;
    [[nodiscard]] uint64_t getPlayCount(SoundEvent event) const noexcept;
    void resetPlayCounts() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace audio
