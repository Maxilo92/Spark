#include "Renderer.h"
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData();

void Renderer::Init() {
}

void Renderer::Shutdown() {
    delete m_SceneData;
}

void Renderer::BeginScene(OrthographicCamera& camera) {
    m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
}

void Renderer::EndScene() {
}

void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform) {
    shader->Bind();
    shader->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
    shader->UploadUniformMat4("u_Transform", transform);
    
    vertexArray->Bind();
    glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
}
