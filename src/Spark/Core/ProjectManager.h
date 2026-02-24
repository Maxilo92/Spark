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

    struct RecentProject {
        std::string Name;
        std::string Path;
        std::string LastOpened;
    };

    class ProjectManager {
    public:
        static bool NewProject(const std::string& name, const std::filesystem::path& location);
        static bool OpenProject(const std::filesystem::path& path);
        static bool SaveProject();

        static const ProjectConfig& GetActiveProject() { return s_ActiveProject; }
        static const std::filesystem::path& GetActiveProjectPath() { return s_ActiveProjectPath; }
        static bool HasActiveProject() { return s_HasActiveProject; }

        static const std::vector<RecentProject>& GetRecentProjects() { return s_RecentProjects; }
        static void RefreshRecentProjects();
        static void RemoveRecentProject(const std::string& path);

    private:
        static void AddToRecentProjects(const std::string& name, const std::string& path);
        static void SaveRecentProjects();

    private:
        static ProjectConfig s_ActiveProject;
        static bool s_HasActiveProject;
        static std::filesystem::path s_ActiveProjectPath;
        static std::vector<RecentProject> s_RecentProjects;
    };

}
