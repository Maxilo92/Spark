#include "DebuggerLayer.h"
#define GL_SILENCE_DEPRECATION
#include "imgui.h"
#include "Application.h"
#include "Input.h"
#include "Log.h"
#include <GLFW/glfw3.h>

#include <yaml-cpp/yaml.h>
#include <fstream>

void DebuggerLayer::OnAttach() {
    LoadEditorSettings();
}

void DebuggerLayer::OnDetach() {
    SaveEditorSettings();
}

void DebuggerLayer::SaveEditorSettings() {
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "DebuggerVisibility" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "ShowDebugger" << YAML::Value << m_ShowDebuggerWindow;
    out << YAML::Key << "ShowDemo" << YAML::Value << m_ShowDemoWindow;
    out << YAML::EndMap;
    out << YAML::EndMap;

    std::ofstream fout("debugger_settings.yaml");
    fout << out.c_str();
}

void DebuggerLayer::LoadEditorSettings() {
    std::ifstream stream("debugger_settings.yaml");
    if (!stream.is_open()) return;

    try {
        YAML::Node data = YAML::Load(stream);
        auto visibility = data["DebuggerVisibility"];
        if (visibility) {
            if (visibility["ShowDebugger"]) m_ShowDebuggerWindow = visibility["ShowDebugger"].as<bool>();
            if (visibility["ShowDemo"]) m_ShowDemoWindow = visibility["ShowDemo"].as<bool>();
        }
    } catch (const std::exception& e) {
        // Silently fail if settings are corrupted
    }
}

void DebuggerLayer::OnEvent(Event& e) {
    if (e.GetType() == EventType::ProjectLoaded) {
        LoadEditorSettings();
    } else if (e.GetType() == EventType::ProjectSaved) {
        SaveEditorSettings();
    }
}

void DebuggerLayer::OnImGuiRender() {
    // Track window visibility changes
    static bool lastDebuggerVisible = m_ShowDebuggerWindow;
    static bool lastDemoVisible = m_ShowDemoWindow;

    if (m_ShowDebuggerWindow != lastDebuggerVisible) {
        SP_DEBUG_TRACE("UI: Window 'Debugger' " + std::string(m_ShowDebuggerWindow ? "opened" : "closed"));
        lastDebuggerVisible = m_ShowDebuggerWindow;
    }
    if (m_ShowDemoWindow != lastDemoVisible) {
        SP_DEBUG_TRACE("UI: Window 'ImGui Demo' " + std::string(m_ShowDemoWindow ? "opened" : "closed"));
        lastDemoVisible = m_ShowDemoWindow;
    }

    // Hauptmenüleiste
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Windows")) {
            ImGui::MenuItem("Debugger", NULL, &m_ShowDebuggerWindow);
            ImGui::MenuItem("ImGui Demo", NULL, &m_ShowDemoWindow);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Demo-Fenster (aus imgui_demo.cpp)
    if (m_ShowDemoWindow) ImGui::ShowDemoWindow(&m_ShowDemoWindow);

    // Debugger Fenster
    if (m_ShowDebuggerWindow) {
        ImGui::Begin("Debugger", &m_ShowDebuggerWindow);
        
        if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            
            // Ein einfacher Graph für die FPS
            static float values[90] = { 0 };
            static int values_offset = 0;
            static double refresh_time = 0.0;
            if (ImGui::GetTime() > refresh_time) {
                values[values_offset] = ImGui::GetIO().Framerate;
                values_offset = (values_offset + 1) % 90;
                refresh_time = ImGui::GetTime() + 1.0f / 30.0f;
            }
            ImGui::PlotLines("FPS", values, IM_ARRAYSIZE(values), values_offset, NULL, 0.0f, 150.0f, ImVec2(0, 80));
        }

        if (ImGui::CollapsingHeader("Input Diagnostics")) {
            // Mouse
            ImVec2 mousePos = ImGui::GetMousePos();
            ImGui::Text("Mouse Pos: %.1f, %.1f", mousePos.x, mousePos.y);
            ImGui::Text("Left Mouse: %s", ImGui::IsMouseDown(0) ? "DOWN" : "UP");
            
            ImGui::Separator();
            ImGui::Text("Key State (Spark/GLFW):");
            ImGui::Columns(2, "KeyCols");
            ImGui::Text("W (87): %s", Spark::Input::IsKeyPressed(87) ? "DOWN" : "up");
            ImGui::Text("S (83): %s", Spark::Input::IsKeyPressed(83) ? "DOWN" : "up");
            ImGui::NextColumn();
            ImGui::Text("A (65): %s", Spark::Input::IsKeyPressed(65) ? "DOWN" : "up");
            ImGui::Text("D (68): %s", Spark::Input::IsKeyPressed(68) ? "DOWN" : "up");
            ImGui::Columns(1);
        }

        if (ImGui::CollapsingHeader("Window Management")) {
            ImGui::Checkbox("Show ImGui Demo", &m_ShowDemoWindow);
        }

        if (ImGui::CollapsingHeader("Backend Info")) {
            ImGui::Text("GLFW version: %s", glfwGetVersionString());
            // OpenGL Infos sind global verfügbar
            ImGui::Text("OpenGL version: %s", (const char*)glGetString(GL_VERSION));
            ImGui::Text("Renderer: %s", (const char*)glGetString(GL_RENDERER));
        }

        if (ImGui::CollapsingHeader("Diagnostics", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool isVerbose = Spark::Log::IsVerbose();
            ImGui::Text("Verbose Mode: ");
            ImGui::SameLine();
            ImGui::TextColored(isVerbose ? ImVec4{ 0.2f, 0.8f, 0.2f, 1.0f } : ImVec4{ 0.8f, 0.2f, 0.2f, 1.0f },
                isVerbose ? "ACTIVE" : "OFF");

            if (ImGui::Checkbox("Runtime Verbose", &isVerbose)) {
                Spark::Log::SetVerbose(isVerbose);
                Application::Get().UpdateWindowTitle();
            }

            ImGui::Separator();
            ImGui::Text("Full Startup Logging:");
            if (ImGui::Button("Restart in Verbose Mode")) {
                Application::Get().Restart(true);
            }
            if (isVerbose) {
                if (ImGui::Button("Restart (Normal Mode)")) {
                    Application::Get().Restart(false);
                }
            }

            ImGui::TextDisabled("Restarts the engine to capture logs from the very first frame.");
        }

        ImGui::End();
    }
}
