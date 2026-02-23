#pragma once
#include "VertexArray.h"
#include "Shader.h"
#include "OrthographicCamera.h"
#include <memory>

class Renderer {
public:
    static void Init();
    static void Shutdown();

    static void BeginScene(OrthographicCamera& camera);
    static void EndScene();

    static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

private:
    struct SceneData {
        glm::mat4 ViewProjectionMatrix;
    };

    static SceneData* m_SceneData;
};
