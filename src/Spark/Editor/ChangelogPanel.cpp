#include "ChangelogPanel.h"
#include <imgui.h>
#include <fstream>
#include <sstream>
#include "Log.h"

namespace Spark {

    ChangelogPanel::ChangelogPanel() {}

    void ChangelogPanel::OnImGuiRender(bool* p_open) {
        if (!*p_open) return;

        ImGui::SetNextWindowSize({ 600, 400 }, ImGuiCond_FirstUseEver);
        if (ImGui::Begin("What's New in Spark?", p_open)) {
            for (const auto& ver : m_Versions) {
                // If a filter is set, only show the filtered version
                if (!m_VersionFilter.empty() && ver.Version != m_VersionFilter)
                    continue;

                ImGui::PushStyleColor(ImGuiCol_Text, { 0.2f, 0.7f, 1.0f, 1.0f });
                ImGui::Text("Version %s (%s)", ver.Version.c_str(), ver.Date.c_str());
                ImGui::PopStyleColor();
                ImGui::Separator();

                for (const auto& section : ver.Sections) {
                    ImGui::Spacing();
                    ImGui::TextDisabled("%s", section.Title.c_str());
                    for (const auto& item : section.Items) {
                        ImGui::Bullet();
                        ImGui::PushTextWrapPos(0.0f);
                        ImGui::TextUnformatted(item.c_str());
                        ImGui::PopTextWrapPos();
                    }
                }
                ImGui::Spacing();
                ImGui::Spacing();
            }

            ImGui::Separator();
            if (ImGui::Button("Close", { 120, 0 })) {
                *p_open = false;
            }
        }
        ImGui::End();
    }

    bool ChangelogPanel::LoadChangelog(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            SP_ERROR("Could not open changelog: " + path.string());
            return false;
        }

        m_Versions.clear();
        std::string line;
        ChangelogVersion* currentVersion = nullptr;
        ChangelogSection* currentSection = nullptr;

        while (std::getline(file, line)) {
            if (line.empty()) continue;

            // Detect Version Header: ## [0.1.0] - 2026-02-24
            if (line.compare(0, 3, "## ") == 0 && line.find('[') != std::string::npos) {
                size_t start = line.find('[') + 1;
                size_t end = line.find(']');
                if (start != std::string::npos && end != std::string::npos) {
                    m_Versions.emplace_back();
                    currentVersion = &m_Versions.back();
                    currentVersion->Version = line.substr(start, end - start);
                    
                    size_t dateStart = line.find(" - ");
                    if (dateStart != std::string::npos) {
                        currentVersion->Date = line.substr(dateStart + 3);
                    }
                    currentSection = nullptr;
                }
            }
            // Detect Section: ### Added
            else if (line.compare(0, 4, "### ") == 0 && currentVersion) {
                currentVersion->Sections.emplace_back();
                currentSection = &currentVersion->Sections.back();
                currentSection->Title = line.substr(4);
            }
            // Detect Bullet: - Added versioning
            else if (line.compare(0, 2, "- ") == 0 && currentSection) {
                currentSection->Items.push_back(line.substr(2));
            }
        }

        return !m_Versions.empty();
    }

}
