#include "ProjectManager.h"
#include "AssetManager.h"
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include "Log.h"
#include <cstdlib>

namespace Spark {

    ProjectConfig ProjectManager::s_ActiveProject;
    bool ProjectManager::s_HasActiveProject = false;
    std::filesystem::path ProjectManager::s_ActiveProjectPath;

    bool ProjectManager::NewProject(const std::string& name, const std::filesystem::path& location) {
        SP_INFO("Creating new project: " + name);
        
        // 1. Release all file handles by shutting down systems
        AssetManager::Shutdown();

        // 2. Backup current assets if they exist
        try {
            if (std::filesystem::exists("assets")) {
                std::string backupName = "assets_backup_" + name;
                if (std::filesystem::exists(backupName)) 
                    std::filesystem::remove_all(backupName);
                std::filesystem::rename("assets", backupName);
            }
        } catch (const std::exception& e) {
            SP_ERROR("Failed to backup assets: " + std::string(e.what()));
            AssetManager::Init(); // Restore
            return false;
        }

        // 3. Create fresh directories
        std::filesystem::create_directories("assets/scenes");
        std::filesystem::create_directories("assets/scripts");
        
        // 4. Create default plan.yaml
        std::ofstream planOut("assets/plan.yaml");
        planOut << "Plan:\n  - Title: Initialize Project\n    Description: Start building " << name << "!\n    Category: General\n    Priority: 1\n    Completed: false\n";
        planOut.close();

        // 5. Re-init Asset Manager for new project
        AssetManager::Init();

        // 6. Update active project state
        s_ActiveProject.Name = name;
        s_ActiveProject.Version = "1.0.0";
        s_ActiveProject.StartScene = "assets/scenes/Example.scene";
        s_ActiveProjectPath = location;
        s_HasActiveProject = true;

        return true;
    }

    bool ProjectManager::SaveProject(const std::filesystem::path& path) {
        SP_INFO("Saving project to: " + path.string());
        
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Project" << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << s_ActiveProject.Name;
        out << YAML::Key << "Version" << YAML::Value << s_ActiveProject.Version;
        out << YAML::EndMap;
        out << YAML::EndMap;

        std::ofstream fout("project_meta.yaml");
        fout << out.c_str();
        fout.close();

        std::string cmd = "zip -r \"" + path.string() + "\" assets project_meta.yaml";
        int result = std::system(cmd.c_str());
        std::filesystem::remove("project_meta.yaml");

        return result == 0;
    }

    bool ProjectManager::LoadProject(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            SP_ERROR("Project file not found: " + path.string());
            return false;
        }

        SP_INFO("Loading project: " + path.string());

        // Release file handles
        AssetManager::Shutdown();

        try {
            if (std::filesystem::exists("assets")) {
                if (std::filesystem::exists("assets_backup")) 
                    std::filesystem::remove_all("assets_backup");
                std::filesystem::rename("assets", "assets_backup");
            }
        } catch (const std::exception& e) {
            SP_ERROR("Failed to backup existing assets: " + std::string(e.what()));
        }

        std::string cmd = "unzip -o \"" + path.string() + "\" -d .";
        int result = std::system(cmd.c_str());

        // Re-init Asset Manager
        AssetManager::Init();

        if (result == 0 && std::filesystem::exists("project_meta.yaml")) {
            YAML::Node data = YAML::LoadFile("project_meta.yaml");
            auto project = data["Project"];
            s_ActiveProject.Name = project["Name"].as<std::string>();
            s_ActiveProject.Version = project["Version"].as<std::string>();
            s_ActiveProjectPath = path;
            s_HasActiveProject = true;
            
            std::filesystem::remove("project_meta.yaml");
            SP_INFO("Project loaded successfully.");
            return true;
        }

        SP_ERROR("Failed to load project.");
        return false;
    }

}
