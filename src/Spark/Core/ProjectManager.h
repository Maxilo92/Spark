#pragma once
#include <string>
#include <filesystem>
#include <vector>

namespace Spark {

    struct ProjectConfig {
        std::string Name = "Untitled Project";
        std::string Version = "1.0.0";
        std::string StartScene = "assets/scenes/Example.scene";
    };

    class ProjectManager {
    public:
        static bool NewProject(const std::string& name, const std::filesystem::path& location);
        static bool SaveProject(const std::filesystem::path& path);
        static bool LoadProject(const std::filesystem::path& path);

        static const ProjectConfig& GetActiveProject() { return s_ActiveProject; }
        static const std::filesystem::path& GetActiveProjectPath() { return s_ActiveProjectPath; }
        static bool HasActiveProject() { return s_HasActiveProject; }

    private:
        static ProjectConfig s_ActiveProject;
        static bool s_HasActiveProject;
        static std::filesystem::path s_ActiveProjectPath;
    };

}
