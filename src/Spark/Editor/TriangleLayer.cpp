#include "TriangleLayer.h"
#include "Version.h"
#include "Application.h"
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
#include "Input.h"
#include "Command.h"
#include "TransformCommand.h"
#include <box2d/box2d.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <yaml-cpp/yaml.h>
#include <fstream>

void TriangleLayer::OnAttach() {
    m_EditorScene = std::make_shared<Scene>();
    m_ActiveScene = m_EditorScene;

    m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    LoadEditorSettings();
    m_ContentBrowserPanel = Spark::ContentBrowserPanel();

    bool sceneLoaded = false;
    if (Spark::ProjectManager::HasActiveProject()) {
        const auto& project = Spark::ProjectManager::GetActiveProject();
        if (!project.StartScene.empty()) {
            SceneSerializer serializer(m_EditorScene);
            if (serializer.Deserialize(project.StartScene)) {
                SP_INFO("Loaded project start scene: " + project.StartScene);
                sceneLoaded = true;
            }
        }
    }

    if (!sceneLoaded) {
        // --- PHYSICS DEMO SETUP ---
        auto ball = m_EditorScene->CreateEntity("Ball");
        ball.AddComponent<CircleRendererComponent>(glm::vec4{ 0.8f, 0.2f, 0.3f, 1.0f });
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
        wallBottom.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.3f, 0.3f, 0.3f, 1.0f });
        wallBottom.AddComponent<Rigidbody2DComponent>(Rigidbody2DComponent::BodyType::Static);
        wallBottom.AddComponent<BoxCollider2DComponent>().Size = { 1.6f, 0.05f };

        auto wallTop = m_EditorScene->CreateEntity("Wall Top");
        if (wallTop.HasComponent<TransformComponent>()) {
            wallTop.GetComponent<TransformComponent>().Translation = { 0.0f, 0.95f, 0.0f };
            wallTop.GetComponent<TransformComponent>().Scale = { 3.2f, 0.1f, 1.0f };
        }
        wallTop.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.3f, 0.3f, 0.3f, 1.0f });
        wallTop.AddComponent<Rigidbody2DComponent>(Rigidbody2DComponent::BodyType::Static);
        wallTop.AddComponent<BoxCollider2DComponent>().Size = { 1.6f, 0.05f };

        auto wallLeft = m_EditorScene->CreateEntity("Wall Left");
        if (wallLeft.HasComponent<TransformComponent>()) {
            wallLeft.GetComponent<TransformComponent>().Translation = { -1.65f, 0.0f, 0.0f };
            wallLeft.GetComponent<TransformComponent>().Scale = { 0.1f, 2.0f, 1.0f };
        }
        wallLeft.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.3f, 0.3f, 0.3f, 1.0f });
        wallLeft.AddComponent<Rigidbody2DComponent>(Rigidbody2DComponent::BodyType::Static);
        wallLeft.AddComponent<BoxCollider2DComponent>().Size = { 0.05f, 1.0f };

        auto wallRight = m_EditorScene->CreateEntity("Wall Right");
        if (wallRight.HasComponent<TransformComponent>()) {
            wallRight.GetComponent<TransformComponent>().Translation = { 1.65f, 0.0f, 0.0f };
            wallRight.GetComponent<TransformComponent>().Scale = { 0.1f, 2.0f, 1.0f };
        }
        wallRight.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.3f, 0.3f, 0.3f, 1.0f });
        wallRight.AddComponent<Rigidbody2DComponent>(Rigidbody2DComponent::BodyType::Static);
        wallRight.AddComponent<BoxCollider2DComponent>().Size = { 0.05f, 1.0f };
        // --- END PHYSICS DEMO SETUP ---
    }

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

    // Register Console Commands
    Spark::Log::RegisterCommand("list", [this](const std::vector<std::string>& args) {
        if (!m_ActiveScene) return;
        SP_INFO("Entities in current scene:");
        m_ActiveScene->m_Registry.view<TagComponent>().each([&](auto entity, auto& tag) {
            SP_INFO(" - " + tag.Tag + " (ID: " + std::to_string((uint32_t)entity) + ")");
        });
    });

    Spark::Log::RegisterCommand("select", [this](const std::vector<std::string>& args) {
        if (!m_ActiveScene) return;
        if (args.empty()) { SP_WARN("Usage: select <name>"); return; }
        
        std::string name = args[0];
        auto view = m_ActiveScene->m_Registry.view<TagComponent>();
        for (auto e : view) {
            if (view.get<TagComponent>(e).Tag == name) {
                m_SceneHierarchyPanel.SetSelectedEntity({e, m_ActiveScene.get()});
                SP_INFO("Selected entity: " + name);
                return;
            }
        }
        SP_ERROR("Entity not found: " + name);
    });

    Spark::Log::RegisterCommand("play", [this](const std::vector<std::string>& args) {
        if (!m_ActiveScene) return;
        if (!m_ActiveScene->IsSimulating()) {
            m_ActiveScene = Scene::Copy(m_EditorScene);
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);
            m_ActiveScene->OnRuntimeStart();
            SP_INFO("Simulation started via console.");
        }
    });

    Spark::Log::RegisterCommand("stop", [this](const std::vector<std::string>& args) {
        if (!m_ActiveScene) return;
        if (m_ActiveScene->IsSimulating()) {
            m_ActiveScene->OnRuntimeStop();
            m_ActiveScene = m_EditorScene;
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);
            SP_INFO("Simulation stopped via console.");
        }
    });
}

