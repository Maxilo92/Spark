#include "AudioManager.h"
#include "miniaudio.h"
#include <iostream>

namespace Spark {

    struct AudioManager::AudioManagerData {
        ma_engine Engine;
    };

    std::unique_ptr<AudioManager::AudioManagerData> AudioManager::s_Data = nullptr;

    void AudioManager::Init() {
        s_Data = std::make_unique<AudioManagerData>();
        ma_result result = ma_engine_init(NULL, &s_Data->Engine);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to initialize audio engine!" << std::endl;
            return;
        }
        std::cout << "Audio Manager Initialized (miniaudio)" << std::endl;
    }

    void AudioManager::Shutdown() {
        if (s_Data) {
            ma_engine_uninit(&s_Data->Engine);
            s_Data.reset();
        }
    }

    void AudioManager::PlaySound(const std::string& filepath, bool loop, float volume) {
        if (!s_Data) return;
        
        ma_uint32 flags = 0;
        if (loop) flags |= MA_SOUND_FLAG_LOOPING;

        // Simple play-and-forget for now
        ma_engine_play_sound(&s_Data->Engine, filepath.c_str(), NULL);
    }

    void* AudioManager::GetEngine() {
        return s_Data ? &s_Data->Engine : nullptr;
    }

}
