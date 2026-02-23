#include "InputTestLayer.h"
#include "imgui.h"
#include "Input.h"
#include "Log.h"

void InputTestLayer::OnImGuiRender() {
    ImGui::Begin("Input Test Panel");
    
    // Mouse
    ImVec2 mousePos = ImGui::GetMousePos();
    ImGui::Text("ImGui Mouse Pos: %.1f, %.1f", mousePos.x, mousePos.y);
    ImGui::Text("Mouse Left Down: %s", ImGui::IsMouseDown(0) ? "YES" : "No");
    
    // Spark Input (GLFW)
    ImGui::Separator();
    ImGui::Text("Spark Input System (GLFW):");
    ImGui::Text("W (87): %s", Spark::Input::IsKeyPressed(87) ? "DOWN" : "up");
    ImGui::Text("A (65): %s", Spark::Input::IsKeyPressed(65) ? "DOWN" : "up");
    ImGui::Text("S (83): %s", Spark::Input::IsKeyPressed(83) ? "DOWN" : "up");
    ImGui::Text("D (68): %s", Spark::Input::IsKeyPressed(68) ? "DOWN" : "up");
    
    if (ImGui::Button("Log Test")) {
        SP_INFO("Input Test Button Clicked");
    }
    
    ImGui::End();
}
