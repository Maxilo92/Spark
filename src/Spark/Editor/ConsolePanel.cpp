#include "ConsolePanel.h"
#include "imgui.h"
#include <string.h>

namespace Spark {

    ConsolePanel::ConsolePanel() {}

    void ConsolePanel::OnImGuiRender(bool* p_open) {
        if (p_open && !*p_open) return;

        ImGui::Begin("Console", p_open);

        if (ImGui::Button("Clear")) {
            Log::Clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto-Scroll", &m_AutoScroll);

        ImGui::Separator();

        // Reserviere Platz für Eingabefeld unten
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

        const auto& messages = Log::GetMessages();
        for (const auto& msg : messages) {
            ImVec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

            switch (msg.Level) {
                case LogLevel::Warning: color = { 1.0f, 0.8f, 0.0f, 1.0f }; break;
                case LogLevel::Error:   color = { 1.0f, 0.2f, 0.2f, 1.0f }; break;
                case LogLevel::Critical:color = { 1.0f, 0.0f, 1.0f, 1.0f }; break;
                case LogLevel::Trace:   color = { 0.6f, 0.6f, 0.6f, 1.0f }; break;
                default: break;
            }

            ImGui::TextDisabled("[%s]", msg.Timestamp.c_str());
            ImGui::SameLine();
            ImGui::TextColored(color, "%s", msg.Message.c_str());
        }

        if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

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
