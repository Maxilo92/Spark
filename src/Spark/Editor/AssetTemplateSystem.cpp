#include "AssetTemplateSystem.h"
#include "Log.h"
#include <fstream>
#include <iomanip>

namespace Spark {

    std::filesystem::path AssetTemplateSystem::CreateLuaScript(const std::filesystem::path& directory, const std::string& name) {
        std::filesystem::path filePath = directory / name;
        if (filePath.extension() != ".lua") {
            filePath += ".lua";
        }

        std::ofstream file(filePath);
        if (!file.is_open()) {
            SP_ERROR("Failed to create Lua script: " + filePath.string());
            return "";
        }

        file << "-- " << name << "\n\n";
        file << "function OnCreate()\n";
        file << "    -- Called when the entity is created\n";
        file << "end\n\n";
        file << "function OnUpdate(dt)\n";
        file << "    -- Called every frame\n";
        file << "end\n";
        
        file.close();
        SP_INFO("Created Lua script: " + filePath.string());
        return filePath;
    }

    std::filesystem::path AssetTemplateSystem::CreateScene(const std::filesystem::path& directory, const std::string& name) {
        std::filesystem::path filePath = directory / name;
        if (filePath.extension() != ".scene") {
            filePath += ".scene";
        }

        std::ofstream file(filePath);
        if (!file.is_open()) {
            SP_ERROR("Failed to create Scene file: " + filePath.string());
            return "";
        }

        file << "Scene: Untitled\n";
        file << "Entities:\n";
        
        file.close();
        SP_INFO("Created Scene file: " + filePath.string());
        return filePath;
    }

    std::filesystem::path AssetTemplateSystem::CreateTextFile(const std::filesystem::path& directory, const std::string& name) {
        std::filesystem::path filePath = directory / name;
        
        std::ofstream file(filePath);
        if (!file.is_open()) {
            SP_ERROR("Failed to create text file: " + filePath.string());
            return "";
        }

        file << ""; // Empty file
        
        file.close();
        SP_INFO("Created text file: " + filePath.string());
        return filePath;
    }

    std::string AssetTemplateSystem::GetUniqueName(const std::filesystem::path& directory, const std::string& baseName, const std::string& extension) {
        std::string name = baseName + extension;
        if (!std::filesystem::exists(directory / name)) {
            return name;
        }

        int counter = 1;
        while (std::filesystem::exists(directory / (baseName + " (" + std::to_string(counter) + ")" + extension))) {
            counter++;
        }
        return baseName + " (" + std::to_string(counter) + ")" + extension;
    }

}
