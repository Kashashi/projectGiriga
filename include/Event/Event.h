#ifndef GIRIGARA_EVENT_EVENT_H
#define GIRIGARA_EVENT_EVENT_H

class Event {
public:
    virtual ~Event() = default;
    bool isHandled = false; // 是否已被消耗（阻止繼續廣播）
};

// 鍵盤基礎事件
class KeyEvent : public Event {
public:
    KeyEvent(int key, int scancode, int action, int mods)
    : key(key), scancode(scancode), action(action), mods(mods) {}

    int key;
    int scancode;
    int action;
    int mods;
};

// 鍵盤按下事件
class KeyPressedEvent : public KeyEvent {
public:
    KeyPressedEvent(int key, int scancode, int mods)
    : KeyEvent(key, scancode, 1 /* GLFW_PRESS */, mods) {}
};

// 鍵盤放開事件
class KeyReleasedEvent : public KeyEvent {
public:
    KeyReleasedEvent(int key, int scancode, int mods)
    : KeyEvent(key, scancode, 0 /* GLFW_RELEASE */, mods) {}
};

// 滑鼠移動事件
class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(double x, double y) : x(x), y(y) {}
    double x;
    double y;
};

// 滑鼠按鍵事件
class MouseButtonEvent : public Event {
public:
    MouseButtonEvent(int button, int action, int mods, double x, double y)
    : button(button), action(action), mods(mods), x(x), y(y) {}
    int button;
    int action;
    int mods;
    double x;
    double y;
};

// 視窗聚焦事件
class WindowFocusEvent : public Event {
public:
    WindowFocusEvent(bool focused) : focused(focused) {}
    bool focused;
};

#endif // GIRIGARA_EVENT_EVENT_H
