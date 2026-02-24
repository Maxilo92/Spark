#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include "imgui.h"

namespace Spark {

    class Input {
    public:
        static void Init(GLFWwindow* window) { s_Window = window; }
        static void Shutdown() { s_Window = nullptr; }

        static void SetViewportFocused(bool focused) { s_ViewportFocused = focused; }

        static void UpdateKeyState(int key, bool pressed) {
            s_KeyStates[key] = pressed;
        }

        static bool IsKeyPressed(int key) {
            // If viewport is focused, we ALWAYS allow input to the game
            if (s_ViewportFocused)
                return IsKeyRaw(key);

            if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard)
                return false;
            return IsKeyRaw(key);
        }

        static bool IsKeyRaw(int key) {
            if (key < 0 || key > GLFW_KEY_LAST) return false;
            
            // 1. Direct polling
            if (s_Window) {
                auto state = glfwGetKey(s_Window, key);
                if (state == GLFW_PRESS || state == GLFW_REPEAT) return true;
            }

            // 2. Event-based state
            return s_KeyStates[key];
        }

        static bool IsMouseButtonPressed(int button) {
            if (s_ViewportFocused)
                return IsMouseButtonRaw(button);

            if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse)
                return false;
            return IsMouseButtonRaw(button);
        }

        static bool IsMouseButtonRaw(int button) {
            if (!s_Window) return false;
            auto state = glfwGetMouseButton(s_Window, button);
            return state == GLFW_PRESS;
        }

        static glm::vec2 GetMousePosition() {
            double xpos, ypos;
            glfwGetCursorPos(s_Window, &xpos, &ypos);
            return { (float)xpos, (float)ypos };
        }

    private:
        inline static GLFWwindow* s_Window = nullptr;
        inline static std::unordered_map<int, bool> s_KeyStates;
        inline static bool s_ViewportFocused = false;
    };

}