void TriangleLayer::OnUpdate(float dt) {
    if (m_Framebuffer && m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
       (m_Framebuffer->GetSpecification().Width != m_ViewportSize.x || m_Framebuffer->GetSpecification().Height != m_ViewportSize.y)) {
        m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        
        if (m_ActiveScene)
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
    }

    if (m_ActiveScene)
        m_ActiveScene->OnUpdate(dt);

    if (m_Framebuffer) {
        m_Framebuffer->Bind();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_ActiveScene) {
            if (m_ActiveScene->IsSimulating())
                m_ActiveScene->RenderRuntime();
            else
                m_ActiveScene->Render(m_Camera);
        }

        m_Framebuffer->Unbind();
    }
}

void TriangleLayer::OnImGuiRender() {
    // Track window visibility changes for verbose logging
    static struct {
        bool Viewport, Hierarchy, Properties, ContentBrowser, FileViewer, Console, Plan, Settings;
    } lastVisible = { m_ShowViewport, m_ShowSceneHierarchy, m_ShowProperties, m_ShowContentBrowser, m_ShowFileViewer, m_ShowConsole, m_ShowPlanPanel, m_ShowSettings };

    auto LogVisibility = [](const std::string& name, bool current, bool& last) {
        if (current != last) {
            SP_DEBUG_TRACE("UI: Window '" + name + "' " + std::string(current ? "opened" : "closed"));
            last = current;
        }
    };

    LogVisibility("Viewport", m_ShowViewport, lastVisible.Viewport);
    LogVisibility("Scene Hierarchy", m_ShowSceneHierarchy, lastVisible.Hierarchy);
    LogVisibility("Properties", m_ShowProperties, lastVisible.Properties);
    LogVisibility("Content Browser", m_ShowContentBrowser, lastVisible.ContentBrowser);
    LogVisibility("File Viewer", m_ShowFileViewer, lastVisible.FileViewer);
    LogVisibility("Console", m_ShowConsole, lastVisible.Console);
    LogVisibility("Plan & Roadmap", m_ShowPlanPanel, lastVisible.Plan);
    LogVisibility("Settings", m_ShowSettings, lastVisible.Settings);

    // Shortcuts
    ImGuiIO& io = ImGui::GetIO();
    bool cmdDown = (io.KeyMods & ImGuiMod_Super) || (io.KeyMods & ImGuiMod_Ctrl);
    
    if (cmdDown && ImGui::IsKeyPressed(ImGuiKey_Z)) {
        if (io.KeyMods & ImGuiMod_Shift) Spark::CommandHistory::Redo();
        else Spark::CommandHistory::Undo();
    }

    if (cmdDown && ImGui::IsKeyPressed(ImGuiKey_B)) {
        SP_DEBUG_TRACE("UI: Shortcut Cmd+B pressed - Triggering Rebuild");
        Application::Get().RebuildAndRestart();
    }

    if (cmdDown && ImGui::IsKeyPressed(ImGuiKey_R)) {
        SP_DEBUG_TRACE("UI: Shortcut Cmd+R pressed - Triggering Quick Restart");
        Application::Get().Restart(Spark::Log::IsVerbose());
    }

    if (cmdDown && ImGui::IsKeyPressed(ImGuiKey_Q)) {
        SP_DEBUG_TRACE("UI: Shortcut Cmd+Q pressed - Exiting Engine");
        glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
    }
    
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
                    Spark::ProjectManager::SaveProject();
                } else {
                    m_ShowSaveProjectPopup = true;
                }
            }
            if (ImGui::MenuItem("Save Project As...")) m_ShowSaveProjectPopup = true;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Cmd+Z", false, Spark::CommandHistory::CanUndo())) { SP_DEBUG_TRACE("UI: Menu 'Undo' clicked"); Spark::CommandHistory::Undo(); }
            if (ImGui::MenuItem("Redo", "Cmd+Shift+Z", false, Spark::CommandHistory::CanRedo())) { SP_DEBUG_TRACE("UI: Menu 'Redo' clicked"); Spark::CommandHistory::Redo(); }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Build")) {
            if (ImGui::MenuItem("Rebuild & Restart", "Cmd+B")) {
                SP_DEBUG_TRACE("UI: Menu 'Rebuild & Restart' clicked");
                Application::Get().RebuildAndRestart();
            }
            if (ImGui::MenuItem("Quick Restart", "Cmd+R")) {
                SP_DEBUG_TRACE("UI: Menu 'Quick Restart' clicked");
                Application::Get().Restart(Spark::Log::IsVerbose());
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit Engine", "Cmd+Q")) {
                SP_DEBUG_TRACE("UI: Menu 'Exit Engine' clicked");
                // Trigger window close event or just call exit
                glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
            }
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
            if (ImGui::MenuItem("Changelog")) {
                m_ShowChangelog = true;
                m_ChangelogPanel.LoadChangelog("CHANGELOG.md");
                m_ChangelogPanel.SetVersionFilter(""); // Show all in manual mode
            }
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
        if (m_ActiveScene && m_ActiveScene->IsSimulating() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            window_flags |= ImGuiWindowFlags_NoMove;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport", &m_ShowViewport, window_flags);
        
        // Update input system with viewport focus state
        Spark::Input::SetViewportFocused(ImGui::IsWindowFocused());
        
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        
        // --- ASPECT RATIO CALCULATION (16:9) ---
        float targetAspect = 16.0f / 9.0f;
        float actualAspect = viewportPanelSize.x / viewportPanelSize.y;
        
        ImVec2 displaySize = viewportPanelSize;
        if (actualAspect > targetAspect) {
            // Window is wider than target
            displaySize.x = viewportPanelSize.y * targetAspect;
        } else {
            // Window is taller than target
            displaySize.y = viewportPanelSize.x / targetAspect;
        }
        
        // Center the display
        ImVec2 offset = { (viewportPanelSize.x - displaySize.x) * 0.5f, (viewportPanelSize.y - displaySize.y) * 0.5f };
        ImGui::SetCursorPos(offset);
        
        m_ViewportSize = { displaySize.x, displaySize.y };
        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)(intptr_t)textureID, displaySize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        
        m_ViewportRectMin = { ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y };

        // --- DRAG & DROP TARGET (VIEWPORT) ---
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
                std::filesystem::path path = (const char*)payload->Data;
                SP_INFO("Viewport: Dropped file: " + path.string());
                
                if (path.extension() == ".png" || path.extension() == ".jpg") {
                    Entity selected = m_SceneHierarchyPanel.GetSelectedEntity();
                    if (selected) {
                        if (!selected.HasComponent<SpriteRendererComponent>())
                            selected.AddComponent<SpriteRendererComponent>();
                        
                        auto& src = selected.GetComponent<SpriteRendererComponent>();
                        src.TextureHandle = Spark::AssetManager::ImportAsset(path);
                    } else {
                        // Create new entity
                        Entity newEntity = m_ActiveScene->CreateEntity("New Sprite");
                        newEntity.AddComponent<SpriteRendererComponent>();
                        auto& src = newEntity.GetComponent<SpriteRendererComponent>();
                        src.TextureHandle = Spark::AssetManager::ImportAsset(path);
                        m_SceneHierarchyPanel.SetSelectedEntity(newEntity);
                    }
                } else if (path.extension() == ".lua") {
                    Entity selected = m_SceneHierarchyPanel.GetSelectedEntity();
                    if (selected) {
                        if (!selected.HasComponent<LuaScriptComponent>())
                            selected.AddComponent<LuaScriptComponent>();
                        auto& script = selected.GetComponent<LuaScriptComponent>();
                        script.Path = path.string();
                    }
                } else if (path.extension() == ".scene") {
                    if (m_ActiveScene->IsSimulating())
                        m_ActiveScene->OnRuntimeStop();
                    
                    m_EditorScene = std::make_shared<Scene>();
                    SceneSerializer serializer(m_EditorScene);
                    if (serializer.Deserialize(path.string())) {
                        m_ActiveScene = m_EditorScene;
                        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
                        SP_INFO("Scene loaded: " + path.string());
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        // --- MOUSE DRAGGING LOGIC ---
        if (m_ActiveScene && m_ActiveScene->IsSimulating() && ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
            ImVec2 mousePos = ImGui::GetMousePos();
            mousePos.x -= m_ViewportRectMin.x;
            mousePos.y -= m_ViewportRectMin.y;

            // Only interact if mouse is actually within the aspect-corrected rect
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
                            body->SetAngularVelocity(0);
                            body->SetAwake(true);
                        }
                        break;
                    }
                }
            }
        }
        // --- END MOUSE DRAGGING ---

        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity && m_ActiveScene && !m_ActiveScene->IsSimulating()) {
            ImGuizmo::SetOrthographic(true);
            ImGuizmo::SetDrawlist();
            
            // Guizmo uses the actual screen position of the centered image
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
        ImGui::PopStyleVar();
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
        if (m_ActiveScene && !m_ActiveScene->IsSimulating()) {
            if (ImGui::Button("Play")) {
                m_ActiveScene = Scene::Copy(m_EditorScene);
                m_SceneHierarchyPanel.SetContext(m_ActiveScene);
                m_ActiveScene->OnRuntimeStart();
            }
        } else {
            if (ImGui::Button("Stop")) {
                if (m_ActiveScene) m_ActiveScene->OnRuntimeStop();
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
                if (m_ActiveScene && m_ActiveScene->IsSimulating()) m_ActiveScene->OnRuntimeStop();
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
                if (Spark::ProjectManager::SaveProject()) {
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
                        if (Spark::ProjectManager::OpenProject(p.path())) {
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
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 20.0f);
            ImGui::TextDisabled("Spark Engine v%s", SPARK_VERSION_STR);
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
            ImGui::PopTextWrapPos();
            ImGui::EndPopup();
        }
    }

    m_ChangelogPanel.OnImGuiRender(&m_ShowChangelog);
}

void TriangleLayer::OnEvent(Event& event) {
    // Basic event handling if needed
}

void TriangleLayer::OnDetach() {
    SaveEditorSettings();
}

void TriangleLayer::SaveEditorSettings() {
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "WindowVisibility" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "Viewport" << YAML::Value << m_ShowViewport;
    out << YAML::Key << "SceneHierarchy" << YAML::Value << m_ShowSceneHierarchy;
    out << YAML::Key << "Properties" << YAML::Value << m_ShowProperties;
    out << YAML::Key << "ContentBrowser" << YAML::Value << m_ShowContentBrowser;
    out << YAML::Key << "FileViewer" << YAML::Value << m_ShowFileViewer;
    out << YAML::Key << "Console" << YAML::Value << m_ShowConsole;
    out << YAML::Key << "PlanPanel" << YAML::Value << m_ShowPlanPanel;
    out << YAML::Key << "Settings" << YAML::Value << m_ShowSettings;
    out << YAML::EndMap;

    out << YAML::Key << "LastVersion" << YAML::Value << SPARK_VERSION_STR;
    out << YAML::EndMap;

    std::ofstream fout("editor_settings.yaml");
    fout << out.c_str();
}

void TriangleLayer::LoadEditorSettings() {
    std::ifstream stream("editor_settings.yaml");
    if (!stream.is_open()) return;

    try {
        YAML::Node data = YAML::Load(stream);
        auto visibility = data["WindowVisibility"];
        if (visibility) {
            if (visibility["Viewport"]) m_ShowViewport = visibility["Viewport"].as<bool>();
            if (visibility["SceneHierarchy"]) m_ShowSceneHierarchy = visibility["SceneHierarchy"].as<bool>();
            if (visibility["Properties"]) m_ShowProperties = visibility["Properties"].as<bool>();
            if (visibility["ContentBrowser"]) m_ShowContentBrowser = visibility["ContentBrowser"].as<bool>();
            if (visibility["FileViewer"]) m_ShowFileViewer = visibility["FileViewer"].as<bool>();
            if (visibility["Console"]) m_ShowConsole = visibility["Console"].as<bool>();
            if (visibility["PlanPanel"]) m_ShowPlanPanel = visibility["PlanPanel"].as<bool>();
            if (visibility["Settings"]) m_ShowSettings = visibility["Settings"].as<bool>();
        }

        if (data["LastVersion"]) {
            std::string lastVersion = data["LastVersion"].as<std::string>();
            if (lastVersion != SPARK_VERSION_STR) {
                m_ShowChangelog = true;
                m_ChangelogPanel.LoadChangelog("CHANGELOG.md");
                m_ChangelogPanel.SetVersionFilter(SPARK_VERSION_STR);
                SP_INFO("New version detected: " + std::string(SPARK_VERSION_STR) + ". Showing changelog.");
            }
        } else {
            // First time or no version found
            m_ShowChangelog = true;
            m_ChangelogPanel.LoadChangelog("CHANGELOG.md");
            m_ChangelogPanel.SetVersionFilter(SPARK_VERSION_STR);
        }
    } catch (const std::exception& e) {
        SP_ERROR("Failed to load editor settings: " + std::string(e.what()));
    }
}
