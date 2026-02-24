#include "ConsolePanel.h"
#include "imgui.h"
#include <string.h>

namespace Spark {

    ConsolePanel::ConsolePanel() {}

    void ConsolePanel::OnImGuiRender(bool* p_open) {
        if (p_open && !*p_open) return;

        ImGui::SetNextWindowSize({ 600, 400 }, ImGuiCond_FirstUseEver);
        ImGui::Begin("Console", p_open);

        // Toolbar
        if (ImGui::Button("Clear")) Log::Clear();
        ImGui::SameLine();
        if (ImGui::Button("Copy All")) {
            std::string log;
            for (const auto& msg : Log::GetMessages()) {
                log += "[" + msg.Timestamp + "] " + msg.Message + "\n";
            }
            ImGui::SetClipboardText(log.c_str());
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto-Scroll", &m_AutoScroll);
        
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        
        // Filter Buttons
        ImGui::Checkbox("Info", &m_FilterInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warn", &m_FilterWarning);
        ImGui::SameLine();
        ImGui::Checkbox("Err", &m_FilterError);

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        ImGui::InputTextWithHint("##Search", "Search...", m_SearchFilter, sizeof(m_SearchFilter));

        ImGui::Separator();

        // Main Scrolling Region
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar)) {
            
            const auto& messages = Log::GetMessages();
            for (const auto& msg : messages) {
                // Apply Filters
                if (!m_FilterInfo && (msg.Level == LogLevel::Info || msg.Level == LogLevel::Trace)) continue;
                if (!m_FilterWarning && msg.Level == LogLevel::Warning) continue;
                if (!m_FilterError && (msg.Level == LogLevel::Error || msg.Level == LogLevel::Critical)) continue;
                
                // Search Filter
                if (strlen(m_SearchFilter) > 0 && msg.Message.find(m_SearchFilter) == std::string::npos) continue;

                ImVec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
                const char* levelStr = "[INFO]";

                switch (msg.Level) {
                    case LogLevel::Warning:  color = { 1.0f, 0.8f, 0.0f, 1.0f }; levelStr = "[WARN]"; break;
                    case LogLevel::Error:    color = { 1.0f, 0.3f, 0.3f, 1.0f }; levelStr = "[ ERR]"; break;
                    case LogLevel::Critical: color = { 1.0f, 0.0f, 1.0f, 1.0f }; levelStr = "[CRIT]"; break;
                    case LogLevel::Trace:    color = { 0.6f, 0.6f, 0.6f, 1.0f }; levelStr = "[TRAC]"; break;
                    default: break;
                }

                ImGui::TextDisabled("[%s]", msg.Timestamp.c_str());
                ImGui::SameLine();
                ImGui::TextColored(color, "%s", levelStr);
                ImGui::SameLine();
                
                ImGui::PushTextWrapPos(0.0f);
                ImGui::TextUnformatted(msg.Message.c_str());
                ImGui::PopTextWrapPos();
            }

            if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
        ImGui::Separator();

        // Eingabefeld
        static char inputBuffer[256] = "";
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue;
        
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("##Command", inputBuffer, sizeof(inputBuffer), input_flags)) {
            std::string cmd(inputBuffer);
            if (!cmd.empty()) {
                Log::ExecuteCommand(cmd);
            }
            strcpy(inputBuffer, "");
            reclaim_focus = true;
        }
        ImGui::PopItemWidth();

        // Fokus automatisch auf das Eingabefeld setzen
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus) ImGui::SetKeyboardFocusHere(-1);

        ImGui::End();
    }

}
