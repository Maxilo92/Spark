#include "FileSystem.h"
#include <fstream>
#include <iostream>

namespace Spark {

    std::map<std::string, std::filesystem::path> FileSystem::s_MountPoints;

    void FileSystem::Init() {
        // Standard-Mounting
        Mount("Assets", "assets");
        Mount("Scripts", "assets/scripts");
        Mount("Scenes", "assets/scenes");
    }

    void FileSystem::Mount(const std::string& virtualPath, const std::filesystem::path& physicalPath) {
        s_MountPoints[virtualPath] = physicalPath;
    }

    void FileSystem::Unmount(const std::string& virtualPath) {
        s_MountPoints.erase(virtualPath);
    }

    std::filesystem::path FileSystem::ResolvePhysicalPath(const std::string& virtualPath) {
        if (virtualPath.substr(0, 2) != "//") return virtualPath;

        size_t slashPos = virtualPath.find('/', 2);
        std::string mountPoint = virtualPath.substr(2, slashPos - 2);
        std::string relativePath = (slashPos == std::string::npos) ? "" : virtualPath.substr(slashPos + 1);

        if (s_MountPoints.find(mountPoint) == s_MountPoints.end()) {
            std::cerr << "VFS: Mount point not found: " << mountPoint << std::endl;
            return virtualPath;
        }

        return s_MountPoints[mountPoint] / relativePath;
    }

    bool FileSystem::Exists(const std::string& virtualPath) {
        return std::filesystem::exists(ResolvePhysicalPath(virtualPath));
    }

    std::vector<char> FileSystem::ReadFile(const std::string& virtualPath) {
        std::filesystem::path physicalPath = ResolvePhysicalPath(virtualPath);
        std::ifstream stream(physicalPath, std::ios::binary | std::ios::ate);

        if (!stream) return {};

        std::streamsize size = stream.tellg();
        stream.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if (!stream.read(buffer.data(), size)) return {};

        return buffer;
    }

    std::string FileSystem::ReadFileText(const std::string& virtualPath) {
        std::filesystem::path physicalPath = ResolvePhysicalPath(virtualPath);
        std::ifstream stream(physicalPath);
        if (!stream) return "";

        std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        return content;
    }

}
