#pragma once
#include "OrthographicCamera.h"
#include "Texture.h"
#include "VertexArray.h"
#include "Shader.h"
#include <memory>

namespace Spark {

    struct QuadVertex {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
    };

    struct CircleVertex {
        glm::vec3 WorldPosition;
        glm::vec3 LocalPosition;
        glm::vec4 Color;
        float Thickness;
        float Fade;
    };

    struct Renderer2DData {
        static constexpr uint32_t MaxQuads = 10000;
        static constexpr uint32_t MaxVertices = MaxQuads * 4;
        static constexpr uint32_t MaxIndices = MaxQuads * 6;
        static constexpr uint32_t MaxTextureSlots = 16; // Mac Standard

        std::shared_ptr<VertexArray> QuadVertexArray;
        std::shared_ptr<VertexBuffer> QuadVertexBuffer;
        std::shared_ptr<Shader> TextureShader;
        std::shared_ptr<Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        // Circles
        std::shared_ptr<VertexArray> CircleVertexArray;
        std::shared_ptr<VertexBuffer> CircleVertexBuffer;
        std::shared_ptr<Shader> CircleShader;
        uint32_t CircleIndexCount = 0;
        CircleVertex* CircleVertexBufferBase = nullptr;
        CircleVertex* CircleVertexBufferPtr = nullptr;

        std::array<std::shared_ptr<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        glm::vec4 QuadVertexPositions[4];
    };

    class Renderer2D {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const OrthographicCamera& camera);
        static void EndScene();
        static void Flush();

        // Primitives
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
        
        static void DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));

        static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness = 1.0f, float fade = 0.005f);

    private:
        static void StartBatch();
        static void NextBatch();
    };

}
