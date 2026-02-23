#pragma once

enum class EventType {
    None = 0,
    WindowClose, WindowResize,
    KeyPressed, KeyReleased,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

class Event {
public:
    virtual ~Event() = default;
    virtual EventType GetType() const = 0;
    bool Handled = false;
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
