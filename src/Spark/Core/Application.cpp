#include "Application.h"
#include "Version.h"
#include "DebuggerLayer.h"
#include "TriangleLayer.h"
#include "MainMenuLayer.h"
#include "ScriptEngine.h"
#include "AudioManager.h"
#include "FileSystem.h"
#include "AssetManager.h"
#include "ProjectManager.h"
#include "InstanceLock.h"
#include "Renderer2D.h"
#include "LoadingScreen.h"
#include "CrashHandler.h"
#include "MacOSUtils.h"
#include "Log.h"
#include "Input.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <mach-o/dyld.h>

Application* Application::s_Instance = nullptr;

Application::Application() {
    s_Instance = this;
    Spark::Log::Init();
    Spark::CrashHandler::Init();
    SP_INFO("Initializing Spark Application...");

    // Ensure only one instance runs
    Spark::InstanceLock::Lock();

    LoadWindowSettings();
    if (!m_Window) {
        m_Window = std::make_unique<Window>(1280, 720, "Spark Engine");
    }
    m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

    m_ImGuiLayer = new ImGuiLayer(*m_Window);
    m_ImGuiLayer->OnAttach();

    Spark::ProjectManager::RefreshRecentProjects();

    // --- START LOADING SEQUENCE (STARTUP STATE) ---
    SetState(ApplicationState::STARTUP);
    
    Spark::LoadingScreen::Render(*m_Window, *m_ImGuiLayer, "Initializing Window & Input...", 0.1f);
    Spark::MacOSUtils::SetupApplicationMenu();
    Spark::Input::Init(m_Window->GetNativeWindow());

    Spark::LoadingScreen::Render(*m_Window, *m_ImGuiLayer, "Mounting File Systems...", 0.2f);
    Spark::FileSystem::Init();

    Spark::LoadingScreen::Render(*m_Window, *m_ImGuiLayer, "Loading Asset Registry...", 0.4f);
    Spark::AssetManager::Init();

    Spark::LoadingScreen::Render(*m_Window, *m_ImGuiLayer, "Starting Renderer...", 0.6f);
    Spark::Renderer2D::Init();

    Spark::LoadingScreen::Render(*m_Window, *m_ImGuiLayer, "Initializing Script Engine...", 0.8f);
    ScriptEngine::Init();

    Spark::LoadingScreen::Render(*m_Window, *m_ImGuiLayer, "Starting Audio Engine...", 0.9f);
    Spark::AudioManager::Init();

    Spark::LoadingScreen::Render(*m_Window, *m_ImGuiLayer, "Preparing UI...", 1.0f);
    
    UpdateWindowTitle();

    // Final Layers
    m_LayerStack.PushOverlayManual(m_ImGuiLayer); 
    
    // Default transition to Main Menu
    SetState(ApplicationState::MAIN_MENU);
}

void Application::Run() {
    float lastFrameTime = 0.0f;
    while (m_Running) {
        float time = (float)glfwGetTime();
        float dt = time - lastFrameTime;
        lastFrameTime = time;

        // Cap dt to prevent physics explosions
        if (dt > 0.1f) dt = 0.1f;

        // 1. Events & Input
        m_Window->PollEvents();
        if (m_Window->ShouldClose())
            m_Running = false;

        // 2. ImGui Start
        m_ImGuiLayer->Begin();

        // 3. Clear window
        m_Window->Clear();

        // 4. Layer-Updates (Spiellogik, Physik, Rendering)
        for (Layer* layer : m_LayerStack) {
            layer->OnUpdate(dt);
        }

        // 5. ImGui-Rendering
        for (Layer* layer : m_LayerStack) {
            layer->OnImGuiRender();
        }
        m_ImGuiLayer->End();

        // 6. Swap Buffers
        m_Window->SwapBuffers();

        if (m_StateChanged) {
            HandleStateTransitions();
        }
    }
}

void Application::SetState(ApplicationState newState) {
    m_NextState = newState;
    m_StateChanged = true;
}

void Application::HandleStateTransitions() {
    m_StateChanged = false;
    SP_INFO("Application State Change: " + std::to_string((int)m_State) + " -> " + std::to_string((int)m_NextState));
    
    ApplicationState oldState = m_State;
    m_State = m_NextState;

    // Clear main layers for new state, but keep ImGui overlay
    m_LayerStack.Clear(true);

    if (m_State == ApplicationState::MAIN_MENU) {
        PushLayer(new Spark::MainMenuLayer());
    }

    if (m_State == ApplicationState::PROJECT_LOADING) {
        Spark::LoadingScreen::Render(*m_Window, *m_ImGuiLayer, "Loading Project Assets...", 0.5f);
        SetState(ApplicationState::EDITOR);
    }

    if (m_State == ApplicationState::EDITOR) {
        PushLayer(new DebuggerLayer());
        PushLayer(new TriangleLayer());
    }
}

void Application::LoadWindowSettings() {
    std::ifstream stream("window_settings.yaml");
    if (!stream.is_open()) return;

    try {
        YAML::Node data = YAML::Load(stream);
        if (data["Window"]) {
            int width = data["Window"]["Width"].as<int>();
            int height = data["Window"]["Height"].as<int>();
            if (width > 100 && height > 100) {
                m_Window = std::make_unique<Window>(width, height, "Spark Engine");
                SP_INFO("Loaded window settings: " + std::to_string(width) + "x" + std::to_string(height));
            }
        }
    } catch (const std::exception& e) {
        SP_ERROR("Failed to load window settings: " + std::string(e.what()));
    }
}

void Application::SaveWindowSettings() {
    if (!m_Window) return;

    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Window" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "Width" << YAML::Value << m_Window->GetWidth();
    out << YAML::Key << "Height" << YAML::Value << m_Window->GetHeight();
    out << YAML::EndMap;
    out << YAML::EndMap;

    std::ofstream fout("window_settings.yaml");
    fout << out.c_str();
}

Application::~Application() {
    SaveWindowSettings();
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

void Application::Restart(bool verbose) {
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        SP_ERROR("Failed to get executable path for restart!");
        return;
    }

    SP_INFO("Restarting Engine...");
    
    // Cleanup
    this->~Application();

    char* argv[3];
    argv[0] = path;
    if (verbose) {
        argv[1] = (char*)"--verbose";
        argv[2] = nullptr;
    } else {
        argv[1] = nullptr;
    }

    execv(path, argv);
    
    // Falls execv fehlschlägt
    exit(1);
}

void Application::RebuildAndRestart() {
    SP_INFO("Starting Rebuild...");
    
    int result = std::system("python3 build.py");
    
    if (result == 0) {
        SP_INFO("Build successful! Restarting...");
        Restart(Spark::Log::IsVerbose());
    } else {
        SP_ERROR("Build failed! Check console for errors.");
    }
}

void Application::UpdateWindowTitle() {
    if (!m_Window) return;
    
    std::string title = "Spark Engine v" + std::string(SPARK_VERSION_STR);
    if (Spark::Log::IsVerbose()) {
        title += " - Verbose";
    }
    m_Window->SetTitle(title);
}
