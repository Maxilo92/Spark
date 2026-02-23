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
        void SaveFile();
        void ReloadFile();
        void RenameFile(const std::string& newName);

    private:
        void DrawTextEditor();
        void DrawImagePreview();
        void DrawAudioPreview();

    private:
        std::filesystem::path m_CurrentFile;
        bool m_IsOpen = false;
        bool m_IsDirty = false;
        bool m_ShowRenamePopup = false;
        char m_RenameBuffer[128] = "";
        
        // Editor Buffer
        std::vector<char> m_EditableBuffer;
        std::shared_ptr<Texture2D> m_ImageBuffer;

        // Editor Settings & State
        float m_FontScale = 1.0f;
        bool m_ShowSearch = false;
        char m_SearchBuffer[256] = "";
        char m_ReplaceBuffer[256] = "";
        int m_SearchResultCount = 0;
        std::string m_StatusMessage = "";
    };

}
