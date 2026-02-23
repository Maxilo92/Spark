#include "FileViewerPanel.h"
#include "imgui.h"
#include "FileSystem.h"
#include "AudioManager.h"
#include "Log.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>

namespace Spark {

    FileViewerPanel::FileViewerPanel() : m_IsDirty(false), m_ShowRenamePopup(false) {}

    void FileViewerPanel::OpenFile(const std::filesystem::path& path) {
        m_CurrentFile = path;
        m_IsOpen = true;
        m_IsDirty = false;
        m_ImageBuffer = nullptr;
        m_EditableBuffer.clear();

        std::string ext = path.extension().string();
        if (ext == ".txt" || ext == ".lua" || ext == ".scene" || ext == ".yaml") {
            std::string content = FileSystem::ReadFileText(path.string());
            m_EditableBuffer.assign(content.begin(), content.end());
            m_EditableBuffer.push_back('\0');
            m_EditableBuffer.resize(m_EditableBuffer.size() + 10240, '\0');
        } else if (ext == ".png" || ext == ".jpg") {
            m_ImageBuffer = std::make_shared<Texture2D>(path.string());
        }
    }

    void FileViewerPanel::SaveFile() {
        if (m_CurrentFile.empty() || m_EditableBuffer.empty()) return;

        std::ofstream ofs(m_CurrentFile);
        if (ofs.is_open()) {
            ofs << m_EditableBuffer.data();
            ofs.close();
            m_IsDirty = false;
            SP_INFO("File saved: " + m_CurrentFile.filename().string());
        } else {
            SP_ERROR("Failed to save file: " + m_CurrentFile.string());
        }
    }

    void FileViewerPanel::ReloadFile() {
        if (!m_CurrentFile.empty()) {
            OpenFile(m_CurrentFile);
            SP_INFO("File reloaded: " + m_CurrentFile.filename().string());
        }
    }

    void FileViewerPanel::RenameFile(const std::string& newName) {
        if (m_CurrentFile.empty() || newName.empty()) return;

        std::filesystem::path newPath = m_CurrentFile.parent_path() / newName;
        try {
            std::filesystem::rename(m_CurrentFile, newPath);
            m_CurrentFile = newPath;
            SP_INFO("File renamed to: " + newName);
        } catch (const std::exception& e) {
            SP_ERROR("Failed to rename file: " + std::string(e.what()));
        }
    }

    void FileViewerPanel::OnImGuiRender(bool* p_open) {
        if (!m_IsOpen) return;
        if (p_open && !*p_open) { m_IsOpen = false; return; }

        ImGuiIO& io = ImGui::GetIO();
        std::string title = "Code Editor: " + m_CurrentFile.filename().string();
        if (m_IsDirty) title += " *";

        ImGui::Begin(title.c_str(), &m_IsOpen, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save", "Cmd+S", false, m_IsDirty)) SaveFile();
                if (ImGui::MenuItem("Reload", nullptr, false)) ReloadFile();
                if (ImGui::MenuItem("Rename", nullptr, false)) {
                    strncpy(m_RenameBuffer, m_CurrentFile.filename().string().c_str(), sizeof(m_RenameBuffer));
                    m_ShowRenamePopup = true;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Close")) m_IsOpen = false;
                ImGui::EndMenu();
            }
            
            // Quick Buttons in MenuBar
            ImGui::Separator();
            if (ImGui::Button(m_IsDirty ? "Save *" : "Save")) SaveFile();
            ImGui::SameLine();
            if (ImGui::Button("Reload")) ReloadFile();
            
            ImGui::EndMenuBar();
        }

        // Shortcut Handling (Mac Cmd+S)
        bool cmdDown = io.ConfigMacOSXBehaviors ? io.KeyMods & ImGuiMod_Super : io.KeyMods & ImGuiMod_Ctrl;
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && cmdDown && ImGui::IsKeyPressed(ImGuiKey_S)) {
            SaveFile();
        }

        if (m_CurrentFile.empty()) {
            ImGui::Text("No file selected.");
            ImGui::End();
            return;
        }

        if (!m_EditableBuffer.empty()) {
            DrawTextEditor();
        } else if (m_ImageBuffer) {
            DrawImagePreview();
        } else if (m_CurrentFile.extension() == ".wav" || m_CurrentFile.extension() == ".mp3") {
            DrawAudioPreview();
        }

