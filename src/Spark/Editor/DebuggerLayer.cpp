#include "DebuggerLayer.h"
#include "imgui.h"
#include <GLFW/glfw3.h>

void DebuggerLayer::OnImGuiRender() {
    // Hauptmenüleiste
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Windows")) {
            ImGui::MenuItem("Debugger", NULL, &m_ShowDebuggerWindow);
            ImGui::MenuItem("Engine Info", NULL, &m_ShowEngineInfo);
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

        if (ImGui::CollapsingHeader("Window Management")) {
            ImGui::Checkbox("Show Engine Info", &m_ShowEngineInfo);
            ImGui::Checkbox("Show ImGui Demo", &m_ShowDemoWindow);
        }

        if (ImGui::CollapsingHeader("Backend Info")) {
            ImGui::Text("GLFW version: %s", glfwGetVersionString());
            // OpenGL Infos sind global verfügbar
            ImGui::Text("OpenGL version: %s", (const char*)glGetString(GL_VERSION));
            ImGui::Text("Renderer: %s", (const char*)glGetString(GL_RENDERER));
        }

        ImGui::End();
    }

    // Dein Engine UI
    if (m_ShowEngineInfo) {
        ImGui::Begin("Engine Info", &m_ShowEngineInfo);
        ImGui::Text("Hallo vom Intel Mac!");
        ImGui::Separator();
        ImGui::Text("Hier können weitere Engine-Daten stehen.");
        ImGui::End();
    }
}
