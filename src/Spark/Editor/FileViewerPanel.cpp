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
        SP_DEBUG_TRACE("FileViewer: Opening file '" + path.string() + "'");
        m_CurrentFile = path;
        m_IsOpen = true;
        m_IsDirty = false;
        m_JustOpened = true;
        m_ImageBuffer = nullptr;
        m_EditableBuffer.clear();
        m_StatusMessage = "Loaded: " + path.filename().string();

        std::string ext = path.extension().string();
        if (ext == ".txt" || ext == ".lua" || ext == ".scene" || ext == ".yaml" || ext == ".spark" || ext == ".json") {
            std::string content = FileSystem::ReadFileText(path.string());
            m_EditableBuffer.assign(content.begin(), content.end());
            m_EditableBuffer.push_back('\0');
            // Reserviere extra Platz fuer Bearbeitung (64KB padding)
            m_EditableBuffer.resize(m_EditableBuffer.size() + 65536, '\0');
        } else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") {
            m_ImageBuffer = std::make_shared<Texture2D>(path.string());
        }
    }

    void FileViewerPanel::SaveFile() {
        if (m_CurrentFile.empty() || m_EditableBuffer.empty()) return;

        std::ofstream ofs(m_CurrentFile, std::ios::out | std::ios::trunc);
        if (ofs.is_open()) {
            ofs << m_EditableBuffer.data();
            ofs.close();
            m_IsDirty = false;
            m_StatusMessage = "Saved successfully.";
            SP_INFO("File saved: " + m_CurrentFile.filename().string());
        } else {
            m_StatusMessage = "ERROR: Could not save file!";
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
        if (m_JustOpened) {
            m_IsOpen = true;
            if (p_open) *p_open = true;
            m_JustOpened = false;
        }

        if (!m_IsOpen) {
            if (p_open) *p_open = false;
            return;
        }

        // Handle the 'X' close button or external toggle
        if (p_open && !*p_open) {
            if (m_IsDirty) {
                *p_open = true; // Stay open for confirmation
                m_ShowCloseConfirmation = true;
            } else {
                m_IsOpen = false;
            }
        }

        ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
        if (m_IsDirty) flags |= ImGuiWindowFlags_UnsavedDocument;

        std::string title = "Code Editor: " + m_CurrentFile.filename().string();
        title += "###CodeEditor";
        
        bool is_open = true;
        if (!ImGui::Begin(title.c_str(), &is_open, flags)) {
            if (!is_open) {
                if (m_IsDirty) m_ShowCloseConfirmation = true;
                else m_IsOpen = false;
            }
            ImGui::End();
            return;
        }

        if (!is_open) {
            if (m_IsDirty) m_ShowCloseConfirmation = true;
            else m_IsOpen = false;
        }

        RenderMenuBar();
        HandleShortcuts();

        if (m_CurrentFile.empty()) {
            ImGui::Text("No file selected.");
            ImGui::End();
            return;
        }

        // Calculate height needed for the status bar
        float statusBarHeight = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;

        // Content Area
        if (!m_EditableBuffer.empty()) {
            DrawTextEditor(statusBarHeight);
        } else if (m_ImageBuffer) {
            DrawImagePreview(statusBarHeight);
        } else if (m_CurrentFile.extension() == ".wav" || m_CurrentFile.extension() == ".mp3") {
            DrawAudioPreview(statusBarHeight);
        }

        // Status Bar (Always at the bottom)
        RenderStatusBar();

        // Popups
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

        if (m_ShowCloseConfirmation) {
            ImGui::OpenPopup("Save Changes?");
            if (ImGui::BeginPopupModal("Save Changes?", &m_ShowCloseConfirmation, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("The file '%s' has unsaved changes.\nDo you want to save before closing?", m_CurrentFile.filename().string().c_str());
                ImGui::Separator();

                if (ImGui::Button("Save & Close", {120, 0})) {
                    SaveFile();
                    m_IsOpen = false;
                    m_ShowCloseConfirmation = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Discard", {120, 0})) {
                    m_IsDirty = false;
                    m_IsOpen = false;
                    m_ShowCloseConfirmation = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", {120, 0})) {
                    m_ShowCloseConfirmation = false;
                }
                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }

    void FileViewerPanel::RenderMenuBar() {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save", "Cmd+S", false, m_IsDirty)) SaveFile();
                if (ImGui::MenuItem("Reload", nullptr, false)) ReloadFile();
                if (ImGui::MenuItem("Rename", nullptr, false)) {
                    strncpy(m_RenameBuffer, m_CurrentFile.filename().string().c_str(), sizeof(m_RenameBuffer));
                    m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
                    m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
                    m_ShowRenamePopup = true;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Close", "Cmd+W")) {
                    if (m_IsDirty) m_ShowCloseConfirmation = true;
                    else m_IsOpen = false;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Zoom In", "Cmd+=", false, m_FontScale < 3.0f)) m_FontScale += 0.1f;
                if (ImGui::MenuItem("Zoom Out", "Cmd+-", false, m_FontScale > 0.1f)) m_FontScale -= 0.1f;
                if (ImGui::MenuItem("Reset Zoom", "Cmd+0", false)) m_FontScale = 1.0f;
                ImGui::EndMenu();
            }
            
            ImGui::EndMenuBar();
        }
    }

    void FileViewerPanel::RenderStatusBar() {
        ImGuiIO& io = ImGui::GetIO();
        ImFont* uiFont = io.Fonts->Fonts[0];

        ImGui::PushFont(uiFont);
        ImGui::Separator();
        
        // Calculate line count if text buffer is active
        int lineCount = 0;
        size_t textLen = 0;
        if (!m_EditableBuffer.empty()) {
            textLen = strlen(m_EditableBuffer.data());
            lineCount = 1;
            for (size_t i = 0; i < textLen; i++) if (m_EditableBuffer[i] == '\n') lineCount++;
        }

        ImGui::Text("Path: %s", m_CurrentFile.string().c_str());
        
        if (lineCount > 0) {
            ImGui::SameLine();
            ImGui::Text("| Lines: %d", lineCount);
            ImGui::SameLine();
            ImGui::Text("| Size: %zu bytes", textLen);
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        if (ImGui::SliderFloat("##zoom", &m_FontScale, 0.5f, 3.0f, "Editor Zoom: %.1fx")) {
            SP_DEBUG_TRACE("UI: FileViewer 'Zoom Slider' adjusted (Scale: " + std::to_string(m_FontScale) + ")");
        }
        
        if (m_IsDirty) {
            ImGui::SameLine();
            ImGui::TextColored({1.0f, 0.4f, 0.4f, 1.0f}, "| Unsaved Changes");
        }

        if (!m_StatusMessage.empty()) {
            ImGui::SameLine();
            ImGui::TextColored({1.0f, 1.0f, 0.0f, 1.0f}, "| %s", m_StatusMessage.c_str());
        }
        ImGui::PopFont();
    }

    void FileViewerPanel::HandleShortcuts() {
        ImGuiIO& io = ImGui::GetIO();
        // Check both Super (Cmd) and Ctrl to be safe across different OS/Backend setups
        bool cmdDown = (io.KeyMods & ImGuiMod_Super) || (io.KeyMods & ImGuiMod_Ctrl);
        
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && cmdDown) {
            if (ImGui::IsKeyPressed(ImGuiKey_S)) SaveFile();
            if (ImGui::IsKeyPressed(ImGuiKey_W)) {
                if (m_IsDirty) m_ShowCloseConfirmation = true;
                else m_IsOpen = false;
            }
            
            // Zoom shortcuts: support Equal (=), Plus (+), and Keypad Add
            if (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd) || ImGui::IsKeyPressed(ImGuiKey_KeypadEqual)) {
                m_FontScale = std::min(3.0f, m_FontScale + 0.1f);
                SP_DEBUG_TRACE("Shortcut: Zoom In (New Scale: " + std::to_string(m_FontScale) + ")");
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)) {
                m_FontScale = std::max(0.5f, m_FontScale - 0.1f);
                SP_DEBUG_TRACE("Shortcut: Zoom Out (New Scale: " + std::to_string(m_FontScale) + ")");
            }
            if (ImGui::IsKeyPressed(ImGuiKey_0) || ImGui::IsKeyPressed(ImGuiKey_Keypad0)) {
                m_FontScale = 1.0f;
                SP_DEBUG_TRACE("Shortcut: Reset Zoom");
            }
        }
    }

    void FileViewerPanel::DrawTextEditor(float footerHeight) {
        ImGuiIO& io = ImGui::GetIO();
        ImFont* uiFont = io.Fonts->Fonts[0];
        ImFont* monoFont = io.Fonts->Fonts[1];

        // --- TOOLBAR ---
        ImGui::PushFont(uiFont);
        if (ImGui::Button("Save")) { SP_DEBUG_TRACE("UI: FileViewer 'Save' clicked"); SaveFile(); }
        ImGui::SameLine();
        if (ImGui::Button("Reload")) { SP_DEBUG_TRACE("UI: FileViewer 'Reload' clicked"); ReloadFile(); }
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();
        
        if (ImGui::Button("A+")) { m_FontScale = std::min(3.0f, m_FontScale + 0.1f); SP_DEBUG_TRACE("UI: FileViewer 'Zoom In' clicked (Scale: " + std::to_string(m_FontScale) + ")"); }
        ImGui::SameLine();
        if (ImGui::Button("A-")) { m_FontScale = std::max(0.1f, m_FontScale - 0.1f); SP_DEBUG_TRACE("UI: FileViewer 'Zoom Out' clicked (Scale: " + std::to_string(m_FontScale) + ")"); }
        ImGui::SameLine();
        if (ImGui::Button("Reset Zoom")) { m_FontScale = 1.0f; SP_DEBUG_TRACE("UI: FileViewer 'Reset Zoom' clicked"); }
        
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();
        if (ImGui::Button(m_ShowSearch ? "Hide Find" : "Find/Replace")) {
            m_ShowSearch = !m_ShowSearch;
            SP_DEBUG_TRACE("UI: FileViewer 'Find/Replace' toggled (" + std::string(m_ShowSearch ? "ON" : "OFF") + ")");
        }
        ImGui::PopFont();

        // --- SEARCH & REPLACE BAR ---
        if (m_ShowSearch) {
            ImGui::PushFont(uiFont);
            ImGui::Separator();
            
            ImGui::Text("Find:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150);
            if (ImGui::InputText("##search", m_SearchBuffer, sizeof(m_SearchBuffer))) {
                // Live count matches
                std::string content = m_EditableBuffer.data();
                std::string search = m_SearchBuffer;
                m_SearchResultCount = 0;
                if (!search.empty()) {
                    size_t pos = 0;
                    while ((pos = content.find(search, pos)) != std::string::npos) {
                        pos += search.length();
                        m_SearchResultCount++;
                    }
                }
            }
            
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
                        m_EditableBuffer.resize(std::max(m_EditableBuffer.size(), (size_t)1024), '\0');
                        m_IsDirty = true;
                        m_StatusMessage = "Replaced " + std::to_string(count) + " occurrences.";
                    }
                }
            }
            
            if (m_SearchResultCount > 0) {
                ImGui::SameLine();
                ImGui::TextColored({0.4f, 1.0f, 0.4f, 1.0f}, "%d matches", m_SearchResultCount);
            }

            ImGui::Separator();
            ImGui::PopFont();
        }

        ImVec2 editorSize = ImGui::GetContentRegionAvail();
        editorSize.y -= footerHeight; // Reserve space for the footer
        
        ImGui::BeginChild("EditorTextArea", editorSize, true, ImGuiWindowFlags_HorizontalScrollbar);
        
        // This only scales the editor content inside the child
        ImGui::SetWindowFontScale(m_FontScale);
        
        ImGui::PushFont(monoFont);
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        
        ImVec2 innerSize = ImGui::GetContentRegionAvail();
        if (ImGui::InputTextMultiline("##editor", m_EditableBuffer.data(), m_EditableBuffer.size(), innerSize, flags)) {
            if (!m_IsDirty) {
                m_IsDirty = true;
                m_StatusMessage = "Modified";
            }
        }
        
        ImGui::PopFont();
        ImGui::SetWindowFontScale(1.0f); // Reset scale
        ImGui::EndChild();
    }

    void FileViewerPanel::DrawImagePreview(float footerHeight) {
        if (m_ImageBuffer && m_ImageBuffer->GetWidth() > 0) {
            float aspect = (float)m_ImageBuffer->GetHeight() / (float)m_ImageBuffer->GetWidth();
            ImVec2 avail = ImGui::GetContentRegionAvail();
            avail.y -= footerHeight;
            
            float width = avail.x;
            float height = width * aspect;
            
            // If image is too tall, scale it down to fit the available height
            if (height > avail.y) {
                height = avail.y;
                width = height / aspect;
            }
            
            ImGui::Image((void*)(intptr_t)m_ImageBuffer->GetRendererID(), { width, height }, {0, 1}, {1, 0});
        }
    }

    void FileViewerPanel::DrawAudioPreview(float footerHeight) {
        ImGui::Text("Format: %s", m_CurrentFile.extension().string().c_str());
        if (ImGui::Button("Play", { 100, 40 })) {
            AudioManager::PlaySound(m_CurrentFile.string());
        }
    }

}
