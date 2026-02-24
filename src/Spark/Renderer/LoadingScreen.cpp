#include "LoadingScreen.h"
#include "Version.h"
#include "Log.h"
#include <imgui.h>
#include <algorithm>

namespace Spark {

    void LoadingScreen::Render(Window& window, ImGuiLayer& imgui, const std::string& status, float progress) {
        window.Clear();
        imgui.Begin();

        ImGui::SetNextWindowPos({ 0, 0 });
        ImGui::SetNextWindowSize({ (float)window.GetWidth(), (float)window.GetHeight() });
        
        ImGui::Begin("SplashScreen", nullptr, 
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove);

        // Center content
        float windowWidth = ImGui::GetWindowWidth();
        float windowHeight = ImGui::GetWindowHeight();

        // Title
        ImGui::SetCursorPosY(windowHeight * 0.3f);
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::SetWindowFontScale(4.0f);
        float textWidth = ImGui::CalcTextSize("SPARK ENGINE").x;
        ImGui::SetCursorPosX((windowWidth - textWidth) / 2.0f);
        ImGui::TextColored({ 0.2f, 0.6f, 1.0f, 1.0f }, "SPARK ENGINE");

        // Version under title
        ImGui::SetWindowFontScale(1.5f);
        std::string verText = "Version " + std::string(SPARK_VERSION_STR);
        float verWidth = ImGui::CalcTextSize(verText.c_str()).x;
        ImGui::SetCursorPosX((windowWidth - verWidth) / 2.0f);
        ImGui::TextDisabled("%s", verText.c_str());
        ImGui::SetWindowFontScale(1.0f); // Reset scale
        ImGui::PopFont();

        // Progress Bar
        ImGui::SetCursorPosY(windowHeight * 0.6f);
        ImGui::SetCursorPosX(windowWidth * 0.2f);
        ImGui::ProgressBar(progress, ImVec2(windowWidth * 0.6f, 30.0f));

        // Status Text
        float statusTextWidth = ImGui::CalcTextSize(status.c_str()).x;
        ImGui::SetCursorPosX((windowWidth - statusTextWidth) / 2.0f);
        ImGui::Text("%s", status.c_str());

        // Verbose Log (only if enabled)
        if (Spark::Log::IsVerbose()) {
            ImGui::SetCursorPosY(windowHeight * 0.75f);
            ImGui::BeginChild("LoadingLog", { windowWidth * 0.8f, 100.0f }, true);
            const auto& messages = Spark::Log::GetMessages();
            int start = std::max(0, (int)messages.size() - 5);
            for (int i = start; i < (int)messages.size(); i++) {
                ImGui::TextDisabled("[%s] %s", messages[i].Timestamp.c_str(), messages[i].Message.c_str());
            }
            ImGui::EndChild();
        }

        ImGui::End();
        imgui.End();
        window.OnUpdate();
    }

}
