#pragma once

#include <filesystem>
#include <string>

namespace Spark {

    /**
     * @brief Service for performing file operations with integrated error reporting.
     * 
     * This service provides safe wrappers for common file system operations like Rename,
     * Delete, and Move. It is designed to be called from UI components (e.g., ContentBrowser)
     * and will automatically log errors and information using SP_ERROR and SP_INFO.
     */
    class FileOperationService {
    public:
        /**
         * @brief Renames a file or directory.
         * @param oldPath The current physical path of the file or directory.
         * @param newPath The new physical path for the file or directory.
         * @return true if the rename was successful, false otherwise.
         */
        static bool Rename(const std::filesystem::path& oldPath, const std::filesystem::path& newPath);

        /**
         * @brief Deletes a file or directory (recursively if it's a directory).
         * @param path The physical path to the file or directory to delete.
         * @return true if the deletion was successful, false otherwise.
         */
        static bool Delete(const std::filesystem::path& path);

        /**
         * @brief Moves a file or directory to a new location.
         * @param source The current physical path of the file or directory.
         * @param destination The target directory or new path.
         * @return true if the move was successful, false otherwise.
         */
        static bool Move(const std::filesystem::path& source, const std::filesystem::path& destination);

        /**
         * @brief Creates a new directory at the specified path.
         * @param path The physical path of the directory to create.
         * @return true if the directory was created successfully, false otherwise.
         */
        static bool CreateDirectory(const std::filesystem::path& path);
    };

}
