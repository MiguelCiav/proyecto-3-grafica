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
 * Precarga los efectos de sonido de interfaz (Kenney UI Pack .ogg) en memoria PCM
 * para reproducción de baja latencia y soporta fallback automático a ma_backend_null
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
     * @brief Inicializa el motor miniaudio y precarga el banco de SFX de interfaz.
     * @return true si el motor se inicializó (con dispositivo real o backend nulo).
     */
    bool init();

    /**
     * @brief Libera todos los sonidos cargados y apaga el motor de audio de forma ordenada.
     */
    void shutdown();

    // Implementación de IAudioService
    void playSound(SoundEvent event, float volume = 1.0f, float pitch = 1.0f) override;
    void setMasterVolume(float volume) override;
    [[nodiscard]] float getMasterVolume() const override;
    void setMuted(bool muted) override;
    [[nodiscard]] bool isMuted() const override;

    // Consultas de estado y diagnóstico para pruebas automatizadas
    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]] bool isSoundLoaded(SoundEvent event) const noexcept;
    [[nodiscard]] bool areAllSoundsLoaded() const noexcept;
    [[nodiscard]] uint64_t getPlayCount(SoundEvent event) const noexcept;
    void resetPlayCounts() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace audio
