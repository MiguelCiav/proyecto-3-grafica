#include "audio/AudioManager.hpp"
#include "ui/UIUtils.hpp"

#include <miniaudio.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <string>

namespace audio {

namespace {
constexpr size_t kSoundCount = static_cast<size_t>(SoundEvent::COUNT);

constexpr std::array<const char*, kSoundCount> kSoundRelativePaths = {
    // UI SFX (Kenney UI Pack)
    "assets/kenney_ui-pack/Sounds/tap-a.ogg",                 // UI_NAVIGATE
    "assets/kenney_ui-pack/Sounds/click-a.ogg",               // UI_CONFIRM
    "assets/kenney_ui-pack/Sounds/click-b.ogg",               // UI_BACK
    "assets/kenney_ui-pack/Sounds/switch-a.ogg",              // UI_SWITCH
    "assets/kenney_ui-pack/Sounds/switch-b.ogg",              // UI_TAB
    "assets/kenney_ui-pack/Sounds/tap-b.ogg",                 // UI_PAUSE

    // Gameplay SFX (Impact & Boost)
    "assets/sfx/impact_sounds/impactMetal_medium_001.ogg",    // KART_IMPACT_HARD
    "assets/sfx/impact_sounds/impactGeneric_light_001.ogg",   // KART_IMPACT_SOFT
    "assets/sfx/impact_sounds/impactBell_heavy_001.ogg"       // KART_BOOST
};

constexpr const char* kEngineLoopRelativePath = "assets/sfx/ENGINE_LOOP.mp3";
constexpr const char* kWheelDriftRelativePath = "assets/sfx/WHEEL_DRIFT.mp3";
constexpr const char* kMenuMusicRelativePath = "assets/sfx/MENU_SONG.mp3";
} // namespace

struct AudioManager::Impl {
    ma_context nullContext{};
    bool hasNullContext{false};

    ma_engine engine{};
    bool engineInitialized{false};

    std::array<ma_sound, kSoundCount> sounds{};
    std::array<bool, kSoundCount> soundLoaded{};
    std::array<uint64_t, kSoundCount> playCounts{};

    ma_sound engineSound{};
    bool engineSoundLoaded{false};
    bool engineSoundPlaying{false};

    ma_sound driftSound{};
    bool driftSoundLoaded{false};
    bool driftSoundPlaying{false};

    ma_sound menuMusic{};
    bool menuMusicLoaded{false};
    bool menuMusicPlaying{false};

    ma_sound raceMusic{};
    bool raceMusicLoaded{false};
    bool raceMusicPlaying{false};
    std::string currentRaceMusicPath{};
    float raceMusicVolume{0.55f};

