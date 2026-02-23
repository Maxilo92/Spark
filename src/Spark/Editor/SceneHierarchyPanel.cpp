#include "SceneHierarchyPanel.h"
#include "imgui.h"
#include "Components.h"
#include "Command.h"
#include "TransformCommand.h"
#include "AssetCommands.h"
#include "AssetManager.h"

SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene>& scene) {
    SetContext(scene);
}

void SceneHierarchyPanel::SetContext(const std::shared_ptr<Scene>& scene) {
    m_Context = scene;
}

void SceneHierarchyPanel::OnImGuiRender(bool* p_open_hierarchy, bool* p_open_properties) {
    if (!p_open_hierarchy || *p_open_hierarchy) {
        ImGui::Begin("Scene Hierarchy", p_open_hierarchy);

        ImGui::PushID("EntityHierarchy");
        m_Context->m_Registry.view<TagComponent>().each([&](auto entityID, auto& tag) {
            Entity entity(entityID, m_Context.get());
            DrawEntityNode(entity);
        });
        ImGui::PopID();

        // Right-click on empty space in Hierarchy
        if (ImGui::BeginPopupContextWindow(0, 1)) {
            if (ImGui::MenuItem("Create Empty Entity")) {
                m_Context->CreateEntity("Empty Entity");
            }
            ImGui::EndPopup();
        }

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            m_SelectionContext = {};

        ImGui::End();
    }

    if (!p_open_properties || *p_open_properties) {
        ImGui::Begin("Properties", p_open_properties);
        if (m_SelectionContext) {
            DrawComponents(m_SelectionContext);
        }
        ImGui::End();
    }
}

