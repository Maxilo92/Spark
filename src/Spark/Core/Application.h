#pragma once
#include "Window.h"
#include "Events.h"
#include "LayerStack.h"
#include "ImGuiLayer.h"
#include <memory>

class Application {
public:
    Application();
    ~Application();

    void Run();
    void OnEvent(Event& e);

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);

private:
    std::unique_ptr<Window> m_Window;
    ImGuiLayer* m_ImGuiLayer;
    bool m_Running = true;
    LayerStack m_LayerStack;
};
