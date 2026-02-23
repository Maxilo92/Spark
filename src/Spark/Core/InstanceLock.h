#pragma once
#include <string>
#include <filesystem>

namespace Spark {

    class InstanceLock {
    public:
        static bool Lock(const std::string& lockFileName = "spark.lock");
        static void Unlock();

    private:
        static std::filesystem::path GetLockFilePath(const std::string& name);
        static void KillProcess(int pid);
        static bool IsProcessRunning(int pid);
    };

}
