#include "MainMenuLayer.h"
#include "ProjectManager.h"
#include "Application.h"
#include "MacOSUtils.h"
#include <imgui.h>
#include <filesystem>

namespace Spark {

    MainMenuLayer::MainMenuLayer() : Layer("MainMenuLayer") {}

    void MainMenuLayer::OnAttach() {
        ProjectManager::RefreshRecentProjects();
    }

    void MainMenuLayer::OnDetach() {}
    void MainMenuLayer::OnUpdate(float dt) {}

    void MainMenuLayer::OnImGuiRender() {
        // Set main menu to fill the window
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::Begin("Spark Engine - Project Manager", nullptr, 
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // Center Column
        float columnWidth = 600.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - columnWidth) / 2.0f);
        ImGui::BeginChild("CenterColumn", ImVec2(columnWidth, 0));

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::SetWindowFontScale(2.5f);
        ImGui::TextColored({ 0.2f, 0.6f, 1.0f, 1.0f }, "SPARK ENGINE");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopFont();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // --- NEW PROJECT SECTION ---
        if (ImGui::CollapsingHeader("Start New Project", ImGuiTreeNodeFlags_DefaultOpen)) {
            static char projectName[128] = "MyNewGame";
            static char projectPath[256] = ""; // Should probably default to Documents or similar
            
            if (strlen(projectPath) == 0) {
                strcpy(projectPath, std::filesystem::current_path().string().c_str());
            }

            ImGui::InputText("Project Name", projectName, IM_ARRAYSIZE(projectName));
            ImGui::InputText("Location", projectPath, IM_ARRAYSIZE(projectPath));
            
            if (ImGui::Button("Create Project", ImVec2(columnWidth, 40))) {
                std::filesystem::path fullPath = std::filesystem::path(projectPath) / projectName;
                if (ProjectManager::NewProject(projectName, fullPath)) {
                    Application::Get().SetState(ApplicationState::PROJECT_LOADING);
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // --- DEMOS SECTION ---
        if (ImGui::CollapsingHeader("Sample Demos", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BeginChild("Demos", ImVec2(0, 80), true);
            
            ImGui::Text("The Spark Adventure");
            ImGui::TextDisabled("A comprehensive demo of Physics, Scripting, and Audio.");
            
            ImGui::SameLine(ImGui::GetWindowWidth() - 80);
            if (ImGui::Button("Launch", ImVec2(70, 40))) {
                if (ProjectManager::OpenProject("Adventure.spark")) {
                    Application::Get().SetState(ApplicationState::PROJECT_LOADING);
                }
            }
            ImGui::EndChild();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // --- RECENT PROJECTS SECTION ---
        ImGui::Text("Recent Projects");
        ImGui::SameLine(columnWidth - 100);
        if (ImGui::Button("Browse...", ImVec2(100, 0))) {
            std::string path = MacOSUtils::OpenFile("spark");
            if (!path.empty()) {
                if (ProjectManager::OpenProject(path)) {
                    Application::Get().SetState(ApplicationState::PROJECT_LOADING);
                }
            }
        }
        ImGui::Spacing();

        const auto& recent = ProjectManager::GetRecentProjects();
        if (recent.empty()) {
            ImGui::TextDisabled("No recent projects found.");
        } else {
            for (const auto& rp : recent) {
                ImGui::PushID(rp.Path.c_str());
                
                ImGui::BeginChild(rp.Path.c_str(), ImVec2(0, 60), true);
                
                ImGui::Text("%s", rp.Name.c_str());
                ImGui::TextDisabled("%s", rp.Path.c_str());
                
                ImGui::SameLine(ImGui::GetWindowWidth() - 150);
                if (ImGui::Button("Open", ImVec2(60, 40))) {
                    if (ProjectManager::OpenProject(rp.Path)) {
                        Application::Get().SetState(ApplicationState::PROJECT_LOADING);
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove", ImVec2(60, 40))) {
                    ProjectManager::RemoveRecentProject(rp.Path);
                }
                
                ImGui::EndChild();
                ImGui::PopID();
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }

}
