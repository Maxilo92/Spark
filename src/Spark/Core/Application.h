#pragma once
#include "Window.h"
#include "Events.h"
#include "LayerStack.h"
#include "ImGuiLayer.h"
#include <memory>

enum class ApplicationState {
    STARTUP,
    MAIN_MENU,
    PROJECT_LOADING,
    EDITOR
};

class Application {
public:
    Application();
    ~Application();

    static Application& Get() { return *s_Instance; }

    void Run();
    void OnEvent(Event& e);

    void Restart(bool verbose = false);
    void RebuildAndRestart();
    void UpdateWindowTitle();

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);

    void SetState(ApplicationState newState);
    ApplicationState GetState() const { return m_State; }

private:
    void HandleStateTransitions();
    void SaveWindowSettings();
    void LoadWindowSettings();

private:
    static Application* s_Instance;
    std::unique_ptr<Window> m_Window;
    ImGuiLayer* m_ImGuiLayer;
    bool m_Running = true;
    
    ApplicationState m_State = ApplicationState::STARTUP;
    ApplicationState m_NextState = ApplicationState::STARTUP;
    bool m_StateChanged = false;
    LayerStack m_LayerStack;
};
