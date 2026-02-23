#include "Window.h"
#include <iostream>
#include <imgui_impl_glfw.h>

Window::Window(int width, int height, const std::string& title) {
    m_Data.Width = width;
    m_Data.Height = height;
    m_Data.Title = title;
    Init();
}

Window::~Window() {
    Shutdown();
}

void Window::Init() {
    if (!glfwInit()) {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return;
    }

    // Mac-spezifische OpenGL Einstellungen
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), NULL, NULL);
    if (!m_Window) {
        glfwTerminate();
        std::cerr << "Could not create window!" << std::endl;
        return;
    }

    glfwMakeContextCurrent(m_Window);
    glfwSetWindowUserPointer(m_Window, &m_Data);
    glfwSwapInterval(1); // VSync

    // GLFW Callbacks setzen
    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        WindowCloseEvent event;
        data.EventCallback(event);
    });

    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.Width = width;
        data.Height = height;

        WindowResizeEvent event(width, height);
        data.EventCallback(event);
    });

    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        
        switch (action) {
            case GLFW_PRESS: {
                KeyPressedEvent event(key);
                data.EventCallback(event);
                break;
            }
            case GLFW_REPEAT: {
                KeyPressedEvent event(key); // Behandle Repeat wie Press für Input-State
                data.EventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleasedEvent event(key);
                data.EventCallback(event);
                break;
            }
        }
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    });

    glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint) {
        ImGui_ImplGlfw_CharCallback(window, codepoint);
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    });
}

void Window::Shutdown() {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::OnUpdate() {
    glfwPollEvents();
    glfwSwapBuffers(m_Window);
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(m_Window);
}

void Window::Clear() {
    int display_w, display_h;
    glfwGetFramebufferSize(m_Window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::SetTitle(const std::string& title) {
    m_Data.Title = title;
    glfwSetWindowTitle(m_Window, title.c_str());
}
