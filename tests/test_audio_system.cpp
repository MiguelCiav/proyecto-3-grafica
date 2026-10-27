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
    // 2. Inicializacion de AudioManager y carga de los 6 archivos .ogg
    // -------------------------------------------------------------------------
    {
        audio::AudioManager audioMgr;
        assert(!audioMgr.isInitialized() && "AudioManager no debe estar inicializado antes de init()");

        bool initOk = audioMgr.init();
        assert(initOk && "AudioManager::init() debe retornar true (dispositivo real o ma_backend_null)");
        (void)initOk;
        assert(audioMgr.isInitialized());

        // Verificar que los 6 SFX del Kenney UI Pack se hayan decodificado y cargado
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::UI_NAVIGATE) && "tap-a.ogg debe cargarse");
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::UI_CONFIRM)  && "click-a.ogg debe cargarse");
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::UI_BACK)     && "click-b.ogg debe cargarse");
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::UI_SWITCH)   && "switch-a.ogg debe cargarse");
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::UI_TAB)      && "switch-b.ogg debe cargarse");
        assert(audioMgr.isSoundLoaded(audio::SoundEvent::UI_PAUSE)    && "tap-b.ogg debe cargarse");
        assert(audioMgr.areAllSoundsLoaded() && "Los 6 sonidos del banco UI deben estar cargados");

        std::cout << "  [CHECK 2] Inicializacion de AudioManager y carga de 6 archivos .ogg OK." << std::endl;

        // ---------------------------------------------------------------------
        // 3. Disparo de los 6 SoundEvent y conteo de reproducciones
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
            audio::SoundEvent::UI_PAUSE
        };

        for (audio::SoundEvent ev : allEvents) {
            assert(audioMgr.getPlayCount(ev) == 0);
            ctx.getAudio().playSound(ev, 0.9f, 1.0f);
            assert(audioMgr.getPlayCount(ev) == 1);
            // Segundo disparo consecutivo para verificar reinicio de cursor PCM (seek a frame 0)
            ctx.getAudio().playSound(ev, 0.75f, 1.05f);
            assert(audioMgr.getPlayCount(ev) == 2);
        }

        std::cout << "  [CHECK 3] Disparo y re-trigger de baja latencia de los 6 SoundEvent OK." << std::endl;

        // ---------------------------------------------------------------------
        // 4. Control de Volumen Maestro y Mute
        // ---------------------------------------------------------------------
        ctx.getAudio().setMasterVolume(0.65f);
        assert(std::abs(ctx.getAudio().getMasterVolume() - 0.65f) < 1e-5f);

        ctx.getAudio().setMasterVolume(1.8f);
        assert(std::abs(ctx.getAudio().getMasterVolume() - 1.0f) < 1e-5f);

        ctx.getAudio().setMasterVolume(-0.3f);
        assert(std::abs(ctx.getAudio().getMasterVolume() - 0.0f) < 1e-5f);

        ctx.getAudio().setMasterVolume(0.8f);
        assert(!ctx.getAudio().isMuted());

        ctx.getAudio().setMuted(true);
        assert(ctx.getAudio().isMuted());
        // Reproducir estando muteado no debe producir errores
        ctx.getAudio().playSound(audio::SoundEvent::UI_NAVIGATE);

        ctx.getAudio().setMuted(false);
        assert(!ctx.getAudio().isMuted());
        assert(std::abs(ctx.getAudio().getMasterVolume() - 0.8f) < 1e-5f);

        std::cout << "  [CHECK 4] Control de volumen maestro (get/set/clamp) y estado Mute OK." << std::endl;

        // ---------------------------------------------------------------------
        // 5. Cierre ordenado (RAII / shutdown)
        // ---------------------------------------------------------------------
        audioMgr.shutdown();
        assert(!audioMgr.isInitialized() && "shutdown() debe marcar el AudioManager como no inicializado");
        std::cout << "  [CHECK 5] Liberacion ordenada de recursos en AudioManager::shutdown() OK." << std::endl;
    }

    std::cout << "==========================================================" << std::endl;
    std::cout << "  [PASS] Todos los tests de AudioSystem completados 100%! " << std::endl;
    std::cout << "==========================================================" << std::endl;
    return 0;
}
