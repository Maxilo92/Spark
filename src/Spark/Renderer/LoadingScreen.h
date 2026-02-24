#pragma once
#include "Window.h"
#include "ImGuiLayer.h"
#include <string>

namespace Spark {

    class LoadingScreen {
    public:
        static void Render(Window& window, ImGuiLayer& imgui, const std::string& status, float progress);
    };

}
