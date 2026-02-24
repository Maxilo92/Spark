#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace Spark {

    struct ChangelogSection {
        std::string Title;
        std::vector<std::string> Items;
    };

    struct ChangelogVersion {
        std::string Version;
        std::string Date;
        std::vector<ChangelogSection> Sections;
    };

    class ChangelogPanel {
    public:
        ChangelogPanel();
        void OnImGuiRender(bool* p_open);
        
        bool LoadChangelog(const std::filesystem::path& path);
        void SetVersionFilter(const std::string& version) { m_VersionFilter = version; }

    private:
        std::vector<ChangelogVersion> m_Versions;
        std::string m_VersionFilter;
    };

}
