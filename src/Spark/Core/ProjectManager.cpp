#include "ProjectManager.h"
#include "AssetManager.h"
#include "FileSystem.h"
#include "Log.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>

namespace Spark {

    ProjectConfig ProjectManager::s_ActiveProject;
    bool ProjectManager::s_HasActiveProject = false;
    std::filesystem::path ProjectManager::s_ActiveProjectPath;
    std::vector<RecentProject> ProjectManager::s_RecentProjects;

    bool ProjectManager::NewProject(const std::string& name, const std::filesystem::path& location) {
        SP_INFO("Creating new project: " + name + " at " + location.string());
        
        try {
            if (!std::filesystem::exists(location)) {
                std::filesystem::create_directories(location);
            }

            std::filesystem::create_directories(location / "assets/scenes");
            std::filesystem::create_directories(location / "assets/scripts");
            
            // Create project manifest (.spark)
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "Project" << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << name;
            out << YAML::Key << "Version" << YAML::Value << "1.0.0";
            out << YAML::EndMap;
            out << YAML::EndMap;

            std::string manifestPath = (location / (name + ".spark")).string();
            std::ofstream fout(manifestPath);
            fout << out.c_str();
            fout.close();

            // Scaffolding default scene
            std::ofstream sceneOut(location / "assets/scenes/Example.scene");
            sceneOut << "Scene: Untitled\nEntities: []\n";
            sceneOut.close();

            AddToRecentProjects(name, manifestPath);
            return OpenProject(manifestPath);

        } catch (const std::exception& e) {
            SP_ERROR("Failed to create project: " + std::string(e.what()));
            return false;
        }
    }

    bool ProjectManager::OpenProject(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            SP_ERROR("Project file not found: " + path.string());
            return false;
        }

        try {
            YAML::Node data = YAML::LoadFile(path.string());
            if (!data["Project"]) return false;

            s_ActiveProject.Name = data["Project"]["Name"].as<std::string>();
            s_ActiveProject.Version = data["Project"]["Version"].as<std::string>();
            if (data["Project"]["StartScene"])
                s_ActiveProject.StartScene = data["Project"]["StartScene"].as<std::string>();
            else
                s_ActiveProject.StartScene = "assets/scenes/Example.scene";

            s_ActiveProjectPath = path;
            s_HasActiveProject = true;

            // Set FileSystem root to project directory
            FileSystem::SetProjectRoot(path.parent_path());

            // Re-init Asset Manager for new project
            AssetManager::Shutdown();
            AssetManager::Init();

            AddToRecentProjects(s_ActiveProject.Name, path.string());
            SP_INFO("Project opened: " + s_ActiveProject.Name);
            return true;
        } catch (const std::exception& e) {
            SP_ERROR("Failed to open project: " + std::string(e.what()));
            return false;
        }
    }

    bool ProjectManager::SaveProject() {
        if (!s_HasActiveProject) return false;

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Project" << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << s_ActiveProject.Name;
        out << YAML::Key << "Version" << YAML::Value << s_ActiveProject.Version;
        out << YAML::Key << "StartScene" << YAML::Value << s_ActiveProject.StartScene;
        out << YAML::EndMap;
        out << YAML::EndMap;

        std::ofstream fout(s_ActiveProjectPath);
        fout << out.c_str();
        fout.close();
        return true;
    }

    void ProjectManager::RefreshRecentProjects() {
        s_RecentProjects.clear();
        if (!std::filesystem::exists("recent_projects.yaml")) return;

        try {
            YAML::Node data = YAML::LoadFile("recent_projects.yaml");
            auto projects = data["RecentProjects"];
            if (projects) {
                for (auto p : projects) {
                    RecentProject rp;
                    rp.Name = p["Name"].as<std::string>();
                    rp.Path = p["Path"].as<std::string>();
                    rp.LastOpened = p["LastOpened"].as<std::string>();
                    s_RecentProjects.push_back(rp);
                }
            }
        } catch (const std::exception& e) {
            SP_ERROR("Failed to load recent projects: " + std::string(e.what()));
        }
    }

    void ProjectManager::RemoveRecentProject(const std::string& path) {
        s_RecentProjects.erase(
            std::remove_if(s_RecentProjects.begin(), s_RecentProjects.end(),
                [&](const RecentProject& rp) { return rp.Path == path; }),
            s_RecentProjects.end());
        SaveRecentProjects();
    }

    void ProjectManager::AddToRecentProjects(const std::string& name, const std::string& path) {
        // Remove if exists
        RemoveRecentProject(path);

        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        char buf[100];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now_c));

        RecentProject rp;
        rp.Name = name;
        rp.Path = path;
        rp.LastOpened = buf;
        
        s_RecentProjects.insert(s_RecentProjects.begin(), rp);
        if (s_RecentProjects.size() > 10) s_RecentProjects.pop_back();

        SaveRecentProjects();
    }

    void ProjectManager::SaveRecentProjects() {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "RecentProjects" << YAML::Value << YAML::BeginSeq;
        for (const auto& rp : s_RecentProjects) {
            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << rp.Name;
            out << YAML::Key << "Path" << YAML::Value << rp.Path;
            out << YAML::Key << "LastOpened" << YAML::Value << rp.LastOpened;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout("recent_projects.yaml");
        fout << out.c_str();
        fout.close();
    }

}
