#pragma once
#include <filesystem>
#include <map>
#include <memory>
#include "FileViewerPanel.h"
#include "Texture.h"

namespace Spark {

    class ContentBrowserPanel {
    public:
        ContentBrowserPanel();

        void OnImGuiRender(FileViewerPanel& viewer, bool* p_open = nullptr);

    private:
        void DrawMenuBar(FileViewerPanel& viewer);
        void HandleShortcuts();
        std::shared_ptr<Texture2D> GetOrCreateThumbnail(const std::filesystem::path& path);

        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_SelectedFile;
        std::filesystem::path m_ItemToRename;
        char m_RenameBuffer[128] = "";
        bool m_ShowRenamePopup = false;
        
        std::filesystem::path m_ItemToDelete;
        bool m_ShowDeleteModal = false;

        std::filesystem::path m_DraggedItem;

        // Simple cache to prevent reloading textures every frame
        std::map<std::string, std::shared_ptr<Texture2D>> m_ThumbnailCache;
    };

}
