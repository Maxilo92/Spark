#include "Application.h"
#include "DebuggerLayer.h"
#include "TriangleLayer.h"
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
#include <unistd.h>
#include <mach-o/dyld.h>

Application* Application::s_Instance = nullptr;

Application::Application() {
    s_Instance = this;
    Spark::Log::Init();
    Spark::CrashHandler::Init();
    SP_INFO("Initializing Spark Application...");

    // Sicherstellen, dass nur eine Instanz läuft
    Spark::InstanceLock::Lock();

    m_Window = std::make_unique<Window>(1280, 720, "Spark Engine");
    m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

    m_ImGuiLayer = new ImGuiLayer(*m_Window);
    m_ImGuiLayer->OnAttach(); // Manual attach for splash screen context

    // --- START LOADING SEQUENCE ---
    SetLoadingStatus("Initializing Window & Input...", 0.1f);
    Spark::MacOSUtils::SetupApplicationMenu();
    Spark::Input::Init(m_Window->GetNativeWindow());

    SetLoadingStatus("Mounting File Systems...", 0.2f);
    Spark::FileSystem::Init();

    SetLoadingStatus("Loading Asset Registry...", 0.4f);
    Spark::AssetManager::Init();

    SetLoadingStatus("Starting Renderer...", 0.6f);
    Spark::Renderer2D::Init();

    SetLoadingStatus("Initializing Script Engine...", 0.8f);
    ScriptEngine::Init();

    SetLoadingStatus("Starting Audio Engine...", 0.9f);
    Spark::AudioManager::Init();

    SetLoadingStatus("Preparing Editor UI...", 1.0f);
    
    UpdateWindowTitle();

    // Final Layers
    // Note: We don't call PushOverlay(m_ImGuiLayer) here because it would call OnAttach again.
    // We add it manually to the stack.
    m_LayerStack.PushOverlayManual(m_ImGuiLayer); 
    
    PushLayer(new DebuggerLayer());
    PushLayer(new TriangleLayer());

    // Artificial wait for 1 second as requested
    usleep(1000000); 
    m_IsLoading = false;
}

void Application::SetLoadingStatus(const std::string& status, float progress) {
    m_LoadingStatus = status;
    m_LoadingProgress = progress;
    SP_INFO("Loading: " + status + " (" + std::to_string((int)(progress * 100)) + "%)");
    
    // Render splash frame immediately
    RenderSplashScreen();
}

void Application::RenderSplashScreen() {
    m_Window->Clear();
    m_ImGuiLayer->Begin();

    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ (float)m_Window->GetWidth(), (float)m_Window->GetHeight() });
    
    ImGui::Begin("SplashScreen", nullptr, 
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove);

    // Center content
    float windowWidth = ImGui::GetWindowWidth();
    float windowHeight = ImGui::GetWindowHeight();

    // Title
    ImGui::SetCursorPosY(windowHeight * 0.3f);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetWindowFontScale(4.0f);
    float textWidth = ImGui::CalcTextSize("SPARK ENGINE").x;
    ImGui::SetCursorPosX((windowWidth - textWidth) / 2.0f);
    ImGui::TextColored({ 0.2f, 0.6f, 1.0f, 1.0f }, "SPARK ENGINE");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();

    // Progress Bar
    ImGui::SetCursorPosY(windowHeight * 0.6f);
    ImGui::SetCursorPosX(windowWidth * 0.2f);
    ImGui::ProgressBar(m_LoadingProgress, ImVec2(windowWidth * 0.6f, 30.0f));

    // Status Text
    float statusWidth = ImGui::CalcTextSize(m_LoadingStatus.c_str()).x;
    ImGui::SetCursorPosX((windowWidth - statusWidth) / 2.0f);
    ImGui::Text("%s", m_LoadingStatus.c_str());

    // Verbose Log (only if enabled)
    if (Spark::Log::IsVerbose()) {
        ImGui::SetCursorPosY(windowHeight * 0.75f);
        ImGui::BeginChild("LoadingLog", { windowWidth * 0.8f, 100.0f }, true);
        const auto& messages = Spark::Log::GetMessages();
        int start = std::max(0, (int)messages.size() - 5);
        for (int i = start; i < (int)messages.size(); i++) {
            ImGui::TextDisabled("[%s] %s", messages[i].Timestamp.c_str(), messages[i].Message.c_str());
        }
        ImGui::EndChild();
    }

    ImGui::End();
    m_ImGuiLayer->End();
    m_Window->OnUpdate();
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
    
    // Wir nutzen system(), um auf den Abschluss des Builds zu warten
    // Auf macOS/Linux ist es sicher, das laufende Executable zu überschreiben
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
    
    std::string title = "Spark Engine";
    if (Spark::Log::IsVerbose()) {
        title += " - Verbose";
    }
    m_Window->SetTitle(title);
}
