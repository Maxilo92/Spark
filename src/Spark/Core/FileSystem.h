#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace Spark {

    class FileSystem {
    public:
        static void Init();
        
        static void Mount(const std::string& virtualPath, const std::filesystem::path& physicalPath);
        static void Unmount(const std::string& virtualPath);

        static std::filesystem::path ResolvePhysicalPath(const std::string& virtualPath);
        static bool Exists(const std::string& virtualPath);
        
        static std::vector<char> ReadFile(const std::string& virtualPath);
        static std::string ReadFileText(const std::string& virtualPath);

    private:
        static std::map<std::string, std::filesystem::path> s_MountPoints;
    };

}
