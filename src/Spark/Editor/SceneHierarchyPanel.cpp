#include "SceneHierarchyPanel.h"
#include "imgui.h"
#include "Components.h"
#include "Log.h"
#include "Command.h"
#include "TransformCommand.h"
#include "AssetCommands.h"
#include "AssetManager.h"
#include "imgui_internal.h"

static bool DrawVec2Control(const std::string& label, glm::vec2& values, float speed = 0.1f) {
    ImGui::PushID(label.c_str());
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 100);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    bool changed = ImGui::DragFloat2("##v2", &values.x, speed);
    ImGui::PopItemWidth();
    ImGui::Columns(1);
    ImGui::PopID();
    return changed;
}

static bool DrawFloatControl(const std::string& label, float* value, float speed = 0.1f, float min = 0.0f, float max = 0.0f) {
    ImGui::PushID(label.c_str());
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 100);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    bool changed = ImGui::DragFloat("##f", value, speed, min, max);
    ImGui::PopItemWidth();
    ImGui::Columns(1);
    ImGui::PopID();
    return changed;
}

static bool DrawColorControl(const std::string& label, glm::vec4& values) {
    ImGui::PushID(label.c_str());
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 100);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    bool changed = ImGui::ColorEdit4("##c4", &values.x);
    ImGui::PopItemWidth();
    ImGui::Columns(1);
    ImGui::PopID();
    return changed;
}

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
                SP_DEBUG_TRACE("UI: Hierarchy 'Create Empty Entity' clicked");
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
            SP_DEBUG_TRACE("UI: Hierarchy 'Delete Entity' clicked for '" + tag + "'");
            if (m_SelectionContext == entity) m_SelectionContext = {};
            m_Context->DestroyEntity(entity);
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
        
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 100);
        ImGui::Text("Tag");
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, tag.c_str());
        if (ImGui::InputText("##tag", buffer, sizeof(buffer))) {
            tag = std::string(buffer);
        }
        ImGui::PopItemWidth();
        ImGui::Columns(1);
    }

    ImGui::Separator();

    if (entity.HasComponent<TransformComponent>()) {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& transform = entity.GetComponent<TransformComponent>();
            
            static glm::vec3 s_InitialTranslation, s_InitialRotation, s_InitialScale;
            
            auto DrawVec3Control = [&](const std::string& label, glm::vec3& values, float resetValue = 0.0f) {
                ImGui::PushID(label.c_str());
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 100);
                ImGui::Text("%s", label.c_str());
                ImGui::NextColumn();

                ImGui::PushMultiItemsWidths(3, ImGui::GetContentRegionAvail().x);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

                float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
                ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
                if (ImGui::Button("X", buttonSize)) values.x = resetValue;
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
                ImGui::PopItemWidth();
                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
                if (ImGui::Button("Y", buttonSize)) values.y = resetValue;
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
                ImGui::PopItemWidth();
                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
                if (ImGui::Button("Z", buttonSize)) values.z = resetValue;
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
                ImGui::PopItemWidth();

                ImGui::PopStyleVar();
                ImGui::Columns(1);
                ImGui::PopID();
            };

            DrawVec3Control("Translation", transform.Translation);
            DrawVec3Control("Rotation", transform.Rotation);
            DrawVec3Control("Scale", transform.Scale, 1.0f);
        }
    }

    if (entity.HasComponent<SpriteRendererComponent>()) {
        if (ImGui::CollapsingHeader("Sprite Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& src = entity.GetComponent<SpriteRendererComponent>();
            DrawColorControl("Color", src.Color);
            
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100);
            ImGui::Text("Texture");
            ImGui::NextColumn();
            ImGui::Button("Drag Asset Here", { -1, 0 });
            ImGui::Columns(1);

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
            DrawColorControl("Color", crc.Color);
            DrawFloatControl("Thickness", &crc.Thickness, 0.025f, 0.0f, 1.0f);
            DrawFloatControl("Fade", &crc.Fade, 0.0001f, 0.0f, 1.0f);
        }
    }

    if (entity.HasComponent<Rigidbody2DComponent>()) {
        if (ImGui::CollapsingHeader("Rigidbody 2D", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100);
            ImGui::Text("Body Type");
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            
            const char* bodyTypeStrings[] = { "Static", "Kinematic", "Dynamic" };
            const char* currentBodyTypeString = bodyTypeStrings[(int)rb2d.Type];
            if (ImGui::BeginCombo("##bodytype", currentBodyTypeString)) {
                for (int i = 0; i < 3; i++) {
                    bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                    if (ImGui::Selectable(bodyTypeStrings[i], isSelected)) {
                        rb2d.Type = (Rigidbody2DComponent::BodyType)i;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::Columns(1);

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100);
            ImGui::Text("Fixed Rotation");
            ImGui::NextColumn();
            ImGui::Checkbox("##fixedrot", &rb2d.FixedRotation);
            ImGui::Columns(1);
        }
    }

    if (entity.HasComponent<BoxCollider2DComponent>()) {
        if (ImGui::CollapsingHeader("Box Collider 2D", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
            DrawVec2Control("Offset", bc2d.Offset);
            DrawVec2Control("Size", bc2d.Size);
            DrawFloatControl("Density", &bc2d.Density, 0.01f, 0.0f, 1.0f);
            DrawFloatControl("Friction", &bc2d.Friction, 0.01f, 0.0f, 1.0f);
            DrawFloatControl("Restitution", &bc2d.Restitution, 0.01f, 0.0f, 1.0f);
            DrawFloatControl("Thres", &bc2d.RestitutionThreshold, 0.01f, 0.0f);
        }
    }

    if (entity.HasComponent<CircleCollider2DComponent>()) {
        if (ImGui::CollapsingHeader("Circle Collider 2D", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
            DrawVec2Control("Offset", cc2d.Offset);
            DrawFloatControl("Radius", &cc2d.Radius, 0.01f);
            DrawFloatControl("Density", &cc2d.Density, 0.01f, 0.0f, 1.0f);
            DrawFloatControl("Friction", &cc2d.Friction, 0.01f, 0.0f, 1.0f);
            DrawFloatControl("Restitution", &cc2d.Restitution, 0.01f, 0.0f, 1.0f);
            DrawFloatControl("Thres", &cc2d.RestitutionThreshold, 0.01f, 0.0f);
        }
    }

    if (entity.HasComponent<AudioSourceComponent>()) {
        if (ImGui::CollapsingHeader("Audio Source", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& audio = entity.GetComponent<AudioSourceComponent>();
            
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100);
            ImGui::Text("Path");
            ImGui::NextColumn();
            ImGui::PushItemWidth(-1);
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, audio.Path.c_str());
            if (ImGui::InputText("##audiopath", buffer, sizeof(buffer))) {
                audio.Path = std::string(buffer);
            }
            ImGui::PopItemWidth();
            ImGui::Columns(1);

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

            DrawFloatControl("Volume", &audio.Volume, 0.01f, 0.0f, 1.0f);
            DrawFloatControl("Pitch", &audio.Pitch, 0.01f, 0.0f, 2.0f);
            
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100);
            ImGui::Text("Settings");
            ImGui::NextColumn();
            ImGui::Checkbox("Loop", &audio.Loop);
            ImGui::Checkbox("Start", &audio.PlayOnStart);
            ImGui::Checkbox("Spatial", &audio.Spatial);
            ImGui::Columns(1);
        }
    }

    if (entity.HasComponent<AudioListenerComponent>()) {
        if (ImGui::CollapsingHeader("Audio Listener", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& listener = entity.GetComponent<AudioListenerComponent>();
            
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 100);
            ImGui::Text("Active");
            ImGui::NextColumn();
            ImGui::Checkbox("##active", &listener.Active);
            ImGui::Columns(1);
        }
    }

    if (ImGui::Button("Add Component")) {
        SP_DEBUG_TRACE("UI: Properties 'Add Component' clicked");
        ImGui::OpenPopup("AddComponent");
    }

    if (ImGui::BeginPopup("AddComponent")) {
        if (ImGui::MenuItem("Sprite Renderer")) {
            SP_DEBUG_TRACE("UI: Menu 'Add Sprite Renderer' clicked");
            if (!m_SelectionContext.HasComponent<SpriteRendererComponent>())
                m_SelectionContext.AddComponent<SpriteRendererComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Circle Renderer")) {
            SP_DEBUG_TRACE("UI: Menu 'Add Circle Renderer' clicked");
            if (!m_SelectionContext.HasComponent<CircleRendererComponent>())
                m_SelectionContext.AddComponent<CircleRendererComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Rigidbody 2D")) {
            SP_DEBUG_TRACE("UI: Menu 'Add Rigidbody 2D' clicked");
            if (!m_SelectionContext.HasComponent<Rigidbody2DComponent>())
                m_SelectionContext.AddComponent<Rigidbody2DComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Box Collider 2D")) {
            SP_DEBUG_TRACE("UI: Menu 'Add Box Collider 2D' clicked");
            if (!m_SelectionContext.HasComponent<BoxCollider2DComponent>())
                m_SelectionContext.AddComponent<BoxCollider2DComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Circle Collider 2D")) {
            SP_DEBUG_TRACE("UI: Menu 'Add Circle Collider 2D' clicked");
            if (!m_SelectionContext.HasComponent<CircleCollider2DComponent>())
                m_SelectionContext.AddComponent<CircleCollider2DComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Audio Source")) {
            SP_DEBUG_TRACE("UI: Menu 'Add Audio Source' clicked");
            if (!m_SelectionContext.HasComponent<AudioSourceComponent>())
                m_SelectionContext.AddComponent<AudioSourceComponent>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Audio Listener")) {
            SP_DEBUG_TRACE("UI: Menu 'Add Audio Listener' clicked");
            if (!m_SelectionContext.HasComponent<AudioListenerComponent>())
                m_SelectionContext.AddComponent<AudioListenerComponent>();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::PopID();
}
