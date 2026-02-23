#include "Application.h"
#include "DebuggerLayer.h"
#include "TriangleLayer.h"
#include "InputTestLayer.h"
#include "ScriptEngine.h"
#include "AudioManager.h"
#include "FileSystem.h"
#include "AssetManager.h"
#include "InstanceLock.h"
#include "Renderer2D.h"
#include "CrashHandler.h"
#include "MacOSUtils.h"
#include "Log.h"
#include "Input.h"
#include <iostream>

Application::Application() {
    Spark::Log::Init();
    Spark::CrashHandler::Init();
    SP_INFO("Initializing Spark Application...");

    // Sicherstellen, dass nur eine Instanz läuft
    Spark::InstanceLock::Lock();

    m_Window = std::make_unique<Window>(1280, 720, "Spark Engine");
    m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

    // Native macOS Menu Hook
    Spark::MacOSUtils::SetupApplicationMenu();

    Spark::Input::Init(m_Window->GetNativeWindow());

    Spark::FileSystem::Init();
    Spark::AssetManager::Init();
    Spark::Renderer2D::Init();
    ScriptEngine::Init();
    Spark::AudioManager::Init();

    m_ImGuiLayer = new ImGuiLayer(*m_Window);
    PushOverlay(m_ImGuiLayer);
    
    // Debugger Layer hinzufügen
    PushLayer(new DebuggerLayer());
    
    // Test Layer hinzufügen
    PushLayer(new InputTestLayer());
    
    // Haupt Editor Layer
    PushLayer(new TriangleLayer());
}

Application::~Application() {
    Spark::AudioManager::Shutdown();
    Spark::Renderer2D::Shutdown();
    Spark::AssetManager::Shutdown();
    Spark::Input::Shutdown();
    Spark::InstanceLock::Unlock();
    Spark::Log::Shutdown();
}

void Application::PushLayer(Layer* layer) {
    m_LayerStack.PushLayer(layer);
}

void Application::PushOverlay(Layer* overlay) {
    m_LayerStack.PushOverlay(overlay);
}

void Application::OnEvent(Event& e) {
    if (e.GetType() == EventType::WindowClose) {
        m_Running = false;
        e.Handled = true;
    }

    if (e.GetType() == EventType::KeyPressed) {
        auto& ke = static_cast<KeyPressedEvent&>(e);
        Spark::Input::UpdateKeyState(ke.KeyCode, true);
    }
    if (e.GetType() == EventType::KeyReleased) {
        auto& ke = static_cast<KeyReleasedEvent&>(e);
        Spark::Input::UpdateKeyState(ke.KeyCode, false);
    }

    // Events durch den LayerStack leiten (von oben nach unten)
    for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) {
        (*--it)->OnEvent(e);
        if (e.Handled) break;
    }
}

void Application::Run() {
    float lastFrameTime = 0.0f;
    while (m_Running) {
        float time = (float)glfwGetTime();
        float dt = time - lastFrameTime;
        lastFrameTime = time;

        // Fenster leeren
        m_Window->Clear();

        // 1. Layer-Updates (Spiellogik, Physik, Rendering)
        for (Layer* layer : m_LayerStack) {
            layer->OnUpdate(dt);
        }

        // 2. ImGui-Rendering
        m_ImGuiLayer->Begin();
        for (Layer* layer : m_LayerStack) {
            layer->OnImGuiRender();
        }
        m_ImGuiLayer->End();

        // Fenster aktualisieren
        m_Window->OnUpdate();
    }
}
