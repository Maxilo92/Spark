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

    static Application& Get() { return *s_Instance; }

    void Run();
    void OnEvent(Event& e);

    void Restart(bool verbose = false);
    void UpdateWindowTitle();

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);

private:
    void RenderSplashScreen();
    void SetLoadingStatus(const std::string& status, float progress);

private:
    static Application* s_Instance;
    std::unique_ptr<Window> m_Window;
    ImGuiLayer* m_ImGuiLayer;
    bool m_Running = true;
    bool m_IsLoading = true;
    float m_LoadingProgress = 0.0f;
    std::string m_LoadingStatus = "Starting...";
    LayerStack m_LayerStack;
};