        // Rename Popup
        if (m_ShowRenamePopup) {
            ImGui::OpenPopup("Rename File");
            if (ImGui::BeginPopupModal("Rename File", &m_ShowRenamePopup)) {
                ImGui::Text("New Name:");
                ImGui::InputText("##newname", m_RenameBuffer, sizeof(m_RenameBuffer));
                if (ImGui::Button("OK", {120, 0})) {
                    RenameFile(m_RenameBuffer);
                    m_ShowRenamePopup = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", {120, 0})) m_ShowRenamePopup = false;
                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }

    void FileViewerPanel::DrawTextEditor() {
        // Toolbar
        if (ImGui::Button("Save")) SaveFile();
        ImGui::SameLine();
        if (ImGui::Button("Reload")) ReloadFile();
        ImGui::SameLine();
        if (ImGui::Button("A+")) m_FontScale += 0.1f;
        ImGui::SameLine();
        if (ImGui::Button("A-")) m_FontScale = std::max(0.5f, m_FontScale - 0.1f);
        ImGui::SameLine();
        if (ImGui::Button("Find/Replace")) m_ShowSearch = !m_ShowSearch;

        // Search & Replace Bar
        if (m_ShowSearch) {
            ImGui::Separator();
            ImGui::Text("Search:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150);
            ImGui::InputText("##search", m_SearchBuffer, sizeof(m_SearchBuffer));
            
            ImGui::SameLine();
            ImGui::Text("Replace:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150);
            ImGui::InputText("##replace", m_ReplaceBuffer, sizeof(m_ReplaceBuffer));
            
            ImGui::SameLine();
            if (ImGui::Button("Replace All")) {
                std::string content = m_EditableBuffer.data();
                std::string search = m_SearchBuffer;
                std::string replace = m_ReplaceBuffer;
                
                if (!search.empty()) {
                    size_t pos = 0;
                    int count = 0;
                    while ((pos = content.find(search, pos)) != std::string::npos) {
                        content.replace(pos, search.length(), replace);
                        pos += replace.length();
                        count++;
                    }
                    
                    if (count > 0) {
                        m_EditableBuffer.assign(content.begin(), content.end());
                        m_EditableBuffer.push_back('\0');
                        // Ensure extra capacity for editing
                        m_EditableBuffer.resize(m_EditableBuffer.size() + 10240, '\0');
                        m_IsDirty = true;
                        m_StatusMessage = std::to_string(count) + " replacements made.";
                    } else {
                        m_StatusMessage = "No matches found.";
                    }
                }
            }
            ImGui::Separator();
        }

        // Editor Area
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        
        // Calculate editor size to leave room for status bar
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        contentSize.y -= ImGui::GetTextLineHeightWithSpacing() * 1.5f;

        // Apply Font Scale
        ImGui::SetWindowFontScale(m_FontScale);
        
        // Use a monospaced font if available (usually index 0 is default, we assume it's good enough or user provides one)
        // Ideally we would push a specific font here.
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); 

        if (ImGui::InputTextMultiline("##editor", m_EditableBuffer.data(), m_EditableBuffer.size(), contentSize, flags)) {
            m_IsDirty = true;
            m_StatusMessage = "Modified";
        }
        
        ImGui::PopFont();
        ImGui::SetWindowFontScale(1.0f); // Reset scale for other UI elements

        // Status Bar
        ImGui::Separator();
        ImGui::Text("File: %s", m_CurrentFile.filename().string().c_str());
        ImGui::SameLine();
        ImGui::Text("| Size: %zu chars", strlen(m_EditableBuffer.data()));
        ImGui::SameLine();
        ImGui::Text("| Scale: %.1fx", m_FontScale);
        
        if (!m_StatusMessage.empty()) {
            ImGui::SameLine();
            ImGui::TextColored({1.0f, 1.0f, 0.0f, 1.0f}, "| %s", m_StatusMessage.c_str());
        }
    }

    void FileViewerPanel::DrawImagePreview() {
        if (m_ImageBuffer) {
            float aspect = (float)m_ImageBuffer->GetHeight() / (float)m_ImageBuffer->GetWidth();
            float width = ImGui::GetContentRegionAvail().x;
            ImGui::Image((void*)(intptr_t)m_ImageBuffer->GetRendererID(), { width, width * aspect }, {0, 1}, {1, 0});
        }
    }

    void FileViewerPanel::DrawAudioPreview() {
        ImGui::Text("Format: %s", m_CurrentFile.extension().string().c_str());
        if (ImGui::Button("Play", { 100, 40 })) {
            AudioManager::PlaySound(m_CurrentFile.string());
        }
    }

}
