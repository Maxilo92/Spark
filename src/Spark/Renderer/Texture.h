#pragma once
#include <string>
#include <cstdint>
#include "Asset.h"

class Texture2D : public Spark::Asset {
public:
    Texture2D(uint32_t width, uint32_t height);
    Texture2D(const std::string& path);
    ~Texture2D();

    void SetData(void* data, uint32_t size);

    void Bind(uint32_t slot = 0) const;

    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    uint32_t GetRendererID() const { return m_RendererID; }

    static Spark::AssetType GetStaticType() { return Spark::AssetType::Texture2D; }
    virtual Spark::AssetType GetType() const override { return GetStaticType(); }

private:
    uint32_t m_RendererID;
    uint32_t m_Width, m_Height;
    std::string m_Path;
};
