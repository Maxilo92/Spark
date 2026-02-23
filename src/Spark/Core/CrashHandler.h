#pragma once
#include <string>
#include <filesystem>

namespace Spark {

    class CrashHandler {
    public:
        static void Init();
        static void HandleCrash(int signal);

    private:
        static void CreateCrashReport(int signal);
        static void CopyFileSafe(const std::filesystem::path& from, const std::filesystem::path& to);
        static std::string GetSignalName(int signal);
    };

}
