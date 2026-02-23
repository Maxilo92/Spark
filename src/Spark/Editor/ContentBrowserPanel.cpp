#include "ContentBrowserPanel.h"
#include "imgui.h"
#include <iostream>
#include <fstream>

namespace Spark {

    static std::filesystem::path GetAssetPath() {
        if (std::filesystem::exists("assets")) return "assets";
        if (std::filesystem::exists("../assets")) return "../assets";
        return "assets";
    }

    ContentBrowserPanel::ContentBrowserPanel()
        : m_CurrentDirectory(GetAssetPath()) {
    }

    void ContentBrowserPanel::OnImGuiRender(FileViewerPanel& viewer, bool* p_open) {
        if (p_open && !*p_open) return;

        ImGui::Begin("Content Browser", p_open);

        std::filesystem::path assetPath = GetAssetPath();

        // Breadcrumbs / Back button
        if (m_CurrentDirectory != assetPath) {
            if (ImGui::Button("<- Back")) {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
            ImGui::SameLine();
        }
        ImGui::Text("Path: //%s", std::filesystem::relative(m_CurrentDirectory, assetPath).string().c_str());
        ImGui::Separator();

        // Listenansicht mit Tabellen
        if (ImGui::BeginTable("ContentTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableHeadersRow();

            if (std::filesystem::exists(m_CurrentDirectory)) {
                for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
                    const auto& path = directoryEntry.path();
                    std::string filenameString = path.filename().string();
                    std::string typeString = directoryEntry.is_directory() ? "Folder" : path.extension().string();

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    // Icon/Emoji Ersatz für Text
                    std::string label = (directoryEntry.is_directory() ? "[D] " : "[F] ") + filenameString;
                    
                    bool isSelected = (m_SelectedFile == path);
                    if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
                        m_SelectedFile = path;
                        
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                            if (directoryEntry.is_directory()) {
                                m_CurrentDirectory /= path.filename();
                                m_SelectedFile = "";
                            } else {
                                viewer.OpenFile(path);
                            }
                        }
                    }

                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Delete")) {
                            std::filesystem::remove_all(path);
                        }
                        if (!directoryEntry.is_directory() && ImGui::MenuItem("Open in Viewer")) {
                            viewer.OpenFile(path);
                        }
                        ImGui::EndPopup();
                    }

                    if (ImGui::BeginDragDropSource()) {
                        std::string pathStr = path.string();
                        const char* itemPath = pathStr.c_str();
                        ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (strlen(itemPath) + 1) * sizeof(char));
                        ImGui::TextUnformatted(filenameString.c_str());
                        ImGui::EndDragDropSource();
                    }

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(typeString.c_str());
                }
            }
            ImGui::EndTable();
        }

        if (ImGui::BeginPopupContextWindow(0, 1)) {
            if (ImGui::MenuItem("New Folder")) {
                std::filesystem::create_directory(m_CurrentDirectory / "NewFolder");
            }
            if (ImGui::MenuItem("New Lua Script")) {
                std::string filename = "NewScript.lua";
                std::ofstream ofs(m_CurrentDirectory / filename);
                ofs << "function OnStart()\nend\n\nfunction OnUpdate(dt)\nend\n";
                ofs.close();
            }
            if (ImGui::MenuItem("New Scene")) {
                std::string filename = "NewScene.scene";
                std::ofstream ofs(m_CurrentDirectory / filename);
                ofs << "Scene: Untitled\nEntities: []\n";
                ofs.close();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

}
