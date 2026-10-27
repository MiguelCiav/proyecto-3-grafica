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
    "assets/kenney_ui-pack/Sounds/tap-a.ogg",    // UI_NAVIGATE
    "assets/kenney_ui-pack/Sounds/click-a.ogg",  // UI_CONFIRM
    "assets/kenney_ui-pack/Sounds/click-b.ogg",  // UI_BACK
    "assets/kenney_ui-pack/Sounds/switch-a.ogg", // UI_SWITCH
    "assets/kenney_ui-pack/Sounds/switch-b.ogg", // UI_TAB
    "assets/kenney_ui-pack/Sounds/tap-b.ogg"     // UI_PAUSE
};
} // namespace

struct AudioManager::Impl {
    ma_context nullContext{};
    bool hasNullContext{false};

    ma_engine engine{};
    bool engineInitialized{false};

    std::array<ma_sound, kSoundCount> sounds{};
    std::array<bool, kSoundCount> soundLoaded{};
    std::array<uint64_t, kSoundCount> playCounts{};

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

    std::cout << "[OK] [AudioManager] Subsistema de audio inicializado ("
              << loadedCount << "/" << kSoundCount << " SFX UI cargados)." << std::endl;
    return true;
}

void AudioManager::shutdown() {
    if (!m_impl) {
        return;
    }

    if (m_impl->engineInitialized) {
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
