#pragma once
#include <string>
#include <memory>

namespace Spark {

    class AudioManager {
    public:
        static void Init();
        static void Shutdown();

        static void PlaySound(const std::string& filepath, bool loop = false, float volume = 1.0f);
        static void* GetEngine(); // Returns ma_engine*

    private:
        struct AudioManagerData;
        static std::unique_ptr<AudioManagerData> s_Data;
    };

}
