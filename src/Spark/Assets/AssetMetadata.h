#pragma once
#include "Asset.h"
#include <filesystem>

namespace Spark {

    struct AssetMetadata {
        AssetHandle Handle;
        AssetType Type = AssetType::None;
        std::filesystem::path FilePath;

        operator bool() const { return Type != AssetType::None; }
    };

}
