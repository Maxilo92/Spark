#include "ContentBrowserPanel.h"
#include "imgui.h"
#include "Log.h"
#include "FileOperationService.h"
#include "AssetTemplateSystem.h"
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

    void ContentBrowserPanel::DrawMenuBar(FileViewerPanel& viewer) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::BeginMenu("New Asset...")) {
                    if (ImGui::MenuItem("Lua Script")) {
                        std::string name = AssetTemplateSystem::GetUniqueName(m_CurrentDirectory, "NewScript", ".lua");
                        SP_DEBUG_TRACE("UI: ContentBrowser 'New Asset > Lua Script' clicked (Name: " + name + ")");
                        AssetTemplateSystem::CreateLuaScript(m_CurrentDirectory, name);
                    }
                    if (ImGui::MenuItem("Scene")) {
                        std::string name = AssetTemplateSystem::GetUniqueName(m_CurrentDirectory, "NewScene", ".scene");
                        SP_DEBUG_TRACE("UI: ContentBrowser 'New Asset > Scene' clicked (Name: " + name + ")");
                        AssetTemplateSystem::CreateScene(m_CurrentDirectory, name);
                    }
                    if (ImGui::MenuItem("Text File")) {
                        std::string name = AssetTemplateSystem::GetUniqueName(m_CurrentDirectory, "note", ".txt");
                        SP_DEBUG_TRACE("UI: ContentBrowser 'New Asset > Text File' clicked (Name: " + name + ")");
                        AssetTemplateSystem::CreateTextFile(m_CurrentDirectory, name);
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("New Folder")) {
                    std::string base = "NewFolder";
                    std::string name = base;
                    int count = 1;
                    while (std::filesystem::exists(m_CurrentDirectory / name)) {
                        name = base + " (" + std::to_string(count++) + ")";
                    }
                    SP_DEBUG_TRACE("UI: ContentBrowser 'New Folder' clicked (Name: " + name + ")");
                    FileOperationService::CreateDirectory(m_CurrentDirectory / name);
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Refresh")) {
                    SP_DEBUG_TRACE("UI: ContentBrowser 'Refresh' clicked");
                    // No internal caching yet, so refresh is implicit.
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                bool hasSelection = !m_SelectedFile.empty();
                if (ImGui::MenuItem("Rename", "F2", false, hasSelection)) {
                    m_ItemToRename = m_SelectedFile;
                    strncpy(m_RenameBuffer, m_SelectedFile.filename().string().c_str(), sizeof(m_RenameBuffer));
                    m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
                    SP_DEBUG_TRACE("UI: ContentBrowser 'Rename' clicked for '" + m_ItemToRename.string() + "'");
                    m_ShowRenamePopup = true;
                }
                if (ImGui::MenuItem("Delete", "Del", false, hasSelection)) {
                    m_ItemToDelete = m_SelectedFile;
                    SP_DEBUG_TRACE("UI: ContentBrowser 'Delete' clicked for '" + m_ItemToDelete.string() + "'");
                    m_ShowDeleteModal = true;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Copy Path", nullptr, false, hasSelection)) {
                    ImGui::SetClipboardText(m_SelectedFile.string().c_str());
                }

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    void ContentBrowserPanel::HandleShortcuts() {
        ImGuiIO& io = ImGui::GetIO();
        bool cmdDown = (io.KeyMods & ImGuiMod_Super) || (io.KeyMods & ImGuiMod_Ctrl);

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
            if (!m_SelectedFile.empty()) {
                if (ImGui::IsKeyPressed(ImGuiKey_F2)) {
                    m_ItemToRename = m_SelectedFile;
                    strncpy(m_RenameBuffer, m_SelectedFile.filename().string().c_str(), sizeof(m_RenameBuffer));
                    m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
                    m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
                    m_ShowRenamePopup = true;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
                    m_ItemToDelete = m_SelectedFile;
                    m_ShowDeleteModal = true;
                }
            }

            // Create New Script with Cmd+N
            if (cmdDown && ImGui::IsKeyPressed(ImGuiKey_N)) {
                std::string name = AssetTemplateSystem::GetUniqueName(m_CurrentDirectory, "NewScript", ".lua");
                AssetTemplateSystem::CreateLuaScript(m_CurrentDirectory, name);
            }
        }
    }

    std::shared_ptr<Texture2D> ContentBrowserPanel::GetOrCreateThumbnail(const std::filesystem::path& path) {
        std::string pathStr = path.string();
        if (m_ThumbnailCache.find(pathStr) == m_ThumbnailCache.end()) {
            m_ThumbnailCache[pathStr] = std::make_shared<Texture2D>(pathStr);
        }
        return m_ThumbnailCache[pathStr];
    }

    void ContentBrowserPanel::OnImGuiRender(FileViewerPanel& viewer, bool* p_open) {
        if (p_open && !*p_open) return;

        ImGui::Begin("Content Browser", p_open, ImGuiWindowFlags_MenuBar);

        DrawMenuBar(viewer);
        HandleShortcuts();

        std::filesystem::path assetPath = GetAssetPath();

        // Breadcrumbs / Back button
        if (m_CurrentDirectory != assetPath) {
            if (ImGui::Button("<- Back")) {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
            ImGui::SameLine();
        }

        std::string pathLabel = "Path: //";
        try {
            pathLabel += std::filesystem::relative(m_CurrentDirectory, assetPath).string();
        } catch (...) {
            pathLabel += m_CurrentDirectory.filename().string();
        }
        ImGui::Text("%s", pathLabel.c_str());
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

                    // determine icon based on type
                    std::string icon = "[F]"; // default file
                    if (directoryEntry.is_directory()) icon = "[D]";
                    else if (path.extension() == ".lua") icon = "[L]";
                    else if (path.extension() == ".scene") icon = "[S]";
                    else if (path.extension() == ".spark") icon = "[*]";
                    else if (path.extension() == ".png" || path.extension() == ".jpg") icon = "[I]";
                    else if (path.extension() == ".wav" || path.extension() == ".mp3") icon = "[A]";
                    else if (path.extension() == ".yaml" || path.extension() == ".json") icon = "[C]";

                    std::string label = icon + " " + filenameString;
                    
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

                    // Right-click to select (fixes context menu and menu bar actions)
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                        m_SelectedFile = path;
                    }

                    // Image Preview Tooltip
                    if (ImGui::IsItemHovered() && (path.extension() == ".png" || path.extension() == ".jpg" || path.extension() == ".jpeg")) {
                        ImGui::BeginTooltip();
                        ImGui::Text("Image Preview:");
                        auto tex = GetOrCreateThumbnail(path);
                        if (tex && tex->GetRendererID() != 0 && tex->GetWidth() > 0) {
                            float aspect = (float)tex->GetHeight() / (float)tex->GetWidth();
                            float width = 128.0f;
                            ImGui::Image((void*)(intptr_t)tex->GetRendererID(), { width, width * aspect }, {0, 1}, {1, 0});
                        }
                        ImGui::Text("%s", filenameString.c_str());
                        ImGui::EndTooltip();
                    }

                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Open", "Double-Click")) {
                            if (directoryEntry.is_directory()) {
                                m_CurrentDirectory /= path.filename();
                                m_SelectedFile = "";
                            } else {
                                viewer.OpenFile(path);
                            }
                        }
                        ImGui::Separator();

                        if (directoryEntry.is_directory()) {
                            if (ImGui::BeginMenu("New Asset...")) {
                                if (ImGui::MenuItem("Lua Script")) {
                                    std::string name = AssetTemplateSystem::GetUniqueName(path, "NewScript", ".lua");
                                    AssetTemplateSystem::CreateLuaScript(path, name);
                                }
                                if (ImGui::MenuItem("Scene")) {
                                    std::string name = AssetTemplateSystem::GetUniqueName(path, "NewScene", ".scene");
                                    AssetTemplateSystem::CreateScene(path, name);
                                }
                                if (ImGui::MenuItem("Text File")) {
                                    std::string name = AssetTemplateSystem::GetUniqueName(path, "note", ".txt");
                                    AssetTemplateSystem::CreateTextFile(path, name);
                                }
                                ImGui::Separator();
                                if (ImGui::MenuItem("New Folder")) {
                                    std::string name = AssetTemplateSystem::GetUniqueName(path, "NewFolder", "");
                                    FileOperationService::CreateDirectory(path / name);
                                }
                                ImGui::EndMenu();
                            }
                            ImGui::Separator();
                        }

                        if (ImGui::MenuItem("Rename")) {
                            m_ItemToRename = path;
                            strncpy(m_RenameBuffer, filenameString.c_str(), sizeof(m_RenameBuffer));
                            m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
                            m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
                            m_ShowRenamePopup = true;
                        }
                        if (ImGui::MenuItem("Delete")) {
                            m_ItemToDelete = path;
                            m_ShowDeleteModal = true;
                        }
                        ImGui::Separator();
                        if (ImGui::MenuItem("Copy Path")) {
                            ImGui::SetClipboardText(path.string().c_str());
                        }
                        
                        if (!directoryEntry.is_directory()) {
                            ImGui::Separator();
                            if (ImGui::MenuItem("Open in Viewer")) {
                                viewer.OpenFile(path);
                            }

                            // Small Thumbnail Preview in Context Menu for Images
                            if (path.extension() == ".png" || path.extension() == ".jpg" || path.extension() == ".jpeg") {
                                ImGui::Separator();
                                ImGui::TextDisabled("Thumbnail:");
                                auto tex = GetOrCreateThumbnail(path);
                                if (tex && tex->GetRendererID() != 0 && tex->GetWidth() > 0) {
                                    float aspect = (float)tex->GetHeight() / (float)tex->GetWidth();
                                    float width = 100.0f;
                                    ImGui::Image((void*)(intptr_t)tex->GetRendererID(), { width, width * aspect }, {0, 1}, {1, 0});
                                }
                            }
                        }
                        ImGui::EndPopup();
                    }

                    // Drag & Drop Source
                    if (ImGui::BeginDragDropSource()) {
                        std::string pathStr = path.string();
                        const char* itemPath = pathStr.c_str();
                        ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (strlen(itemPath) + 1) * sizeof(char));
                        ImGui::TextUnformatted(filenameString.c_str());
                        ImGui::EndDragDropSource();
                    }

                    // Drag & Drop Target (Move into folder)
                    if (directoryEntry.is_directory() && ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                            std::filesystem::path draggedPath = (const char*)payload->Data;
                            if (draggedPath != path) {
                                FileOperationService::Move(draggedPath, path / draggedPath.filename());
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(typeString.c_str());
                }
            }
            ImGui::EndTable();
        }

        // --- CONTEXT MENU (WINDOW) ---
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("New Folder")) {
                std::string name = AssetTemplateSystem::GetUniqueName(m_CurrentDirectory, "NewFolder", "");
                FileOperationService::CreateDirectory(m_CurrentDirectory / name);
            }
            
            ImGui::Separator();

            if (ImGui::MenuItem("New Lua Script")) {
                std::string name = AssetTemplateSystem::GetUniqueName(m_CurrentDirectory, "NewScript", ".lua");
                AssetTemplateSystem::CreateLuaScript(m_CurrentDirectory, name);
            }
            if (ImGui::MenuItem("New Scene")) {
                std::string name = AssetTemplateSystem::GetUniqueName(m_CurrentDirectory, "NewScene", ".scene");
                AssetTemplateSystem::CreateScene(m_CurrentDirectory, name);
            }
            if (ImGui::MenuItem("New Text File")) {
                std::string name = AssetTemplateSystem::GetUniqueName(m_CurrentDirectory, "note", ".txt");
                AssetTemplateSystem::CreateTextFile(m_CurrentDirectory, name);
            }

            if (!m_SelectedFile.empty()) {
                ImGui::Separator();
                if (ImGui::MenuItem("Rename Selected", "F2")) {
                    m_ItemToRename = m_SelectedFile;
                    strncpy(m_RenameBuffer, m_SelectedFile.filename().string().c_str(), sizeof(m_RenameBuffer));
                    m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
                    m_ShowRenamePopup = true;
                }
                if (ImGui::MenuItem("Delete Selected", "Del")) {
                    m_ItemToDelete = m_SelectedFile;
                    m_ShowDeleteModal = true;
                }
            }

            ImGui::EndPopup();
        }

        // --- RENAME POPUP ---
        if (m_ShowRenamePopup) {
            ImGui::OpenPopup("Rename Item");
            if (ImGui::BeginPopupModal("Rename Item", &m_ShowRenamePopup, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Enter new name for: %s", m_ItemToRename.filename().string().c_str());
                ImGui::InputText("##newName", m_RenameBuffer, sizeof(m_RenameBuffer));
                
                if (ImGui::Button("Rename", {120, 0})) {
                    std::filesystem::path newPath = m_ItemToRename.parent_path() / m_RenameBuffer;
                    FileOperationService::Rename(m_ItemToRename, newPath);
                    m_ShowRenamePopup = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", {120, 0})) {
                    m_ShowRenamePopup = false;
                }
                ImGui::EndPopup();
            }
        }

        // --- DELETE MODAL ---
        if (m_ShowDeleteModal) {
            ImGui::OpenPopup("Delete Item?");
            if (ImGui::BeginPopupModal("Delete Item?", &m_ShowDeleteModal, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Are you sure you want to delete: %s?", m_ItemToDelete.filename().string().c_str());
                ImGui::Separator();
                if (ImGui::Button("Delete", {120, 0})) {
                    FileOperationService::Delete(m_ItemToDelete);
                    if (m_SelectedFile == m_ItemToDelete) m_SelectedFile = "";
                    m_ShowDeleteModal = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", {120, 0})) {
                    m_ShowDeleteModal = false;
                }
                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }

}
