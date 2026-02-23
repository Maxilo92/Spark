#pragma once
#include "UUID.h"

namespace Spark {

    using AssetHandle = UUID;

    enum class AssetType : uint16_t {
        None = 0,
        Texture2D,
        AudioSource,
        Scene
    };

    class Asset {
    public:
        AssetHandle Handle;

        virtual AssetType GetType() const = 0;
    };

}
