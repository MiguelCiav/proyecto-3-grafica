#include <iostream>
#include <cassert>
#include <cmath>

#include "audio/SoundEvent.hpp"
#include "audio/IAudioService.hpp"
#include "audio/AudioManager.hpp"
#include "core/GameContext.hpp"

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST: Validacion del Subsistema de Audio (miniaudio)    " << std::endl;
    std::cout << "==========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // 1. Seguridad de GameContext::getAudio() con NullAudioService (Fallback)
    // -------------------------------------------------------------------------
    {
        core::GameContext ctx;
        assert(ctx.audioService == nullptr && "audioService debe iniciar en nullptr");

        audio::IAudioService& nullSvc = ctx.getAudio();
        assert(!nullSvc.isMuted() && "NullAudioService no debe iniciar muteado");
        assert(std::abs(nullSvc.getMasterVolume() - 1.0f) < 1e-5f);

        // Llamadas seguras sin crash cuando no hay AudioManager inyectado
        nullSvc.playSound(audio::SoundEvent::UI_NAVIGATE);
        nullSvc.playSound(audio::SoundEvent::UI_CONFIRM, 0.8f, 1.1f);
        nullSvc.playSound(audio::SoundEvent::KART_IMPACT_HARD, 0.9f, 1.0f);
        nullSvc.playSound(audio::SoundEvent::KART_IMPACT_SOFT, 0.6f, 1.05f);
        nullSvc.playSound(audio::SoundEvent::KART_BOOST, 0.95f, 1.15f);
        nullSvc.setEngineSoundState(true, 0.7f, false);
        nullSvc.setDriftSoundState(true, 0.8f);
        nullSvc.stopAllGameplaySounds();

        nullSvc.setMasterVolume(0.45f);
        assert(std::abs(nullSvc.getMasterVolume() - 0.45f) < 1e-5f);

        // Clamping de volumen en NullAudioService
        nullSvc.setMasterVolume(2.5f);
        assert(std::abs(nullSvc.getMasterVolume() - 1.0f) < 1e-5f);
        nullSvc.setMasterVolume(-0.5f);
        assert(std::abs(nullSvc.getMasterVolume() - 0.0f) < 1e-5f);

        nullSvc.setMuted(true);
        assert(nullSvc.isMuted());
        nullSvc.setMuted(false);
        assert(!nullSvc.isMuted());
        nullSvc.setMasterVolume(1.0f);

        std::cout << "  [CHECK 1] Fallback seguro NullAudioService en GameContext OK." << std::endl;
    }

    // -------------------------------------------------------------------------
    // 2. Inicializacion de AudioManager, 9 SFX (.ogg) y 2 Loops (.mp3)
    // -------------------------------------------------------------------------
    {
        audio::AudioManager audioMgr;
        assert(!audioMgr.isInitialized() && "AudioManager no debe estar inicializado antes de init()");

        bool initOk = audioMgr.init();
        assert(initOk && "AudioManager::init() debe retornar true (dispositivo real o ma_backend_null)");
        (void)initOk;
        assert(audioMgr.isInitialized());

        // Verificar que los 6 SFX del Kenney UI Pack se hayan cargado
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::UI_NAVIGATE) && "tap-a.ogg debe cargarse");
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::UI_CONFIRM)  && "click-a.ogg debe cargarse");
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::UI_BACK)     && "click-b.ogg debe cargarse");
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::UI_SWITCH)   && "switch-a.ogg debe cargarse");
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::UI_TAB)      && "switch-b.ogg debe cargarse");
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::UI_PAUSE)    && "tap-b.ogg debe cargarse");

        // Verificar que los 3 SFX de Gameplay (colisiones y boost) se hayan cargado
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::KART_IMPACT_HARD) && "impactMetal_medium_001.ogg debe cargarse");
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::KART_IMPACT_SOFT) && "impactGeneric_light_001.ogg debe cargarse");
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::KART_BOOST)       && "impactBell_heavy_001.ogg debe cargarse");
        assert(audioMgr.areAllSoundsLoaded() && "Los 9 sonidos discretos deben estar cargados");

        // Verificar carga de los 2 loops de vehiculo (.mp3)
        assert(audioMgr.isEngineSoundLoaded() && "ENGINE_LOOP.mp3 debe cargarse");
        assert(audioMgr.isDriftSoundLoaded()  && "WHEEL_DRIFT.mp3 debe cargarse");

        std::cout << "  [CHECK 2] Carga de 9 SFX (.ogg) + ENGINE_LOOP.mp3 + WHEEL_DRIFT.mp3 OK." << std::endl;

        // ---------------------------------------------------------------------
        // 3. Disparo de los 9 SoundEvent y conteo de reproducciones
        // ---------------------------------------------------------------------
        core::GameContext ctx;
        ctx.audioService = &audioMgr;
        assert(&ctx.getAudio() == &audioMgr && "GameContext::getAudio() debe retornar AudioManager inyectado");

        const audio::SoundEvent allEvents[] = {
            audio::SoundEvent::UI_NAVIGATE,
            audio::SoundEvent::UI_CONFIRM,
            audio::SoundEvent::UI_BACK,
            audio::SoundEvent::UI_SWITCH,
            audio::SoundEvent::UI_TAB,
            audio::SoundEvent::UI_PAUSE,
            audio::SoundEvent::KART_IMPACT_HARD,
            audio::SoundEvent::KART_IMPACT_SOFT,
            audio::SoundEvent::KART_BOOST
        };

        for (audio::SoundEvent ev : allEvents) {
            assert(audioMgr.getPlayCount(ev) == 0);
            ctx.getAudio().playSound(ev, 0.9f, 1.0f);
            assert(audioMgr.getPlayCount(ev) == 1);
            // Segundo disparo consecutivo para verificar reinicio de cursor PCM (seek a frame 0)
            ctx.getAudio().playSound(ev, 0.75f, 1.05f);
            assert(audioMgr.getPlayCount(ev) == 2);
        }

        std::cout << "  [CHECK 3] Disparo y re-trigger de baja latencia de los 9 SoundEvent OK." << std::endl;

        // ---------------------------------------------------------------------
        // 4. Loop de Motor (ENGINE_LOOP.mp3) y Corte Instantaneo de Derrape (WHEEL_DRIFT.mp3)
        // ---------------------------------------------------------------------
        assert(!audioMgr.isEngineSoundPlaying());
        assert(!audioMgr.isDriftSoundPlaying());

        // Activar y modular motor normal y en turbo
        ctx.getAudio().setEngineSoundState(true, 0.5f, false);
        assert(audioMgr.isEngineSoundPlaying() && "El loop de motor debe estar activo");
        ctx.getAudio().setEngineSoundState(true, 1.0f, true);
        assert(audioMgr.isEngineSoundPlaying() && "El loop de motor debe seguir activo en boost");

        // Detener motor
        ctx.getAudio().setEngineSoundState(false);
        assert(!audioMgr.isEngineSoundPlaying() && "El loop de motor debe detenerse con setEngineSoundState(false)");

        // Activar derrape y verificar corte instantaneo al soltar el derrape
        ctx.getAudio().setDriftSoundState(true, 0.8f);
        assert(audioMgr.isDriftSoundPlaying() && "WHEEL_DRIFT.mp3 debe sonar mientras el derrape esta activo");
        ctx.getAudio().setDriftSoundState(false);
        assert(!audioMgr.isDriftSoundPlaying() && "WHEEL_DRIFT.mp3 debe cortarse inmediatamente al terminar el derrape");

        // Verificar stopAllGameplaySounds() con ambos activos
        ctx.getAudio().setEngineSoundState(true, 0.8f, false);
        ctx.getAudio().setDriftSoundState(true, 0.8f);
        assert(audioMgr.isEngineSoundPlaying() && audioMgr.isDriftSoundPlaying());
        ctx.getAudio().stopAllGameplaySounds();
        assert(!audioMgr.isEngineSoundPlaying() && !audioMgr.isDriftSoundPlaying() &&
               "stopAllGameplaySounds() debe detener motor y derrape simultaneamente");

        std::cout << "  [CHECK 4] Modulacion de ENGINE_LOOP.mp3 y corte instantaneo de WHEEL_DRIFT.mp3 OK." << std::endl;

        // ---------------------------------------------------------------------
        // 5. Control de Volumen Maestro y Mute
        // ---------------------------------------------------------------------
        ctx.getAudio().setMasterVolume(0.65f);
        assert(std::abs(ctx.getAudio().getMasterVolume() - 0.65f) < 1e-5f);

        ctx.getAudio().setMasterVolume(1.8f);
        assert(std::abs(ctx.getAudio().getMasterVolume() - 1.0f) < 1e-5f);

        ctx.getAudio().setMasterVolume(-0.3f);
        assert(std::abs(ctx.getAudio().getMasterVolume() - 0.0f) < 1e-5f);

        ctx.getAudio().setMasterVolume(0.8f);
        assert(!ctx.getAudio().isMuted());

        ctx.getAudio().setEngineSoundState(true, 0.6f, false);
        assert(audioMgr.isEngineSoundPlaying());
        ctx.getAudio().setMuted(true);
        assert(ctx.getAudio().isMuted());
        assert(!audioMgr.isEngineSoundPlaying() && "Mute debe detener sonidos continuos activos");

        // Reproducir estando muteado no debe producir errores
        ctx.getAudio().playSound(audio::SoundEvent::UI_NAVIGATE);

        ctx.getAudio().setMuted(false);
        assert(!ctx.getAudio().isMuted());
        assert(std::abs(ctx.getAudio().getMasterVolume() - 0.8f) < 1e-5f);

        std::cout << "  [CHECK 5] Control de volumen maestro (get/set/clamp) y estado Mute OK." << std::endl;

        // ---------------------------------------------------------------------
        // 6. Cierre ordenado (RAII / shutdown)
        // ---------------------------------------------------------------------
        audioMgr.shutdown();
        assert(!audioMgr.isInitialized() && "shutdown() debe marcar el AudioManager como no inicializado");
        std::cout << "  [CHECK 6] Liberacion ordenada de recursos en AudioManager::shutdown() OK." << std::endl;
    }

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Todos los tests de AudioSystem completados 100%! " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