    float masterVolume{1.0f};
    bool muted{false};
};

AudioManager::AudioManager()
    : m_impl(std::make_unique<Impl>()) {
}

AudioManager::~AudioManager() {
    shutdown();
}

AudioManager::AudioManager(AudioManager&&) noexcept = default;
AudioManager& AudioManager::operator=(AudioManager&&) noexcept = default;

bool AudioManager::init() {
    if (!m_impl) {
        m_impl = std::make_unique<Impl>();
    }

    if (m_impl->engineInitialized) {
        shutdown();
    }

    ma_result result = ma_engine_init(nullptr, &m_impl->engine);
    if (result != MA_SUCCESS) {
        std::cout << "[WARN] [AudioManager] No se detecto dispositivo de audio activo. Inicializando ma_backend_null..." << std::endl;
        ma_backend nullBackend = ma_backend_null;
        if (ma_context_init(&nullBackend, 1, nullptr, &m_impl->nullContext) == MA_SUCCESS) {
            m_impl->hasNullContext = true;
            ma_engine_config engineConfig = ma_engine_config_init();
            engineConfig.pContext = &m_impl->nullContext;
            result = ma_engine_init(&engineConfig, &m_impl->engine);
        }

        if (result != MA_SUCCESS) {
            ma_engine_config noDevConfig = ma_engine_config_init();
            noDevConfig.noDevice = MA_TRUE;
            noDevConfig.channels = 2;
            noDevConfig.sampleRate = 44100;
            result = ma_engine_init(&noDevConfig, &m_impl->engine);
        }
    }

    if (result != MA_SUCCESS) {
        std::cerr << "[ERROR] [AudioManager] Fallo critico al inicializar ma_engine (codigo " << result << ")." << std::endl;
        if (m_impl->hasNullContext) {
            ma_context_uninit(&m_impl->nullContext);
            m_impl->hasNullContext = false;
        }
        return false;
    }

    m_impl->engineInitialized = true;
    ma_engine_set_volume(&m_impl->engine, m_impl->muted ? 0.0f : m_impl->masterVolume);

    constexpr ma_uint32 soundFlags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION;
    size_t loadedCount = 0;

    for (size_t i = 0; i < kSoundCount; ++i) {
        m_impl->soundLoaded[i] = false;
        m_impl->playCounts[i] = 0;

        std::string resolvedPath = ui::resolveAssetPath(kSoundRelativePaths[i]);
        ma_result sndRes = ma_sound_init_from_file(
            &m_impl->engine,
            resolvedPath.c_str(),
            soundFlags,
            nullptr,
            nullptr,
            &m_impl->sounds[i]
        );

        if (sndRes == MA_SUCCESS) {
            m_impl->soundLoaded[i] = true;
            ++loadedCount;
        } else {
            std::cerr << "[WARN] [AudioManager] No se pudo cargar el SFX: " << resolvedPath
                      << " (codigo " << sndRes << ")" << std::endl;
        }
    }

    // Cargar loop del motor (ENGINE_LOOP.mp3)
    m_impl->engineSoundLoaded = false;
    m_impl->engineSoundPlaying = false;
    {
        std::string resolvedEngine = ui::resolveAssetPath(kEngineLoopRelativePath);
        ma_result engRes = ma_sound_init_from_file(
            &m_impl->engine,
            resolvedEngine.c_str(),
            soundFlags,
            nullptr,
            nullptr,
            &m_impl->engineSound
        );
        if (engRes == MA_SUCCESS) {
            ma_sound_set_looping(&m_impl->engineSound, MA_TRUE);
            m_impl->engineSoundLoaded = true;
        } else {
            std::cerr << "[WARN] [AudioManager] No se pudo cargar ENGINE_LOOP.mp3: " << resolvedEngine
                      << " (codigo " << engRes << ")" << std::endl;
        }
    }

    // Cargar loop de derrape de neumaticos (WHEEL_DRIFT.mp3)
    m_impl->driftSoundLoaded = false;
    m_impl->driftSoundPlaying = false;
    {
        std::string resolvedDrift = ui::resolveAssetPath(kWheelDriftRelativePath);
        ma_result driftRes = ma_sound_init_from_file(
            &m_impl->engine,
            resolvedDrift.c_str(),
            soundFlags,
            nullptr,
            nullptr,
            &m_impl->driftSound
        );
        if (driftRes == MA_SUCCESS) {
            ma_sound_set_looping(&m_impl->driftSound, MA_TRUE);
            m_impl->driftSoundLoaded = true;
        } else {
            std::cerr << "[WARN] [AudioManager] No se pudo cargar WHEEL_DRIFT.mp3: " << resolvedDrift
                      << " (codigo " << driftRes << ")" << std::endl;
        }
    }

    // Cargar musica de menu (MENU_SONG.mp3)
    m_impl->menuMusicLoaded = false;
    m_impl->menuMusicPlaying = false;
    {
        std::string resolvedMusic = ui::resolveAssetPath(kMenuMusicRelativePath);
        ma_result musicRes = ma_sound_init_from_file(
            &m_impl->engine,
            resolvedMusic.c_str(),
            MA_SOUND_FLAG_NO_SPATIALIZATION,
            nullptr,
            nullptr,
            &m_impl->menuMusic
        );
        if (musicRes == MA_SUCCESS) {
            ma_sound_set_looping(&m_impl->menuMusic, MA_TRUE);
            m_impl->menuMusicLoaded = true;
        } else {
            std::cerr << "[WARN] [AudioManager] No se pudo cargar MENU_SONG.mp3: " << resolvedMusic
                      << " (codigo " << musicRes << ")" << std::endl;
        }
    }

    std::cout << "[OK] [AudioManager] Subsistema de audio inicializado ("
              << loadedCount << "/" << kSoundCount << " SFX discretos, Motor="
              << (m_impl->engineSoundLoaded ? "OK" : "NO") << ", Drift="
              << (m_impl->driftSoundLoaded ? "OK" : "NO") << ", MenuMusic="
              << (m_impl->menuMusicLoaded ? "OK" : "NO") << ")." << std::endl;
    return true;
}

void AudioManager::shutdown() {
    if (!m_impl) {
        return;
    }

    if (m_impl->engineInitialized) {
        stopAllGameplaySounds();

        if (m_impl->menuMusicLoaded) {
            ma_sound_uninit(&m_impl->menuMusic);
            m_impl->menuMusicLoaded = false;
            m_impl->menuMusicPlaying = false;
        }

        if (m_impl->raceMusicLoaded) {
            ma_sound_uninit(&m_impl->raceMusic);
            m_impl->raceMusicLoaded = false;
            m_impl->raceMusicPlaying = false;
            m_impl->currentRaceMusicPath.clear();
        }

        if (m_impl->engineSoundLoaded) {
            ma_sound_uninit(&m_impl->engineSound);
            m_impl->engineSoundLoaded = false;
            m_impl->engineSoundPlaying = false;
        }

        if (m_impl->driftSoundLoaded) {
            ma_sound_uninit(&m_impl->driftSound);
            m_impl->driftSoundLoaded = false;
            m_impl->driftSoundPlaying = false;
        }

        for (size_t i = 0; i < kSoundCount; ++i) {
            if (m_impl->soundLoaded[i]) {
                ma_sound_uninit(&m_impl->sounds[i]);
                m_impl->soundLoaded[i] = false;
            }
        }
        ma_engine_uninit(&m_impl->engine);
        m_impl->engineInitialized = false;
    }

    if (m_impl->hasNullContext) {
        ma_context_uninit(&m_impl->nullContext);
        m_impl->hasNullContext = false;
    }
}

void AudioManager::playSound(SoundEvent event, float volume, float pitch) {
    if (!m_impl || !m_impl->engineInitialized) {
        return;
    }

    const size_t idx = static_cast<size_t>(event);
    if (idx >= kSoundCount) {
        return;
    }

    ++m_impl->playCounts[idx];

    if (m_impl->muted || !m_impl->soundLoaded[idx]) {
        return;
    }

    ma_sound& sound = m_impl->sounds[idx];
    ma_sound_set_volume(&sound, std::clamp(volume, 0.0f, 2.0f));
    ma_sound_set_pitch(&sound, std::max(0.1f, pitch));
    ma_sound_seek_to_pcm_frame(&sound, 0);
    ma_sound_start(&sound);
}

void AudioManager::setEngineSoundState(bool active, float speedRatio, bool isBoosting) {
    if (!m_impl || !m_impl->engineInitialized || !m_impl->engineSoundLoaded) {
        return;
    }

    if (!active || m_impl->muted) {
        if (m_impl->engineSoundPlaying) {
            ma_sound_stop(&m_impl->engineSound);
            m_impl->engineSoundPlaying = false;
        }
        return;
    }

    const float clampedRatio = std::clamp(speedRatio, 0.0f, 1.5f);
    const float pitch = isBoosting ? 1.65f : (0.75f + clampedRatio * 0.70f);
    const float vol   = isBoosting ? 0.95f : (0.40f + clampedRatio * 0.45f);

    ma_sound_set_pitch(&m_impl->engineSound, pitch);
    ma_sound_set_volume(&m_impl->engineSound, vol);

    if (!m_impl->engineSoundPlaying) {
        ma_sound_start(&m_impl->engineSound);
        m_impl->engineSoundPlaying = true;
    }
}

void AudioManager::setDriftSoundState(bool active, float volume) {
    if (!m_impl || !m_impl->engineInitialized || !m_impl->driftSoundLoaded) {
        return;
    }

    if (active && !m_impl->muted) {
        ma_sound_set_volume(&m_impl->driftSound, std::clamp(volume * 1.5f, 0.0f, 3.0f));
        if (!m_impl->driftSoundPlaying) {
            ma_sound_seek_to_pcm_frame(&m_impl->driftSound, 0);
            ma_sound_start(&m_impl->driftSound);
            m_impl->driftSoundPlaying = true;
        }
    } else {
        if (m_impl->driftSoundPlaying) {
            ma_sound_stop(&m_impl->driftSound);
            ma_sound_seek_to_pcm_frame(&m_impl->driftSound, 0);
            m_impl->driftSoundPlaying = false;
        }
    }
}

void AudioManager::setMenuMusicActive(bool active, float volume) {
    if (!m_impl || !m_impl->engineInitialized || !m_impl->menuMusicLoaded) {
        return;
    }

    if (active && !m_impl->muted) {
        ma_sound_set_volume(&m_impl->menuMusic, std::clamp(volume, 0.0f, 1.5f));
        if (!m_impl->menuMusicPlaying) {
            ma_sound_start(&m_impl->menuMusic);
            m_impl->menuMusicPlaying = true;
        }
    } else {
        if (m_impl->menuMusicPlaying) {
            ma_sound_stop(&m_impl->menuMusic);
            m_impl->menuMusicPlaying = false;
        }
    }
}

bool AudioManager::isMenuMusicPlaying() const {
    return m_impl && m_impl->menuMusicPlaying;
}

void AudioManager::playRaceMusic(const std::string& musicPath, float volume, bool loop) {
    if (!m_impl || !m_impl->engineInitialized || musicPath.empty()) {
        return;
    }

    m_impl->raceMusicVolume = std::clamp(volume, 0.0f, 1.5f);

    // Si ya está cargada la misma pista musical
    if (m_impl->raceMusicLoaded && m_impl->currentRaceMusicPath == musicPath) {
        if (!m_impl->muted) {
            ma_sound_set_volume(&m_impl->raceMusic, m_impl->raceMusicVolume);
        }
        if (!m_impl->raceMusicPlaying && !m_impl->muted) {
            ma_sound_start(&m_impl->raceMusic);
            m_impl->raceMusicPlaying = true;
        }
        return;
    }

    // Si había otra pista de carrera cargada, liberarla
    if (m_impl->raceMusicLoaded) {
        ma_sound_uninit(&m_impl->raceMusic);
        m_impl->raceMusicLoaded = false;
        m_impl->raceMusicPlaying = false;
        m_impl->currentRaceMusicPath.clear();
    }

    std::string resolved = ui::resolveAssetPath(musicPath);
    ma_result res = ma_sound_init_from_file(
        &m_impl->engine,
        resolved.c_str(),
        MA_SOUND_FLAG_NO_SPATIALIZATION,
        nullptr,
        nullptr,
        &m_impl->raceMusic
    );

    if (res != MA_SUCCESS) {
        std::cerr << "[WARN] [AudioManager] No se pudo cargar musica de carrera: " << resolved
                  << " (codigo " << res << ")" << std::endl;
        return;
    }

    m_impl->raceMusicLoaded = true;
    m_impl->currentRaceMusicPath = musicPath;
    ma_sound_set_looping(&m_impl->raceMusic, loop ? MA_TRUE : MA_FALSE);
    ma_sound_set_volume(&m_impl->raceMusic, m_impl->muted ? 0.0f : m_impl->raceMusicVolume);

    if (!m_impl->muted) {
        ma_sound_start(&m_impl->raceMusic);
        m_impl->raceMusicPlaying = true;
    }
}

void AudioManager::pauseRaceMusic() {
    if (!m_impl || !m_impl->engineInitialized || !m_impl->raceMusicLoaded) {
        return;
    }
    if (m_impl->raceMusicPlaying) {
        ma_sound_stop(&m_impl->raceMusic);
        m_impl->raceMusicPlaying = false;
    }
}

void AudioManager::resumeRaceMusic() {
    if (!m_impl || !m_impl->engineInitialized || !m_impl->raceMusicLoaded) {
        return;
    }
    if (!m_impl->raceMusicPlaying && !m_impl->muted) {
        ma_sound_start(&m_impl->raceMusic);
        m_impl->raceMusicPlaying = true;
    }
}

void AudioManager::stopRaceMusic() {
    if (!m_impl || !m_impl->engineInitialized || !m_impl->raceMusicLoaded) {
        return;
    }
    ma_sound_uninit(&m_impl->raceMusic);
    m_impl->raceMusicLoaded = false;
    m_impl->raceMusicPlaying = false;
    m_impl->currentRaceMusicPath.clear();
}

bool AudioManager::isRaceMusicPlaying() const {
    return m_impl && m_impl->raceMusicPlaying;
}

bool AudioManager::isRaceMusicLoaded() const noexcept {
    return m_impl && m_impl->raceMusicLoaded;
}

const std::string& AudioManager::getCurrentRaceMusicPath() const noexcept {
    static const std::string s_empty;
    return m_impl ? m_impl->currentRaceMusicPath : s_empty;
}

void AudioManager::stopAllGameplaySounds() {
    setEngineSoundState(false);
    setDriftSoundState(false);
}

void AudioManager::setMasterVolume(float volume) {
    if (!m_impl) {
        return;
    }
    m_impl->masterVolume = std::clamp(volume, 0.0f, 1.0f);
    if (m_impl->engineInitialized && !m_impl->muted) {
        ma_engine_set_volume(&m_impl->engine, m_impl->masterVolume);
    }
}

float AudioManager::getMasterVolume() const {
    return m_impl ? m_impl->masterVolume : 0.0f;
}

void AudioManager::setMuted(bool muted) {
    if (!m_impl) {
        return;
    }
    m_impl->muted = muted;
    if (m_impl->engineInitialized) {
        ma_engine_set_volume(&m_impl->engine, m_impl->muted ? 0.0f : m_impl->masterVolume);
        if (m_impl->muted) {
            stopAllGameplaySounds();
            if (m_impl->menuMusicPlaying) {
                ma_sound_stop(&m_impl->menuMusic);
                m_impl->menuMusicPlaying = false;
            }
            if (m_impl->raceMusicPlaying) {
                ma_sound_stop(&m_impl->raceMusic);
                m_impl->raceMusicPlaying = false;
            }
        }
    }
}

bool AudioManager::isMuted() const {
    return m_impl ? m_impl->muted : true;
}

bool AudioManager::isInitialized() const noexcept {
    return m_impl && m_impl->engineInitialized;
}

bool AudioManager::isSoundLoaded(SoundEvent event) const noexcept {
    if (!m_impl) {
        return false;
    }
    const size_t idx = static_cast<size_t>(event);
    return (idx < kSoundCount) && m_impl->soundLoaded[idx];
}

bool AudioManager::areAllSoundsLoaded() const noexcept {
    if (!m_impl || !m_impl->engineInitialized) {
        return false;
    }
    for (bool loaded : m_impl->soundLoaded) {
        if (!loaded) {
            return false;
        }
    }
    return true;
}

bool AudioManager::isEngineSoundLoaded() const noexcept {
    return m_impl && m_impl->engineSoundLoaded;
}

bool AudioManager::isDriftSoundLoaded() const noexcept {
    return m_impl && m_impl->driftSoundLoaded;
}

bool AudioManager::isMenuMusicLoaded() const noexcept {
    return m_impl && m_impl->menuMusicLoaded;
}

bool AudioManager::isEngineSoundPlaying() const noexcept {
    return m_impl && m_impl->engineSoundPlaying;
}

bool AudioManager::isDriftSoundPlaying() const noexcept {
    return m_impl && m_impl->driftSoundPlaying;
}

uint64_t AudioManager::getPlayCount(SoundEvent event) const noexcept {
    if (!m_impl) {
        return 0;
    }
    const size_t idx = static_cast<size_t>(event);
    return (idx < kSoundCount) ? m_impl->playCounts[idx] : 0;
}

void AudioManager::resetPlayCounts() noexcept {
    if (m_impl) {
        m_impl->playCounts.fill(0);
    }
}

} // namespace audio
