#include "TriangleLayer.h"
#include "ProjectManager.h"
#include "MacOSUtils.h"
#include "imgui.h"
#include "Entity.h"
#include "Components.h"
#include "SceneSerializer.h"
#include "AssetManager.h"
#include "Renderer2D.h"
#include "ImGuizmo.h"
#include "Log.h"
#include "Command.h"
#include "TransformCommand.h"
#include <box2d/box2d.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

void TriangleLayer::OnAttach() {
    m_EditorScene = std::make_shared<Scene>();
    m_ActiveScene = m_EditorScene;

    m_SceneHierarchyPanel.SetContext(m_ActiveScene);

    // --- PHYSICS DEMO SETUP ---
    auto ball = m_EditorScene->CreateEntity("Ball");
    ball.AddComponent<CircleRendererComponent>(glm::vec4{0.8f, 0.2f, 0.3f, 1.0f});
    ball.AddComponent<Rigidbody2DComponent>(Rigidbody2DComponent::BodyType::Dynamic);
    ball.AddComponent<LuaScriptComponent>("assets/scripts/Ball.lua");
    auto& ballCollider = ball.AddComponent<CircleCollider2DComponent>();
    ballCollider.Radius = 0.5f;
    ballCollider.Restitution = 0.8f;
    ballCollider.Friction = 0.1f;
    if (ball.HasComponent<TransformComponent>())
        ball.GetComponent<TransformComponent>().Scale = { 0.2f, 0.2f, 1.0f };

    auto wallBottom = m_EditorScene->CreateEntity("Wall Bottom");
    if (wallBottom.HasComponent<TransformComponent>()) {
        wallBottom.GetComponent<TransformComponent>().Translation = { 0.0f, -0.95f, 0.0f };
        wallBottom.GetComponent<TransformComponent>().Scale = { 3.2f, 0.1f, 1.0f };
    }
    wallBottom.AddComponent<SpriteRendererComponent>(glm::vec4{0.3f, 0.3f, 0.3f, 1.0f});
    wallBottom.AddComponent<Rigidbody2DComponent>(Rigidbody2DComponent::BodyType::Static);
    wallBottom.AddComponent<BoxCollider2DComponent>().Size = { 1.6f, 0.05f };

    auto wallTop = m_EditorScene->CreateEntity("Wall Top");
    if (wallTop.HasComponent<TransformComponent>()) {
        wallTop.GetComponent<TransformComponent>().Translation = { 0.0f, 0.95f, 0.0f };
        wallTop.GetComponent<TransformComponent>().Scale = { 3.2f, 0.1f, 1.0f };
    }
    wallTop.AddComponent<SpriteRendererComponent>(glm::vec4{0.3f, 0.3f, 0.3f, 1.0f});
    wallTop.AddComponent<Rigidbody2DComponent>(Rigidbody2DComponent::BodyType::Static);
    wallTop.AddComponent<BoxCollider2DComponent>().Size = { 1.6f, 0.05f };

    auto wallLeft = m_EditorScene->CreateEntity("Wall Left");
    if (wallLeft.HasComponent<TransformComponent>()) {
        wallLeft.GetComponent<TransformComponent>().Translation = { -1.65f, 0.0f, 0.0f };
        wallLeft.GetComponent<TransformComponent>().Scale = { 0.1f, 2.0f, 1.0f };
    }
    wallLeft.AddComponent<SpriteRendererComponent>(glm::vec4{0.3f, 0.3f, 0.3f, 1.0f});
    wallLeft.AddComponent<Rigidbody2DComponent>(Rigidbody2DComponent::BodyType::Static);
    wallLeft.AddComponent<BoxCollider2DComponent>().Size = { 0.05f, 1.0f };

    auto wallRight = m_EditorScene->CreateEntity("Wall Right");
    if (wallRight.HasComponent<TransformComponent>()) {
        wallRight.GetComponent<TransformComponent>().Translation = { 1.65f, 0.0f, 0.0f };
        wallRight.GetComponent<TransformComponent>().Scale = { 0.1f, 2.0f, 1.0f };
    }
    wallRight.AddComponent<SpriteRendererComponent>(glm::vec4{0.3f, 0.3f, 0.3f, 1.0f});
    wallRight.AddComponent<Rigidbody2DComponent>(Rigidbody2DComponent::BodyType::Static);
    wallRight.AddComponent<BoxCollider2DComponent>().Size = { 0.05f, 1.0f };
    // --- END PHYSICS DEMO SETUP ---

    m_VertexArray = std::make_shared<VertexArray>();
    float vertices[4 * 5] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
    };
    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices, sizeof(vertices));
    m_VertexArray->AddVertexBuffer(vertexBuffer);
    uint32_t indices[6] = { 0, 1, 2, 2, 3, 0 };
    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices, 6);
    m_VertexArray->SetIndexBuffer(indexBuffer);

    std::string vertexSrc = R"(#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
