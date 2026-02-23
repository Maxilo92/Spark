#include "FileOperationService.h"
#include "Log.h"
#include <system_error>

namespace Spark {

    bool FileOperationService::Rename(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) {
        try {
            if (!std::filesystem::exists(oldPath)) {
                SP_ERROR("Rename failed: Source path does not exist: " + oldPath.string());
                return false;
            }
            std::filesystem::rename(oldPath, newPath);
            SP_INFO("Renamed: " + oldPath.string() + " to " + newPath.string());
            return true;
        } catch (const std::filesystem::filesystem_error& e) {
            SP_ERROR("Rename failed: " + std::string(e.what()));
            return false;
        }
    }

    bool FileOperationService::Delete(const std::filesystem::path& path) {
        try {
            if (!std::filesystem::exists(path)) {
                SP_WARN("Delete operation on non-existent path: " + path.string());
                return true; // Technically deleted since it's gone
            }
            std::filesystem::remove_all(path);
            SP_INFO("Deleted: " + path.string());
            return true;
        } catch (const std::filesystem::filesystem_error& e) {
            SP_ERROR("Delete failed: " + std::string(e.what()));
            return false;
        }
    }

    bool FileOperationService::Move(const std::filesystem::path& source, const std::filesystem::path& destination) {
        try {
            if (!std::filesystem::exists(source)) {
                SP_ERROR("Move failed: Source path does not exist: " + source.string());
                return false;
            }
            
            std::filesystem::path finalDest = destination;
            if (std::filesystem::is_directory(destination)) {
                finalDest /= source.filename();
            }

            std::filesystem::rename(source, finalDest);
            SP_INFO("Moved: " + source.string() + " to " + finalDest.string());
            return true;
        } catch (const std::filesystem::filesystem_error& e) {
            SP_ERROR("Move failed: " + std::string(e.what()));
            return false;
        }
    }

    bool FileOperationService::CreateDirectory(const std::filesystem::path& path) {
        try {
            if (std::filesystem::exists(path)) {
                SP_INFO("Directory already exists: " + path.string());
                return true;
            }
            if (std::filesystem::create_directories(path)) {
                SP_INFO("Created directory: " + path.string());
                return true;
            }
            SP_ERROR("Failed to create directory: " + path.string());
            return false;
        } catch (const std::filesystem::filesystem_error& e) {
            SP_ERROR("CreateDirectory failed: " + std::string(e.what()));
            return false;
        }
    }

}
