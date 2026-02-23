#pragma once

#include <filesystem>
#include <string>

namespace Spark {

    /**
     * @brief System for generating boilerplate content for common assets.
     * 
     * Provides helper functions for creating new asset files like Lua scripts,
     * Scene files, and Text files with basic boilerplate code already in place.
     * These methods will handle file creation and log errors via SP_ERROR if the
     * operation fails.
     */
    class AssetTemplateSystem {
    public:
        /**
         * @brief Creates a new Lua script with boilerplate functions.
         * @param directory The directory where the script should be created.
         * @param name The desired name for the script file (e.g., "NewScript.lua").
         * @return The physical path to the created file, or an empty path on failure.
         */
        static std::filesystem::path CreateLuaScript(const std::filesystem::path& directory, const std::string& name);

        /**
         * @brief Creates a new scene file with basic scene structure.
         * @param directory The directory where the scene should be created.
         * @param name The desired name for the scene file (e.g., "NewScene.scene").
         * @return The physical path to the created file, or an empty path on failure.
         */
        static std::filesystem::path CreateScene(const std::filesystem::path& directory, const std::string& name);

        /**
         * @brief Creates a new empty text file.
         * @param directory The directory where the text file should be created.
         * @param name The desired name for the text file (e.g., "note.txt").
         * @return The physical path to the created file, or an empty path on failure.
         */
        static std::filesystem::path CreateTextFile(const std::filesystem::path& directory, const std::string& name);

        /**
         * @brief Generates a unique name for a new file in a directory by appending a number if necessary.
         * @param directory The directory to check for existing files.
         * @param baseName The base name for the file (e.g., "NewScript").
         * @param extension The file extension including the dot (e.g., ".lua").
         * @return A unique filename that does not currently exist in the directory.
         */
        static std::string GetUniqueName(const std::filesystem::path& directory, const std::string& baseName, const std::string& extension);
    };

}