out vec2 v_TexCoord;
void main() {
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
    v_TexCoord = a_TexCoord;
})";

    std::string fragmentSrc = R"(#version 330 core
out vec4 color;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;
uniform vec4 u_Color;
uniform bool u_HasTexture;
void main() {
    if (u_HasTexture) color = texture(u_Texture, v_TexCoord) * u_Color;
    else color = u_Color;
})";

    m_Shader = std::make_shared<Shader>(vertexSrc, fragmentSrc);
    
    FramebufferSpecification fbSpec;
    fbSpec.Width = 1280;
    fbSpec.Height = 720;
    m_Framebuffer = std::make_shared<Framebuffer>(fbSpec);
}

void TriangleLayer::OnUpdate(float dt) {
    if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
       (m_Framebuffer->GetSpecification().Width != m_ViewportSize.x || m_Framebuffer->GetSpecification().Height != m_ViewportSize.y)) {
        m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
    }

    m_ActiveScene->OnUpdate(dt);

    m_Framebuffer->Bind();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_ActiveScene->Render(m_Camera);

    m_Framebuffer->Unbind();
}

void TriangleLayer::OnImGuiRender() {
    // Check for native macOS About request
    if (Spark::MacOSUtils::ShouldShowAbout()) {
        m_ShowAboutPopup = true;
        Spark::MacOSUtils::ResetAboutFlag();
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Project")) {
            if (ImGui::MenuItem("New Project...")) m_ShowNewProjectPopup = true;
            if (ImGui::MenuItem("Open Project...")) m_ShowLoadProjectPopup = true;
            ImGui::Separator();
            if (ImGui::MenuItem("Save Project")) {
                if (Spark::ProjectManager::HasActiveProject()) {
                    Spark::ProjectManager::SaveProject(Spark::ProjectManager::GetActiveProjectPath());
                } else {
                    m_ShowSaveProjectPopup = true;
                }
            }
            if (ImGui::MenuItem("Save Project As...")) m_ShowSaveProjectPopup = true;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows")) {
            if (ImGui::MenuItem("Reset Layout")) {
                m_ShowViewport = true; m_ShowSceneHierarchy = true; m_ShowProperties = true;
                m_ShowPlanPanel = true; m_ShowSettings = true; m_ShowConsole = true;
                m_ShowContentBrowser = true; m_ShowFileViewer = true;
            }
            ImGui::Separator();
            ImGui::MenuItem("Viewport", nullptr, &m_ShowViewport);
            ImGui::MenuItem("Scene Hierarchy", nullptr, &m_ShowSceneHierarchy);
            ImGui::MenuItem("Properties", nullptr, &m_ShowProperties);
            ImGui::MenuItem("Content Browser", nullptr, &m_ShowContentBrowser);
            ImGui::MenuItem("File Viewer", nullptr, &m_ShowFileViewer);
            ImGui::MenuItem("Console", nullptr, &m_ShowConsole);
            ImGui::MenuItem("Plan & Roadmap", nullptr, &m_ShowPlanPanel);
            ImGui::MenuItem("Settings", nullptr, &m_ShowSettings);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About Spark")) {
                m_ShowAboutPopup = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    static ImGuizmo::OPERATION m_CurrentGizmoOperation(ImGuizmo::TRANSLATE);
    ImGuizmo::BeginFrame();

    if (m_ShowViewport) {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
        if (m_ActiveScene->IsSimulating() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            window_flags |= ImGuiWindowFlags_NoMove;

        ImGui::Begin("Viewport", &m_ShowViewport, window_flags);
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)(intptr_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        
        m_ViewportRectMin = { ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y };

        // --- MOUSE DRAGGING LOGIC ---
        if (m_ActiveScene->IsSimulating() && ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
            ImVec2 mousePos = ImGui::GetMousePos();
            mousePos.x -= m_ViewportRectMin.x;
            mousePos.y -= m_ViewportRectMin.y;

            if (mousePos.x >= 0 && mousePos.x <= m_ViewportSize.x && mousePos.y >= 0 && mousePos.y <= m_ViewportSize.y) {
                float mx = (mousePos.x / m_ViewportSize.x) * 2.0f - 1.0f;
                float my = ((1.0f - (mousePos.y / m_ViewportSize.y)) * 2.0f) - 1.0f;

                glm::mat4 invVP = glm::inverse(m_Camera.GetViewProjectionMatrix());
                glm::vec4 mouseWorld = invVP * glm::vec4(mx, my, 0.0f, 1.0f);
                mouseWorld /= mouseWorld.w;

                auto view = m_ActiveScene->m_Registry.view<TagComponent, Rigidbody2DComponent>();
                for (auto e : view) {
                    if (view.get<TagComponent>(e).Tag == "Ball") {
                        auto& rb2d = view.get<Rigidbody2DComponent>(e);
                        if (rb2d.RuntimeBody) {
                            b2Body* body = (b2Body*)rb2d.RuntimeBody;
                            body->SetTransform({ mouseWorld.x, mouseWorld.y }, body->GetAngle());
                            body->SetLinearVelocity({ 0, 0 });
                        }
                        break;
                    }
                }
            }
        }
        // --- END MOUSE DRAGGING ---

        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity && !m_ActiveScene->IsSimulating()) {
            ImGuizmo::SetOrthographic(true);
            ImGuizmo::SetDrawlist();
            
            ImGuizmo::SetRect(m_ViewportRectMin.x, m_ViewportRectMin.y, m_ViewportSize.x, m_ViewportSize.y);

            const glm::mat4& cameraProjection = m_Camera.GetProjectionMatrix();
            glm::mat4 cameraView = m_Camera.GetViewMatrix();
            auto& tc = selectedEntity.GetComponent<TransformComponent>();
            glm::mat4 transform = tc.GetTransform();

            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), 
                m_CurrentGizmoOperation, ImGuizmo::LOCAL, glm::value_ptr(transform));

            if (ImGuizmo::IsUsing()) {
                float matrixTranslation[3], matrixRotation[3], matrixScale[3];
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), matrixTranslation, matrixRotation, matrixScale);
                tc.Translation = { matrixTranslation[0], matrixTranslation[1], matrixTranslation[2] };
                tc.Rotation = { matrixRotation[0], matrixRotation[1], matrixRotation[2] };
                tc.Scale = { matrixScale[0], matrixScale[1], matrixScale[2] };
            }
        }
        ImGui::End();
    }

    m_SceneHierarchyPanel.OnImGuiRender(&m_ShowSceneHierarchy, &m_ShowProperties);
    m_ContentBrowserPanel.OnImGuiRender(m_FileViewerPanel, &m_ShowContentBrowser);
    m_FileViewerPanel.OnImGuiRender(&m_ShowFileViewer);
    m_ConsolePanel.OnImGuiRender(&m_ShowConsole);
    
    if (m_ShowPlanPanel) m_PlanPanel.OnImGuiRender(&m_ShowPlanPanel);

    if (m_ShowSettings) {
        ImGui::Begin("Settings", &m_ShowSettings);
        if (ImGui::Button("Reload Plan Data")) m_PlanPanel.Load();
        ImGui::Separator();
        if (!m_ActiveScene->IsSimulating()) {
            if (ImGui::Button("Play")) {
                m_ActiveScene = Scene::Copy(m_EditorScene);
                m_SceneHierarchyPanel.SetContext(m_ActiveScene);
                m_ActiveScene->OnRuntimeStart();
            }
        } else {
            if (ImGui::Button("Stop")) {
                m_ActiveScene->OnRuntimeStop();
                m_ActiveScene = m_EditorScene;
                m_SceneHierarchyPanel.SetContext(m_ActiveScene);
            }
        }
        ImGui::End();
    }

    if (m_ShowNewProjectPopup) {
        ImGui::OpenPopup("Create New Project");
        if (ImGui::BeginPopupModal("Create New Project", &m_ShowNewProjectPopup)) {
            ImGui::Text("Enter Project Name:");
            ImGui::InputText("##projname", m_NewProjectNameBuffer, sizeof(m_NewProjectNameBuffer));
            ImGui::Separator();
            if (ImGui::Button("Create", ImVec2(120, 0))) {
                if (m_ActiveScene->IsSimulating()) m_ActiveScene->OnRuntimeStop();
                if (Spark::ProjectManager::NewProject(m_NewProjectNameBuffer, ".")) {
                    m_EditorScene->Clear();
                    m_EditorScene->CreateEntity("New Entity");
                    m_ActiveScene = m_EditorScene;
                    m_SceneHierarchyPanel.SetContext(m_ActiveScene);
                    m_PlanPanel.Load();
                    m_ShowNewProjectPopup = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) m_ShowNewProjectPopup = false;
            ImGui::EndPopup();
        }
    }

    // --- SAVE PROJECT AS POPUP ---
    if (m_ShowSaveProjectPopup) {
        ImGui::OpenPopup("Save Project As");
        if (ImGui::BeginPopupModal("Save Project As", &m_ShowSaveProjectPopup)) {
            ImGui::Text("Enter Filename:");
            ImGui::InputText("##savefilename", m_SaveProjectNameBuffer, sizeof(m_SaveProjectNameBuffer));
            ImGui::SameLine(); ImGui::Text(".spark");

            ImGui::Separator();
            if (ImGui::Button("Save", ImVec2(120, 0))) {
                std::string filename = std::string(m_SaveProjectNameBuffer) + ".spark";
                if (Spark::ProjectManager::SaveProject(filename)) {
                    m_ShowSaveProjectPopup = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) m_ShowSaveProjectPopup = false;
            ImGui::EndPopup();
        }
    }

    // --- LOAD PROJECT POPUP ---
    if (m_ShowLoadProjectPopup) {
        ImGui::OpenPopup("Load Project");
        if (ImGui::BeginPopupModal("Load Project", &m_ShowLoadProjectPopup)) {
            ImGui::Text("Select a project to load:");
            ImGui::BeginChild("ProjectList", ImVec2(0, 200), true);
            
            for (auto& p : std::filesystem::directory_iterator(".")) {
                if (p.path().extension() == ".spark") {
                    std::string filename = p.path().filename().string();
                    if (ImGui::Selectable(filename.c_str())) {
                        if (Spark::ProjectManager::LoadProject(p.path())) {
                            m_ActiveScene = m_EditorScene;
                            m_SceneHierarchyPanel.SetContext(m_ActiveScene);
                            m_PlanPanel.Load();
                            m_ShowLoadProjectPopup = false;
                        }
                    }
                }
            }
            ImGui::EndChild();

            ImGui::Separator();
            if (ImGui::Button("Close", ImVec2(120, 0))) m_ShowLoadProjectPopup = false;
            ImGui::EndPopup();
        }
    }

    // --- ABOUT SPARK POPUP ---
    if (m_ShowAboutPopup) {
        ImGui::OpenPopup("About Spark");
        // Center the window
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        
        if (ImGui::BeginPopupModal("About Spark", &m_ShowAboutPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextDisabled("Spark Engine v1.0.0");
            ImGui::Separator();
            
            ImGui::Text("Lead Developer: Maximilian");
            ImGui::Text("A modern, lightweight 2D game engine built for performance.");
            
            ImGui::Spacing();
            ImGui::Text("Core Technologies:");
            ImGui::BulletText("OpenGL 3.3+ (Graphics)");
            ImGui::BulletText("EnTT (Entity Component System)");
            ImGui::BulletText("Box2D (Physics)");
            ImGui::BulletText("sol2 & Lua (Scripting)");
            ImGui::BulletText("miniaudio (Sound)");
            
            ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                m_ShowAboutPopup = false;
            }
            ImGui::EndPopup();
        }
    }
}
