#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unordered_map>

namespace Spark {

    class Input {
    public:
        static void Init(GLFWwindow* window) { s_Window = window; }
        static void Shutdown() { s_Window = nullptr; }

        static void UpdateKeyState(int key, bool pressed) {
            s_KeyStates[key] = pressed;
        }

        static bool IsKeyPressed(int key) {
            if (key < 0 || key > GLFW_KEY_LAST) return false;
            
            // 1. Check internal state map (Event-based)
            if (s_KeyStates[key]) return true;

            // 2. Fallback: Poll GLFW directly (Window state)
            if (s_Window) {
                auto state = glfwGetKey(s_Window, key);
                return state == GLFW_PRESS || state == GLFW_REPEAT;
            }

            return false;
        }

        static bool IsMouseButtonPressed(int button) {
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
    };

}
