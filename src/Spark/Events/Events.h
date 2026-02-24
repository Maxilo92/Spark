#pragma once
#include <string>

enum class EventType {
    None = 0,
    WindowClose, WindowResize,
    KeyPressed, KeyReleased,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
    ProjectLoaded, ProjectSaved
};

class Event {
public:
    virtual ~Event() = default;
    virtual EventType GetType() const = 0;
    bool Handled = false;
};

class ProjectLoadedEvent : public Event {
public:
    ProjectLoadedEvent(const std::string& path) : Path(path) {}
    EventType GetType() const override { return EventType::ProjectLoaded; }
    std::string Path;
};

class ProjectSavedEvent : public Event {
public:
    ProjectSavedEvent(const std::string& path) : Path(path) {}
    EventType GetType() const override { return EventType::ProjectSaved; }
    std::string Path;
};

class WindowCloseEvent : public Event {
public:
    EventType GetType() const override { return EventType::WindowClose; }
};

class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(int width, int height) : Width(width), Height(height) {}
    EventType GetType() const override { return EventType::WindowResize; }
    int Width, Height;
};

class KeyEvent : public Event {
public:
    KeyEvent(int keycode) : KeyCode(keycode) {}
    int KeyCode;
};

class KeyPressedEvent : public KeyEvent {
public:
    KeyPressedEvent(int keycode) : KeyEvent(keycode) {}
    EventType GetType() const override { return EventType::KeyPressed; }
};

class KeyReleasedEvent : public KeyEvent {
public:
    KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}
    EventType GetType() const override { return EventType::KeyReleased; }
};

class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(float x, float y) : MouseX(x), MouseY(y) {}
    EventType GetType() const override { return EventType::MouseMoved; }
    float MouseX, MouseY;
};

class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(float xOffset, float yOffset) : XOffset(xOffset), YOffset(yOffset) {}
    EventType GetType() const override { return EventType::MouseScrolled; }
    float XOffset, YOffset;
};

class MouseButtonEvent : public Event {
public:
    MouseButtonEvent(int button) : Button(button) {}
    int Button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
public:
    MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}
    EventType GetType() const override { return EventType::MouseButtonPressed; }
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
    MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}
    EventType GetType() const override { return EventType::MouseButtonReleased; }
};
