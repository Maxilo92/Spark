#pragma once
#include <filesystem>
#include "FileViewerPanel.h"

namespace Spark {

    class ContentBrowserPanel {
    public:
        ContentBrowserPanel();

        void OnImGuiRender(FileViewerPanel& viewer, bool* p_open = nullptr);

    private:
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_SelectedFile;
    };

}