void SceneHierarchyPanel::DrawEntityNode(Entity entity) {
    if (!entity.HasComponent<TagComponent>()) return;
    
    auto& tag = entity.GetComponent<TagComponent>().Tag;
    
    ImGui::PushID((int)(uint32_t)entity);

    ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
    flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
    
    // Wir nutzen den Namen + die ID als versteckten String-Identifier
    bool opened = ImGui::TreeNodeEx((void*)(intptr_t)((uint32_t)entity + 1000), flags, "%s", tag.c_str());
    if (ImGui::IsItemClicked()) {
        m_SelectionContext = entity;
    }

    // Context menu for the entity itself
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Delete Entity")) {
            if (m_SelectionContext == entity) m_SelectionContext = {};
            m_Context->m_Registry.destroy(entity);
        }
        ImGui::EndPopup();
    }

    if (opened) {
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void SceneHierarchyPanel::DrawComponents(Entity entity) {
    // Nutze den Handle des Entities als Scope für alle IDs in diesem Panel
    ImGui::PushID((int)(uint32_t)entity);

    if (entity.HasComponent<IDComponent>()) {
        ImGui::Text("UUID: %llu", (uint64_t)entity.GetComponent<IDComponent>().ID);
    }

    if (entity.HasComponent<TagComponent>()) {
        auto& tag = entity.GetComponent<TagComponent>().Tag;
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, tag.c_str());
        if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
            tag = std::string(buffer);
        }
    }

    ImGui::Separator();

    if (entity.HasComponent<TransformComponent>()) {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& transform = entity.GetComponent<TransformComponent>();
            
            static glm::vec3 s_InitialTranslation, s_InitialRotation, s_InitialScale;
            
            auto DrawVec3Control = [&](const std::string& label, glm::vec3& values, float resetValue = 0.0f) {
                if (ImGui::DragFloat3(label.c_str(), &values.x, 0.1f)) {
                    // Während des Drags wird nichts gemacht, außer live Update
                }

                if (ImGui::IsItemActivated()) {
                    s_InitialTranslation = transform.Translation;
                    s_InitialRotation = transform.Rotation;
                    s_InitialScale = transform.Scale;
                }

                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    auto command = std::make_unique<Spark::TransformCommand>(entity, 
                        s_InitialTranslation, s_InitialRotation, s_InitialScale,
                        transform.Translation, transform.Rotation, transform.Scale);
                    Spark::CommandHistory::ExecuteCommand(std::move(command));
                }
            };

            DrawVec3Control("Translation", transform.Translation);
            DrawVec3Control("Rotation", transform.Rotation);
            DrawVec3Control("Scale", transform.Scale, 1.0f);
        }
    }

    if (entity.HasComponent<SpriteRendererComponent>()) {
        if (ImGui::CollapsingHeader("Sprite Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& src = entity.GetComponent<SpriteRendererComponent>();
            ImGui::ColorEdit4("Color", &src.Color.x);
            
            ImGui::Text("Texture Handle: %llu", (uint64_t)src.TextureHandle);

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                    const char* path = (const char*)payload->Data;
                    Spark::AssetHandle newHandle = Spark::AssetManager::ImportAsset(path);
                    
                    if (newHandle != src.TextureHandle) {
                        auto command = std::make_unique<Spark::SpriteChangeCommand>(entity, src.TextureHandle, newHandle);
                        Spark::CommandHistory::ExecuteCommand(std::move(command));
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }
    }

    if (entity.HasComponent<CircleRendererComponent>()) {
        if (ImGui::CollapsingHeader("Circle Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& crc = entity.GetComponent<CircleRendererComponent>();
            ImGui::ColorEdit4("Color", &crc.Color.x);
            ImGui::DragFloat("Thickness", &crc.Thickness, 0.025f, 0.0f, 1.0f);
            ImGui::DragFloat("Fade", &crc.Fade, 0.0001f, 0.0f, 1.0f);
        }
    }

    if (entity.HasComponent<Rigidbody2DComponent>()) {
        if (ImGui::CollapsingHeader("Rigidbody 2D", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            const char* bodyTypeStrings[] = { "Static", "Kinematic", "Dynamic" };
            const char* currentBodyTypeString = bodyTypeStrings[(int)rb2d.Type];
            if (ImGui::BeginCombo("Body Type", currentBodyTypeString)) {
                for (int i = 0; i < 3; i++) {
                    bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                    if (ImGui::Selectable(bodyTypeStrings[i], isSelected)) {
                        rb2d.Type = (Rigidbody2DComponent::BodyType)i;
                    }
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Checkbox("Fixed Rotation", &rb2d.FixedRotation);
        }
    }

    if (entity.HasComponent<BoxCollider2DComponent>()) {
        if (ImGui::CollapsingHeader("Box Collider 2D", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
            ImGui::DragFloat2("Offset", &bc2d.Offset.x, 0.1f);
            ImGui::DragFloat2("Size", &bc2d.Size.x, 0.1f);
            ImGui::DragFloat("Density", &bc2d.Density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &bc2d.Friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &bc2d.Restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &bc2d.RestitutionThreshold, 0.01f, 0.0f);
        }
    }

    if (entity.HasComponent<CircleCollider2DComponent>()) {
        if (ImGui::CollapsingHeader("Circle Collider 2D", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
            ImGui::DragFloat2("Offset", &cc2d.Offset.x, 0.1f);
            ImGui::DragFloat("Radius", &cc2d.Radius, 0.01f);
            ImGui::DragFloat("Density", &cc2d.Density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &cc2d.Friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &cc2d.Restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &cc2d.RestitutionThreshold, 0.01f, 0.0f);
        }
    }

    if (entity.HasComponent<AudioSourceComponent>()) {
        if (ImGui::CollapsingHeader("Audio Source", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& audio = entity.GetComponent<AudioSourceComponent>();
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, audio.Path.c_str());
            if (ImGui::InputText("Path", buffer, sizeof(buffer))) {
                // Live update (can be wrapped in undo/redo if needed)
                audio.Path = std::string(buffer);
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                    const char* path = (const char*)payload->Data;
                    if (audio.Path != path) {
                        auto command = std::make_unique<Spark::AudioPathChangeCommand>(entity, audio.Path, path);
                        Spark::CommandHistory::ExecuteCommand(std::move(command));
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::DragFloat("Volume", &audio.Volume, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Pitch", &audio.Pitch, 0.01f, 0.0f, 2.0f);
            ImGui::Checkbox("Loop", &audio.Loop);
            ImGui::Checkbox("Play on Start", &audio.PlayOnStart);
            ImGui::Checkbox("Spatial", &audio.Spatial);
        }
    }

    if (entity.HasComponent<AudioListenerComponent>()) {
        if (ImGui::CollapsingHeader("Audio Listener", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& listener = entity.GetComponent<AudioListenerComponent>();
            ImGui::Checkbox("Active", &listener.Active);
        }
    }

    if (ImGui::Button("Add Component"))
        ImGui::OpenPopup("AddComponent");

    if (ImGui::BeginPopup("AddComponent")) {
        if (ImGui::MenuItem("Sprite Renderer")) {
            if (!m_SelectionContext.HasComponent<SpriteRendererComponent>())
                m_SelectionContext.AddComponent<SpriteRendererComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Circle Renderer")) {
            if (!m_SelectionContext.HasComponent<CircleRendererComponent>())
                m_SelectionContext.AddComponent<CircleRendererComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Rigidbody 2D")) {
            if (!m_SelectionContext.HasComponent<Rigidbody2DComponent>())
                m_SelectionContext.AddComponent<Rigidbody2DComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Box Collider 2D")) {
            if (!m_SelectionContext.HasComponent<BoxCollider2DComponent>())
                m_SelectionContext.AddComponent<BoxCollider2DComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Circle Collider 2D")) {
            if (!m_SelectionContext.HasComponent<CircleCollider2DComponent>())
                m_SelectionContext.AddComponent<CircleCollider2DComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Audio Source")) {
            if (!m_SelectionContext.HasComponent<AudioSourceComponent>())
                m_SelectionContext.AddComponent<AudioSourceComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Audio Listener")) {
            if (!m_SelectionContext.HasComponent<AudioListenerComponent>())
                m_SelectionContext.AddComponent<AudioListenerComponent>();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::PopID();
}
