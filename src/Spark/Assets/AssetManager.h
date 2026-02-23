#pragma once
#include "Asset.h"
#include "AssetMetadata.h"
#include <map>
#include <memory>
#include <unordered_map>

namespace Spark {

    class AssetManager {
    public:
        static void Init();
        static void Shutdown();

        // Asset laden oder aus Cache holen
        template<typename T>
        static std::shared_ptr<T> GetAsset(AssetHandle handle) {
            if (IsAssetHandleValid(handle)) {
                if (IsAssetLoaded(handle))
                    return std::static_pointer_cast<T>(s_LoadedAssets[handle]);
                
                // Asset laden
                std::shared_ptr<Asset> asset = LoadAsset(handle);
                return std::static_pointer_cast<T>(asset);
            }
            return nullptr;
        }

        static bool IsAssetHandleValid(AssetHandle handle);
        static bool IsAssetLoaded(AssetHandle handle);

        // Neues Asset registrieren (Import)
        static AssetHandle ImportAsset(const std::filesystem::path& filepath);

    private:
        static std::shared_ptr<Asset> LoadAsset(AssetHandle handle);
        static void LoadAssetRegistry();
        static void SerializeAssetRegistry();

    private:
        static std::map<AssetHandle, AssetMetadata> s_AssetRegistry;
        static std::unordered_map<AssetHandle, std::shared_ptr<Asset>> s_LoadedAssets;
    };

}
