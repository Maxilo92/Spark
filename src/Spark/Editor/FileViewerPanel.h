#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <memory>
#include "Texture.h"

namespace Spark {

    class FileViewerPanel {
    public:
        FileViewerPanel();
        
        void OnImGuiRender(bool* p_open = nullptr);
        void OpenFile(const std::filesystem::path& path);
        bool IsOpen() const { return m_IsOpen; }
        void SaveFile();
        void SaveFileAs(const std::filesystem::path& path);
        void ReloadFile();
        void RenameFile(const std::string& newName);

    private:
        void DrawTextEditor(float footerHeight);
        void DrawImagePreview(float footerHeight);
        void DrawAudioPreview(float footerHeight);

        void RenderMenuBar();
        void RenderStatusBar();
        void HandleShortcuts();

    private:
        std::filesystem::path m_CurrentFile;
        bool m_IsOpen = false;
        bool m_IsDirty = false;
        bool m_JustOpened = false;
        bool m_ShowRenamePopup = false;
        bool m_ShowSaveAsPopup = false;
        bool m_ShowCloseConfirmation = false;
        char m_RenameBuffer[128] = "";
        char m_SaveAsBuffer[128] = "";
        
        // Editor Buffer
        std::vector<char> m_EditableBuffer;
        std::shared_ptr<Texture2D> m_ImageBuffer;

        // Editor Settings & State
        int m_FontSize = 18;
        bool m_ShowSearch = false;
        char m_SearchBuffer[256] = "";
        char m_ReplaceBuffer[256] = "";
        int m_SearchResultCount = 0;
        int m_SearchResultIndex = 0;
        std::string m_StatusMessage = "";
    };

}
