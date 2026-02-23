#include "AssetManager.h"
#include "FileSystem.h"
#include "Texture.h"
#include "AudioManager.h"
#include "Log.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>

namespace Spark {

    std::map<AssetHandle, AssetMetadata> AssetManager::s_AssetRegistry;
    std::unordered_map<AssetHandle, std::shared_ptr<Asset>> AssetManager::s_LoadedAssets;

    void AssetManager::Init() {
        LoadAssetRegistry();
    }

    void AssetManager::Shutdown() {
        SerializeAssetRegistry();
        s_LoadedAssets.clear();
        s_AssetRegistry.clear();
    }

    bool AssetManager::IsAssetHandleValid(AssetHandle handle) {
        return handle != 0 && s_AssetRegistry.find(handle) != s_AssetRegistry.end();
    }

    bool AssetManager::IsAssetLoaded(AssetHandle handle) {
        return s_LoadedAssets.find(handle) != s_LoadedAssets.end();
    }

    AssetHandle AssetManager::ImportAsset(const std::filesystem::path& filepath) {
        SP_INFO("Importing asset: " + filepath.string());
        
        AssetMetadata metadata;
        metadata.Handle = AssetHandle();
        metadata.FilePath = filepath;
        
        std::string ext = filepath.extension().string();
        if (ext == ".png" || ext == ".jpg") metadata.Type = AssetType::Texture2D;
        else if (ext == ".wav" || ext == ".mp3") metadata.Type = AssetType::AudioSource;
        else if (ext == ".scene") metadata.Type = AssetType::Scene;

        if (metadata.Type != AssetType::None) {
            s_AssetRegistry[metadata.Handle] = metadata;
            SerializeAssetRegistry();
            SP_INFO("Asset imported successfully with Handle " + std::to_string((uint64_t)metadata.Handle));
            return metadata.Handle;
        }
        
        SP_WARN("Unknown asset type for file: " + filepath.string());
        return 0;
    }

    std::shared_ptr<Asset> AssetManager::LoadAsset(AssetHandle handle) {
        const auto& metadata = s_AssetRegistry.at(handle);
        SP_INFO("Loading asset: " + metadata.FilePath.string());
        
        std::shared_ptr<Asset> asset = nullptr;

        std::string physicalPath = FileSystem::ResolvePhysicalPath(metadata.FilePath.string()).string();

        switch (metadata.Type) {
            case AssetType::Texture2D:
                asset = std::make_shared<Texture2D>(physicalPath);
                break;
            default:
                break;
        }

        if (asset) {
            asset->Handle = handle;
            s_LoadedAssets[handle] = asset;
            SP_INFO("Asset loaded successfully.");
        } else {
            SP_ERROR("Failed to load asset: " + metadata.FilePath.string());
        }
        return asset;
    }

    void AssetManager::LoadAssetRegistry() {
        std::string path = "assets/AssetRegistry.yaml";
        if (!std::filesystem::exists(path)) return;

        std::ifstream stream(path);
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        auto assets = data["Assets"];
        if (assets) {
            for (auto asset : assets) {
                AssetMetadata metadata;
                metadata.Handle = asset["Handle"].as<uint64_t>();
                metadata.FilePath = asset["FilePath"].as<std::string>();
                metadata.Type = (AssetType)asset["Type"].as<int>();
                s_AssetRegistry[metadata.Handle] = metadata;
            }
            SP_DEBUG_TRACE("AssetManager: Loaded registry with " + std::to_string(s_AssetRegistry.size()) + " entries.");
        }
    }

    void AssetManager::SerializeAssetRegistry() {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

        for (auto const& [handle, metadata] : s_AssetRegistry) {
            out << YAML::BeginMap;
            out << YAML::Key << "Handle" << YAML::Value << (uint64_t)handle;
            out << YAML::Key << "FilePath" << YAML::Value << metadata.FilePath.string();
            out << YAML::Key << "Type" << YAML::Value << (int)metadata.Type;
            out << YAML::EndMap;
        }

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout("assets/AssetRegistry.yaml");
        fout << out.c_str();
    }

}
