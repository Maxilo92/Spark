#include "Renderer2D.h"
#include "SubTexture2D.h"
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

namespace Spark {

    static Renderer2DData s_Data;

    void Renderer2D::Init() {
        s_Data.QuadVertexArray = std::make_shared<VertexArray>();

        s_Data.QuadVertexBuffer = std::make_shared<VertexBuffer>(nullptr, s_Data.MaxVertices * sizeof(QuadVertex));
        
        // Layout definieren
        s_Data.QuadVertexArray->Bind();
        s_Data.QuadVertexBuffer->Bind();
        
        glEnableVertexAttribArray(0); // Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, Position));
        
        glEnableVertexAttribArray(1); // Color
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, Color));
        
        glEnableVertexAttribArray(2); // TexCoord
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, TexCoord));
        
        glEnableVertexAttribArray(3); // TexIndex
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, TexIndex));

        s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

        uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6) {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;
            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;
            offset += 4;
        }
        std::shared_ptr<IndexBuffer> quadIB = std::make_shared<IndexBuffer>(quadIndices, s_Data.MaxIndices);
        s_Data.QuadVertexArray->SetIndexBuffer(quadIB);
        delete[] quadIndices;

        // Echte 1x1 White Texture für einfarbige Quads
        s_Data.WhiteTexture = std::make_shared<Texture2D>(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
        
        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        // Shader
        std::string vertexSrc = R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec4 a_Color;
            layout(location = 2) in vec2 a_TexCoord;
            layout(location = 3) in float a_TexIndex;
            uniform mat4 u_ViewProjection;
            out vec4 v_Color;
            out vec2 v_TexCoord;
            out float v_TexIndex;
            void main() {
                v_Color = a_Color;
                v_TexCoord = a_TexCoord;
                v_TexIndex = a_TexIndex;
                gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
            }
        )";

        std::string fragmentSrc = R"(
            #version 330 core
            layout(location = 0) out vec4 color;
            in vec4 v_Color;
            in vec2 v_TexCoord;
            in float v_TexIndex;
            uniform sampler2D u_Textures[16];
            void main() {
                color = v_Color * texture(u_Textures[int(v_TexIndex)], v_TexCoord);
            }
        )";
        s_Data.TextureShader = std::make_shared<Shader>(vertexSrc, fragmentSrc);
        s_Data.TextureShader->Bind();
        int samplers[16];
        for (int i = 0; i < 16; i++) samplers[i] = i;
        // Shader::UploadUniformIntArray bräuchten wir hier, nutzen wir Einzelaufrufe
        for (int i = 0; i < 16; i++) {
            std::string name = "u_Textures[" + std::to_string(i) + "]";
            s_Data.TextureShader->UploadUniformInt(name, i);
        }

        s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

        // Circles
        s_Data.CircleVertexArray = std::make_shared<VertexArray>();
        s_Data.CircleVertexBuffer = std::make_shared<VertexBuffer>(nullptr, s_Data.MaxVertices * sizeof(CircleVertex));
        s_Data.CircleVertexArray->Bind();
        s_Data.CircleVertexBuffer->Bind();

        glEnableVertexAttribArray(0); // WorldPosition
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CircleVertex), (const void*)offsetof(CircleVertex, WorldPosition));
        glEnableVertexAttribArray(1); // LocalPosition
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CircleVertex), (const void*)offsetof(CircleVertex, LocalPosition));
        glEnableVertexAttribArray(2); // Color
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(CircleVertex), (const void*)offsetof(CircleVertex, Color));
        glEnableVertexAttribArray(3); // Thickness
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(CircleVertex), (const void*)offsetof(CircleVertex, Thickness));
        glEnableVertexAttribArray(4); // Fade
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(CircleVertex), (const void*)offsetof(CircleVertex, Fade));

        s_Data.CircleVertexArray->SetIndexBuffer(quadIB); // Same indices as quads
        s_Data.CircleVertexBufferBase = new CircleVertex[s_Data.MaxVertices];

        std::string circleVertexSrc = R"(
            #version 330 core
            layout(location = 0) in vec3 a_WorldPosition;
            layout(location = 1) in vec3 a_LocalPosition;
            layout(location = 2) in vec4 a_Color;
            layout(location = 3) in float a_Thickness;
            layout(location = 4) in float a_Fade;

            uniform mat4 u_ViewProjection;

            out vec3 v_LocalPosition;
            out vec4 v_Color;
            out float v_Thickness;
            out float v_Fade;

            void main() {
                v_LocalPosition = a_LocalPosition;
                v_Color = a_Color;
                v_Thickness = a_Thickness;
                v_Fade = a_Fade;
                gl_Position = u_ViewProjection * vec4(a_WorldPosition, 1.0);
            }
        )";

        std::string circleFragmentSrc = R"(
            #version 330 core
            layout(location = 0) out vec4 color;

            in vec3 v_LocalPosition;
            in vec4 v_Color;
            in float v_Thickness;
            in float v_Fade;

            void main() {
                float distance = 1.0 - length(v_LocalPosition);
                float circle = smoothstep(0.0, v_Fade, distance);
                circle *= smoothstep(v_Thickness + v_Fade, v_Thickness, 1.0 - length(v_LocalPosition));

                if (circle == 0.0)
                    discard;

                color = v_Color;
                color.a *= circle;
            }
        )";
        s_Data.CircleShader = std::make_shared<Shader>(circleVertexSrc, circleFragmentSrc);
    }

    void Renderer2D::Shutdown() {
        delete[] s_Data.QuadVertexBufferBase;
        delete[] s_Data.CircleVertexBufferBase;
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera) {
        s_Data.TextureShader->Bind();
        s_Data.TextureShader->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
        
        s_Data.CircleShader->Bind();
        s_Data.CircleShader->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

        ResetStats();
        StartBatch();
    }

    void Renderer2D::EndScene() {
        Flush();
    }

    void Renderer2D::ResetStats() {
        memset(&s_Data.Stats, 0, sizeof(Statistics));
    }

    Statistics Renderer2D::GetStats() {
        return s_Data.Stats;
    }

    void Renderer2D::StartBatch() {
        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

        s_Data.CircleIndexCount = 0;
        s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::Flush() {
        if (s_Data.QuadIndexCount > 0) {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
            s_Data.QuadVertexBuffer->Bind();
            glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, s_Data.QuadVertexBufferBase);

            for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
                s_Data.TextureSlots[i]->Bind(i);

            s_Data.TextureShader->Bind();
            s_Data.QuadVertexArray->Bind();
            glDrawElements(GL_TRIANGLES, s_Data.QuadIndexCount, GL_UNSIGNED_INT, nullptr);
            s_Data.Stats.DrawCalls++;
        }

        if (s_Data.CircleIndexCount > 0) {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase);
            s_Data.CircleVertexBuffer->Bind();
            glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, s_Data.CircleVertexBufferBase);

            s_Data.CircleShader->Bind();
            s_Data.CircleVertexArray->Bind();
            glDrawElements(GL_TRIANGLES, s_Data.CircleIndexCount, GL_UNSIGNED_INT, nullptr);
            s_Data.Stats.DrawCalls++;
        }
    }

    void Renderer2D::NextBatch() {
        Flush();
        StartBatch();
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
        DrawQuad(transform, color);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color) {
        if (s_Data.QuadIndexCount >= s_Data.MaxIndices) NextBatch();

        const float texIndex = 0.0f; // White Texture
        const glm::vec2 texCoords[] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };

        for (int i = 0; i < 4; i++) {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, const glm::vec4& tintColor) {
        if (s_Data.QuadIndexCount >= s_Data.MaxIndices) NextBatch();

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {
            if (s_Data.TextureSlots[i]->GetRendererID() == texture->GetRendererID()) {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f) {
            if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                NextBatch();

            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        const glm::vec2 texCoords[] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };

        for (int i = 0; i < 4; i++) {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = tintColor;
            s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const std::shared_ptr<SubTexture2D>& subTexture, const glm::vec4& tintColor) {
        if (s_Data.QuadIndexCount >= s_Data.MaxIndices) NextBatch();

        const std::shared_ptr<Texture2D> texture = subTexture->GetTexture();
        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {
            if (s_Data.TextureSlots[i]->GetRendererID() == texture->GetRendererID()) {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f) {
            if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                NextBatch();

            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        const glm::vec2* texCoords = subTexture->GetTexCoords();

        for (int i = 0; i < 4; i++) {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = tintColor;
            s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness, float fade) {
        if (s_Data.CircleIndexCount >= s_Data.MaxIndices) NextBatch();

        for (int i = 0; i < 4; i++) {
            s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadVertexPositions[i];
            s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
            s_Data.CircleVertexBufferPtr->Color = color;
            s_Data.CircleVertexBufferPtr->Thickness = thickness;
            s_Data.CircleVertexBufferPtr->Fade = fade;
            s_Data.CircleVertexBufferPtr++;
        }

        s_Data.CircleIndexCount += 6;
        s_Data.Stats.CircleCount++;
    }

}
