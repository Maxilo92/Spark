#pragma once
#include <glm/glm.hpp>
#include "Texture.h"
#include <memory>

namespace Spark {

    class SubTexture2D {
    public:
        SubTexture2D(const std::shared_ptr<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max);

        const std::shared_ptr<Texture2D> GetTexture() const { return m_Texture; }
        const glm::vec2* GetTexCoords() const { return m_TexCoords; }

        static std::shared_ptr<SubTexture2D> CreateFromCoords(const std::shared_ptr<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize = { 1, 1 });

    private:
        std::shared_ptr<Texture2D> m_Texture;
        glm::vec2 m_TexCoords[4];
    };

}
