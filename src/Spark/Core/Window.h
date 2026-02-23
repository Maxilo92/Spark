#pragma once
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "Events.h"
#include <functional>

class Window {
public:
    using EventCallbackFn = std::function<void(Event&)>;

    Window(int width, int height, const std::string& title);
    ~Window();

    void OnUpdate();
    bool ShouldClose() const;
    void Clear();

    void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }

    GLFWwindow* GetNativeWindow() const { return m_Window; }
    int GetWidth() const { return m_Data.Width; }
    int GetHeight() const { return m_Data.Height; }

private:
    void Init();
    void Shutdown();

private:
    GLFWwindow* m_Window;

    struct WindowData {
        std::string Title;
        unsigned int Width, Height;
        EventCallbackFn EventCallback;
    };

    WindowData m_Data;
};
